#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

u_char get_msg_type(msg x) {
  u_char aux;
  memcpy(&aux, x.payload + 3, 1);
  return aux;
}

u_char get_msg_seq(msg* x) {
  u_char aux;
  memcpy(&aux, x->payload + 2, 1);
  return aux;
}

void change_msg_seq(msg* x, u_char i) {
  memcpy(x->payload + 2, &i, 1);
  unsigned short crc = crc16_ccitt(x->payload, x->len - 3);
  memcpy(x->payload + x->len - 3, &crc, 2);
}

int main(int argc, char** argv) {

    msg t, last_msg;
    kerm_pack pack;
    int local_seq = 0;
    // ----------------------------------------
    //Preparing the first frame - INIT
    // ----------------------------------------
    /*
    Construirea pachetului de initializare cu numarul de secventa 0
    si datele pentru timeout si MAXL
    */
    pack.SOH = 0x01;
    pack.LEN = 16;
    pack.SEQ = ((unsigned char) local_seq) % 64;
    local_seq = pack.SEQ;
    pack.TYPE = 'S';

    memcpy(t.payload, &pack, 4);

    init_pack *ipack = calloc(1, sizeof(init_pack));
    ipack->MAXL = 250;
    ipack->TIME = 5;
    ipack->EOL = 0x0D;

    memcpy(t.payload + 4, ipack, 11);

    int crc_len = pack.LEN - 1;
    pack.CHECK = crc16_ccitt(t.payload, crc_len);
    pack.MARK = ipack->EOL;
    printf("[%s] : CRC of INIT before sending = 0x%04X\n", argv[0], pack.CHECK);
    memcpy(t.payload + 15, &(pack.CHECK), 2);
    memcpy(t.payload + 17, &(pack.MARK), 1);

    init(HOST, PORT);

    t.len = (int) (pack.LEN + 2);
    send_message(&t);
    last_msg = t;
    printf("[%s][%d] : Init Frame sent. Waiting ACK...\n", argv[0], get_msg_seq(&t));
    //Se trimite pachetul INIT si se asteapta ACK pentru el
    // ----------------------------------------
    //INIT sent
    // ----------------------------------------

    //Waiting ACK for INIT
    int counter = 0;
    while(1) {
      msg *init_ack = receive_message_timeout(5000);
      if (init_ack == NULL) {
          counter++;
          if(counter == 4) {
            printf("[%s] : Maximum amount of timeouts (ack for init). Exiting...\n", argv[0]);
            return 0;
          }
          printf("[%s] : Timeout at receiving ACK for INIT. Resending...\n", argv[0]);
          /*In cazul in care nu se primeste ACK si se da timeout din diferite motive
          fie pachetul INIT nu a ajuns la receiver, fie a ajuns insa raspunsul acestuia
          nu a ajuns inapoi la sender, se va retrimite ultimul mesaj, in acest caz mesajul
          de initializare a conexinuii.
          */
          send_message(&last_msg);


        } else {

          if(get_msg_type(*init_ack) == 'N') {
            //Daca se primeste NAK se retrimite pachetul
            local_seq = get_msg_seq(init_ack) + 1;
            change_msg_seq(&last_msg, local_seq);
            printf("[%s][%d] Got NAK (init) - Resending...\n", argv[0], get_msg_seq(&last_msg));
            send_message(&last_msg);

          } else if(get_msg_type(*init_ack) == 'Y') {
            //Daca se primeste ACK se merge mai departe
            local_seq = get_msg_seq(init_ack) + 1;
            change_msg_seq(&last_msg, local_seq);
            printf("[%s][%d] Got ACK (init) - Continuing...\n", argv[0], get_msg_seq(&last_msg));
            break;

          }
        }
    }
    //In urmatorul for se parcurg fisierele de trimis si se trimit pe rand
    for(int i = 1; i < argc; i++) {
    printf("[%s] : Name of file = %s\n", argv[0], argv[i]);
    // ----------------------------------------
    //Preparing the second frame - FILE HEADER
    // ----------------------------------------
    kerm_pack file_header;
    file_header.SOH = 0x01;
    file_header.LEN = strlen(argv[i]) + 6;
    file_header.SEQ = (unsigned char) local_seq;
    //local_seq = file_header.SEQ;
    file_header.TYPE = 'F';
    int offset = 0;
    //Se construieste pachetul de tip HEADER ce contine numele fisierului de trimis
    memcpy(t.payload, &file_header, 4);
    offset += 4;

    memcpy(t.payload + offset, argv[i], strlen(argv[i]) + 1);
    offset += strlen(argv[i]) + 1;

    crc_len = file_header.LEN - 1;

    file_header.CHECK = crc16_ccitt(t.payload, crc_len);
    file_header.MARK = 0x0D;

    printf("[%s] : CRC of FILE-HEADER before sending = 0x%04X\n", argv[0], file_header.CHECK);
    //Se calculeaza CRC-ul si pentru utilizare mai facila se afisieaza la output
    memcpy(t.payload + offset, &(file_header.CHECK), 2);
    offset += 2;
    memcpy(t.payload + offset, &(file_header.MARK), 1);
    offset = 0;
    t.len = (int) (file_header.LEN + 2);
    printf("[%s][%d] : Sending FILE HEADER\n", argv[0], get_msg_seq(&t));
    send_message(&t);
    last_msg = t;

    counter = 0;
    //Se asteapta raspuns pentru frameul file headeer
    while(1) {
      msg *fhead_ack = receive_message_timeout(5000);

      if(fhead_ack == NULL){
        counter++;
        if(counter == 4) {
          printf("[%s] : Maximum amount of timeouts (ack for fheader). Exiting...\n", argv[0]);
          return 0;
        }
        printf("[%s] : Timeout at receiving ACK for FILE HEADER. Resending...\n", argv[0]);
        //In caz de timeut se retrimite ultimul pachet
        send_message(&last_msg);

      } else {
        if(get_msg_type(*fhead_ack) == 'N') {

          local_seq = get_msg_seq(fhead_ack) + 1;
          change_msg_seq(&last_msg, local_seq);
          printf("[%s][%d] Got NAK (file_header) - Resending...\n", argv[0], get_msg_seq(&last_msg));
          //In caz de NAK se retrimite ultimul pachet
          send_message(&last_msg);

        } else if(get_msg_type(*fhead_ack) == 'Y') {
          //In caz de ACK se merge mai departe
          local_seq = get_msg_seq(fhead_ack) + 1;
          change_msg_seq(&last_msg, local_seq);
          printf("[%s][%d] Got ACK (file_header) - Continuing...\n", argv[0], get_msg_seq(&last_msg));
          break;

        }
      }
    }
    FILE *fp;
    fp = fopen(argv[i],"rb");
    //Se deschide fisierul pentru a incepe sa trimitem date
    char* buffer = calloc(1,251);
    int bytes_read = fread(buffer, 1, 250, fp);
    /*Cat timp se poate citii din fisier maxim 250 de bytes_read
    se trimit pachete de date
    */
    while(bytes_read != 0) {
      printf("[%s] : Bytes read = %d\n", argv[0], bytes_read);
      //BUILDING THE DATA frame
      printf("[%s] : Building the DATA block...\n", argv[0]);
      kerm_pack data;
      data.SOH = 0x01;
      data.LEN = (unsigned char) (bytes_read + 5);
      data.SEQ = (unsigned char) local_seq;
      data.TYPE = 'D';
      offset = 0;
      memcpy(t.payload, &data, 4);
      offset += 4;
      memcpy(t.payload + offset, buffer, bytes_read);
      offset += bytes_read;
      crc_len = data.LEN - 1;
      data.CHECK = crc16_ccitt(t.payload, crc_len);
      printf("[%s] : CRC of DATA pack before sending is 0x%04X\n", argv[0], data.CHECK);
      data.MARK = 0x0D;
      memcpy(t.payload + offset, &(data.CHECK), 2);
      offset += 2;
      memcpy(t.payload + offset, &(data.MARK), 1);
      offset = 0;
      t.len = (int) (data.LEN + 2);
      printf("[%s][%d] : Sending DATA pack...\n", argv[0], get_msg_seq(&t));
      send_message(&t);
      last_msg = t;

      counter = 0;
      while(1) {
        msg* data_ack = receive_message_timeout(5000);
        if(data_ack == NULL) {

          counter++;
          if(counter == 4) {
            printf("[%s] : Maximum amount of timeouts (ack for data). Exiting...\n", argv[0]);
            return 0;
          }
          printf("[%s] : TIMEOUT at waiting ACK for DATA \n", argv[0]);
          send_message(&last_msg);

        } else {
          /*Se verifica daca ACK-ul primit este cel asteptat pentru
          a nu trimite urmatorul pachet de date daca actualul nu este confirmat
          sau sa primeasca un ACK ce nu corespunde, senderul sa il vada ca un ACK ok
          si sa trimita urmatorul pachet de date, in acest caz un pachet de date este
          practic "sarit".
          */
          if(get_msg_seq(data_ack) == local_seq + 1) {
            if(get_msg_type(*data_ack) == 'N') {

              local_seq = get_msg_seq(data_ack) + 1;
              change_msg_seq(&last_msg, local_seq);
              printf("[%s][%d] Got NAK (data) - Resending...\n", argv[0], get_msg_seq(&last_msg));
              send_message(&last_msg);

            } else if(get_msg_type(*data_ack) == 'Y') {
              local_seq = get_msg_seq(data_ack) + 1;
              change_msg_seq(&last_msg, local_seq);
              printf("[%s][%d] Got ACK (data) - Continuing...\n", argv[0], get_msg_seq(&last_msg));
              break;

            }
          } else {
            printf("[%s] : Expected sequence %d. Got sequence %d\n", argv[0], local_seq + 1, get_msg_seq(data_ack));
            send_message(&last_msg);
          }
        }
      }
      bytes_read = fread(buffer, 1, 250, fp);
    }
    msg eof;
    pack.SOH = 0x01;
    pack.LEN = 5;
    pack.SEQ = local_seq;
    pack.TYPE = 'Z';
    memcpy(eof.payload, &pack, 4);
    pack.CHECK = crc16_ccitt(eof.payload, 4);
    pack.MARK = 0x0D;
    memcpy(eof.payload + 4, &(pack.CHECK), 2);
    memcpy(eof.payload + 6, &(pack.MARK), 1);
    eof.len = 7;
    send_message(&eof);
    /*Dupa iesirea din while adica terminarea fisierului se va trimite EOF ( End of File )
    pentru ca recieverul sa inchida fisierul curent si sa astepte urmatorul File Header sau
    End of Transmission daca a fost ultimul fisier.
    */
    counter = 0;
    while(1) {
      msg *eof_ack = receive_message_timeout(5000);

      if(eof_ack == NULL){
        counter++;
        if(counter == 4) {
          printf("[%s] : Maximum amount of timeouts (ack for eof). Exiting...\n", argv[0]);
          return 0;
        }
        printf("[%s] : Timeout at receiving ACK for EOF. Resending...\n", argv[0]);
        send_message(&eof);

      } else {
        /*Se asteapta raspuns pentru EOF, se verifica si secventa din acelasi motiv
        pentru a nu sari un pachet de date.
        */
        if(get_msg_seq(eof_ack) == local_seq + 1) {
          if(get_msg_type(*eof_ack) == 'N') {

            local_seq = get_msg_seq(eof_ack) + 1;
            change_msg_seq(&eof, local_seq);
            printf("[%s][%d] Got NAK (eof) - Resending...\n", argv[0], get_msg_seq(&eof));
            send_message(&eof);

          } else if(get_msg_type(*eof_ack) == 'Y') {
            local_seq = get_msg_seq(eof_ack) + 1;
            change_msg_seq(&eof, local_seq);
            printf("[%s][%d] Got ACK (eof) - Ending ! Good bye !\n", argv[0], get_msg_seq(&eof));
            break;
          }
        } else {
          printf("[%s] : Eof ACK - wrong sequence\n", argv[0]);
        }
      }
    }
    fclose(fp);
  }
  //EOT Frame
  msg eot;
  pack.SOH = 0x01;
  pack.LEN = 5;
  pack.SEQ = local_seq;
  pack.TYPE = 'B';
  memcpy(eot.payload, &pack, 4);
  pack.CHECK = crc16_ccitt(eot.payload, 4);
  pack.MARK = 0x0D;
  memcpy(eot.payload + 4, &(pack.CHECK), 2);
  memcpy(eot.payload + 6, &(pack.MARK), 1);
  eot.len = 7;
  send_message(&eot);
  /*Dupa iesirea din for, adica transmiterea tuturor fisierelor se
  trimite semnalul EOT pentru inchierea transmisiei. De asemenea, se asteapta
  ACK pentru pachetul EOT, adica o confirmare ca si receiverul este constient
  de incheierea transmisiei
  */
  counter = 0;
  while(1) {
    msg *eot_ack = receive_message_timeout(5000);

    if(eot_ack == NULL){
      counter++;
      if(counter == 4) {
        printf("[%s] : Maximum amount of timeouts (ack for eot). Exiting...\n", argv[0]);
        return 0;
      }
      printf("[%s] : Timeout at receiving ACK for EOt. Resending...\n", argv[0]);
      send_message(&eot);

    } else {
      if(get_msg_seq(eot_ack) == local_seq + 1) {
        if(get_msg_type(*eot_ack) == 'N') {

          local_seq = get_msg_seq(eot_ack) + 1;
          change_msg_seq(&eot, local_seq);
          printf("[%s][%d] Got NAK (eot) - Resending...\n", argv[0], get_msg_seq(&eot));
          send_message(&eot);

        } else if(get_msg_type(*eot_ack) == 'Y') {
          local_seq = get_msg_seq(eot_ack) + 1;
          change_msg_seq(&eot, local_seq);
          printf("[%s][%d] Got ACK (eot) - Continuing...\n", argv[0], get_msg_seq(&eot));
          break;
        }
      } else {
        printf("[%s] : EOT ACK - wrong sequence\n", argv[0]);
      }
    }
  }
    // ----------------------------------------
    // ----------------------------------------
    return 0;
}
