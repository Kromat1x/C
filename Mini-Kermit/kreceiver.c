#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

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
}

int main(int argc, char** argv) {
    //msg t, last_msg;

    //----ACK MSG----
    /*Pachetele ACK si NAK sunt create dinainte deoarece structura
    lor este aceeasi, difera doar numarul de secventa si CRC-ul,
    numarul de secventa este modificat folosind functia change_msg_seq. La
    reciever aceasta functie nu mai recalculeaza CRC-ul deoarece oricum el
    nu este verificat cand ajunge la sender pentru ca transmisia de la reciever
    spre sender nu se corupe.
    */
    msg ack;
    kerm_pack pack;
    pack.SOH = 0x01;
    pack.LEN = 5;
    pack.SEQ = 0; //to be set
    pack.TYPE = 'Y';
    memcpy(ack.payload, &pack, 4);
    pack.CHECK = crc16_ccitt(ack.payload, 4);
    pack.MARK = 0x0D;
    //---------------
    //----NAK MSG----
    msg nak;
    pack.SOH = 0x01;
    pack.LEN = 5;
    pack.SEQ = 0; //to be set
    pack.TYPE = 'N';
    memcpy(nak.payload, &pack, 4);
    pack.CHECK = crc16_ccitt(nak.payload, 4);
    pack.MARK = 0x0D;
    //---------------

    init(HOST, PORT);
    int counter = 0;
    int local_seq = 0;
    // ----------------------------------------
    //Recieving the first frame - INIT
    // ----------------------------------------
    //Se asteapta pachetul de initializare
    while(1) {
      msg *r = receive_message_timeout(5000);
      if(r == NULL) {
        counter++;
        if(counter == 4) {
          /*La fel ca si la celelalte primiri de pachete se verifica de cat ori se da timeout,
          datorita contorului se poate da timeout in acelasi loc de maxim 3 ori.
          */
          printf("[%s] : Maximum amount of timeouts (init). Exiting...\n", argv[0]);
          return 0;
        }
        printf("[%s] : Timeout at receiving INIT Frame\n", argv[0]);
      } else {
        /*Creem un pachet nou in care urmeaza sa "despachetam" frame-ul primit
        se printeaza la consola CRC-ul primit in pachet dar si cel calculat local_seq
        si totodata se verifica egalitatea acestora, in caz de egalitate se trimite
        ACK pentru pachetul cu numarul de secventa : secventa pachet primit + 1.
        In cazul in care pachetul este corupt si CRC-urile difera se trimite NAK.
        */
        kerm_pack pack;
        memcpy(&pack, r->payload, 4);
        init_pack ipack;
        memcpy(&ipack, r->payload + 4, 11);
        memcpy(&(pack.CHECK), r->payload + 15, 2);
        memcpy(&(pack.MARK), r->payload + 17, 1);
      // ----------------------------------------
      //INIT recieved
      // ----------------------------------------
      //Sending ACK for INIT
        printf("[%s] : CRC RECEIEVED IN PACKAGE = 0x%04X\n", argv[0], pack.CHECK);
        unsigned short crc = crc16_ccitt(r->payload, r->len - 3);
        printf("[%s] : CRC calculated here from data = 0x%04X\n",argv[0], crc);
        if(crc == pack.CHECK) {
          printf("[%s] : CRC correct (init) - Sending ACK...\n", argv[0]);
          local_seq = get_msg_seq(r) + 1;
          change_msg_seq(&ack, (unsigned char) local_seq);
          printf("[%s][%d] : Sending ACK...\n", argv[0], get_msg_seq(&ack));
          send_message(&ack);
          break;
        } else {
          printf("[%s] : CRC incorrect (init) - Sending NAK...\n", argv[0]);
          local_seq = get_msg_seq(r) + 1;
          change_msg_seq(&nak, (unsigned char) local_seq);
          printf("[%s][%d] : Sending NAK...\n", argv[0], get_msg_seq(&nak));
          send_message(&nak);
        }
      }
    }

    //Recieving FILE HEADER
    /*Dupa ce s-a stabilit initializarea conexiunii
    urmeaza sa se trimita numele fisierului intr-un frame numit FILE HEADER
    */
    while(1) {
    counter = 0;
    char* filename;
    while(1) {
      msg *y = receive_message_timeout(5000);
      if( y == NULL ) {
        counter++;
        if(counter == 4) {
          printf("[%s] : Maximum amount of timeouts (file header). Exiting...\n", argv[0]);
          return 0;
        }
        printf("[%s] : Timeout at receiving FILE HEADER\n", argv[0]);
        change_msg_seq(&nak, (unsigned char) local_seq);
        printf("[%s][%d] : Sending NAK (timeout)...\n", argv[0], get_msg_seq(&nak));
        send_message(&nak);
        local_seq++;
      } else {
        /*La fel se iau informatiile din pachet, se citeste lungimea
        numelui fisierului pentru a stii cat trebuie sa se citeasca, se verifica
        CRC-ul, in cazul in care este bun, se trimite ACK, daca pachetul a fost corrupt
        se trimte NAK. Daca in acest stadiu se primeste un pachet de tip B (End of Transmission)
        inseamna ca fisierele de primit s-au terminat si se poate incheia cu succes conexiunea,
        astfel se trimite ACK pentru EOT si se iese din program.
        */
        kerm_pack pack;
        printf("[%s] : Received FILE HEADER Frame. Checking for corruption...\n", argv[0]);
        memcpy(&pack, y->payload, 4);
        int offset = 4;
        filename = malloc(pack.LEN - 5);
        memcpy(filename, y->payload + offset, pack.LEN - 5);
        offset += pack.LEN - 5;
        memcpy(&(pack.CHECK), y->payload + offset, 2);
        offset += 2;
        memcpy(&(pack.MARK), y->payload + offset, 1);
        unsigned short crc = crc16_ccitt(y->payload, y->len - 3);
        printf("[%s] CRC calculated here : 0x%04X\n", argv[0], crc);
        printf("[%s] CRC received : 0x%04X\n", argv[0], pack.CHECK);
        if(crc == pack.CHECK) {
          if(get_msg_type(*y) == 'B') {
            printf("[%s] ---- End of Transmission ---- \n", argv[0]);
            local_seq = get_msg_seq(y) + 1;
            change_msg_seq(&ack, (unsigned char)local_seq);
            send_message(&ack);
            return 0;
          } else {
            printf("[%s] : CRC correct (file header)\n", argv[0]);
            local_seq = get_msg_seq(y) + 1;
            change_msg_seq(&ack, (unsigned char)local_seq);
            printf("[%s][%d] : Sending ACK...\n", argv[0], get_msg_seq(&ack));
            send_message(&ack);
            printf("[%s] : Filename received is : %s\n", argv[0], filename);
            break;
          }
        } else {
          printf("[%s] : CRC incorrect (file header)\n", argv[0]);
          local_seq = get_msg_seq(y) + 1;
          change_msg_seq(&nak, (unsigned char) local_seq);
          printf("[%s][%d] : Sending NAK...\n", argv[0], get_msg_seq(&nak));
          send_message(&nak);
        }
      }
    }
    printf("[%s] : Filename outside while is : %s\n", argv[0], filename);
    printf("[%s] : Creating file with name %s...\n", argv[0], strcat(strdup("recv_"), filename));
    FILE *fp;
    char* data;
    counter = 0;
    fp = fopen(strcat(strdup("recv_"),filename), "wb");
    //Se deschide fisierul cu numele primit in pachet
    /*
    In urmatorul while se vor astepta pachete de date, singura
    diferenta fata de celelalte cazuri de asteptare cu while este
    faptul ca din acest while se iese doar cand se primeste un pachet de tip
    'Z' (End of File), caz in care fisierul este inchis si se revine la stadiul unde
    se asteapta file header, daca acel fisier a fost ultimul, se va primit
    End of Transmission si conexiunea se va incheia.
    */

    while(1) {
        msg *f = receive_message_timeout(5000);
        if(f == NULL) {
          printf("[%s] : TIMEOUT at receiving DATA\n", argv[0]);
          counter++;
          if(counter == 4) {
            printf("[%s] : Maximum amount of timeouts (data). Exiting...\n", argv[0]);
            return 0;
          }
          change_msg_seq(&nak, local_seq);
          printf("[%s][%d] : Sending NAK (timeout)...\n", argv[0], get_msg_seq(&nak));
          send_message(&nak);
        } else {
          kerm_pack pack;
          printf("[%s] : Received DATA Frame. Checking for corruption...\n", argv[0]);
          memcpy(&pack, f->payload, 4);
          int offset = 4;
          data = malloc(pack.LEN - 5);
          memcpy(data, f->payload + offset, pack.LEN - 5);
          offset += pack.LEN - 5;
          memcpy(&(pack.CHECK), f->payload + offset, 2);
          offset += 2;
          memcpy(&(pack.MARK), f->payload + offset, 1);
          unsigned short crc = crc16_ccitt(f->payload, f->len - 3);
          printf("[%s] CRC calculated here (data) : 0x%04X\n", argv[0], crc);
          printf("[%s] CRC received (data) : 0x%04X\n", argv[0], pack.CHECK);
          if(crc == pack.CHECK) {
            printf("[%s] : CRC correct (data)\n", argv[0]);
            if(pack.TYPE == 'D') {
              /*Se verifica daca pachetul primit este pachetul asteptat pentru a nu scrie
              date ce nu sunt corecte sau intr-o ordine corecta in fisier.
              */
              if(get_msg_seq(f) == local_seq + 1)
                fwrite(data, 1, pack.LEN - 5, fp);
            } else if(pack.TYPE == 'Z') {
              fclose(fp);
              local_seq = get_msg_seq(f) + 1;
              change_msg_seq(&ack, (unsigned char)local_seq);
              printf("[%s][%d] : Sending ACK...\n", argv[0], get_msg_seq(&ack));
              send_message(&ack);
              break;
            }
            local_seq = get_msg_seq(f) + 1;
            change_msg_seq(&ack, (unsigned char)local_seq);
            printf("[%s][%d] : Sending ACK...\n", argv[0], get_msg_seq(&ack));
            send_message(&ack);
          } else {
            printf("[%s] : CRC incorrect (data)\n", argv[0]);
            local_seq = get_msg_seq(f) + 1;
            change_msg_seq(&nak, (unsigned char) local_seq);
            printf("[%s][%d] : Sending NAK...\n", argv[0], get_msg_seq(&nak));
            send_message(&nak);
          }
        }
    }
  }
    //fclose(fp);
	return 0;
}
