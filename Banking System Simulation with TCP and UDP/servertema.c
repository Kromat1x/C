#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "lib.h"

#define MAX_CLIENTS 5
#define BUFLEN 256

void error(char * msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char * argv[]) {
    int sockfd, newsockfd, portno, clilen, udpsock;
    char buffer[BUFLEN];
    char buffer_send[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, j, pass_awaited = 0;

    fd_set read_fds; //multimea de citire folosita in select()
    fd_set tmp_fds; //multime folosita temporar
    int fdmax; //valoare maxima file descriptor din multimea read_fds

    if (argc < 3) {
        fprintf(stderr, "Usage : %s port_server user_data_file\n", argv[0]);
        exit(1);
    }

    //Reading users from file
    FILE * file;
    file = fopen(argv[2], "r");
    char * buff = calloc(250, 1);
    fgets(buff, 10, file);

    int users_num = atoi(buff);
    card_data * users = calloc(users_num, sizeof(card_data));
    char * token;
    //Se citesc cardurile linie cu linie
    for (i = 0; i < users_num; i++) {
        fgets(buff, 200, file);
        users[i].nume = strdup(strtok(buff, " \n"));
        users[i].prenume = strdup(strtok(NULL, " \n"));
        users[i].nr_card = strdup(strtok(NULL, " \n"));
        users[i].pin = strdup(strtok(NULL, " \n"));
        users[i].secret_pass = strdup(strtok(NULL, " \n"));
        users[i].sold = atof(strdup(strtok(NULL, " \n")));
        users[i].mistakes = 0;
        users[i].blocked = 0;
        users[i].socket = -1;
        users[i].logged_flag = 0;
    }

    printf("User 1 este : %s %s %d %d %s %2lf\n", users[1].nume, users[1].prenume, atoi(users[1].nr_card), atoi(users[1].pin), users[1].secret_pass, users[1].sold);

    printf("Number of users is : %d\n", users_num);

    //golim multimea de descriptori de citire (read_fds) si multimea tmp_fds
    FD_ZERO( & read_fds);
    FD_ZERO( & tmp_fds);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    udpsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
        error("ERROR opening socket");

    portno = atoi(argv[1]);

    memset((char * ) & serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // foloseste adresa IP a masinii
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr * ) & serv_addr, sizeof(struct sockaddr)) < 0)
        error("ERROR on binding TCP");

    if (bind(udpsock, (struct sockaddr * ) & serv_addr, sizeof(struct sockaddr)) < 0)
        error("ERROR on binding UDP");

    listen(sockfd, MAX_CLIENTS);

    //adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
    FD_SET(sockfd, & read_fds);
    FD_SET(udpsock, & read_fds);
    FD_SET(0, & read_fds);
    fdmax = sockfd;
    if (udpsock > sockfd) {
        fdmax = udpsock;
    }
    // main loop
    while (1) {
        tmp_fds = read_fds;
        if (select(fdmax + 1, & tmp_fds, NULL, NULL, NULL) == -1)
            error("ERROR in select");

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, & tmp_fds)) {

                if (i == sockfd) {
                    // a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
                    // actiunea serverului: accept()
                    clilen = sizeof(cli_addr);
                    if ((newsockfd = accept(sockfd, (struct sockaddr * ) & cli_addr, & clilen)) == -1) {
                        error("ERROR in accept");
                    } else {
                        //adaug noul socket intors de accept() la multimea descriptorilor de citire
                        FD_SET(newsockfd, & read_fds);
                        if (newsockfd > fdmax) {
                            fdmax = newsockfd;
                        }
                    }
                    printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
                } else if (i == 0) {
                  //Cazul in care serverul primeste input de la stdin (quit);
                    memset(buffer, 0, BUFLEN);
                    fgets(buffer, BUFLEN - 1, stdin);
                    if (strstr(buffer, "quit") != NULL) {
                      //Se inchid toate socketele si se anunta clientii de inchiderea serverului
                        printf("Quit received on server from stdin\n");
                        for (int i = fdmax; i >= 3; i--) {
                            sprintf(buffer, "quit");
                            printf("Sending quit to client on socket %d\n", i);
                            send(i, buffer, BUFLEN, 0);
                            close(i);
                        }
                        close(sockfd);
                        close(udpsock);
                        FD_ZERO( & read_fds);
                        FD_ZERO( & tmp_fds);
                        return 0;
                    }
                } else if (i == udpsock) {
                  // Daca am primit pe socketul de udp verificam daca asteptam sau nu parola
                    if (pass_awaited == 0) {
                      //Daca nu asteptam parola inseamna ca primim unlock
                        int len = sizeof(serv_addr);
                        int r = recvfrom(i, buffer, BUFLEN, 0, (struct sockaddr * ) & cli_addr, & len);
                        printf("UNLOCK> RECEIVED FROM UDP : %s\n", buffer);
                        if (strstr(buffer, "unlock") != NULL) {
                            token = strtok(buffer, " \n");
                            token = strtok(NULL, " \n");
                            printf("UNLOCK> Card number to be unlocked is : %s\n", token);
                            //Se verifica cazurile de la unlock si se modifica error_code
                            int error_code = 0;
                            for (int k = 0; k < users_num; k++) {
                                if (strcmp(token, users[k].nr_card) == 0) {
                                    error_code = 1;
                                    if (users[k].blocked == 0) {
                                        error_code = 2;
                                    }
                                }
                            }
                            //In functie de eroare se trimite un anumit mesaj la client
                            if (error_code == 0) {
                                sprintf(buffer, "UNLOCK> -6 : Operatie esuata");
                            } else if (error_code == 1) {
                                sprintf(buffer, "UNLOCK> Trimite parola secreta");
                                //something to show that i am waiting for pass
                                pass_awaited = 1;
                            } else if (error_code == 2) {
                                sprintf(buffer, "UNLOCK> -6 : Operatie esuata");
                            }
                            r = sendto(i, buffer, BUFLEN, 0, (struct sockaddr * ) & cli_addr, len);
                        }
                    } else {
                        //Password is awaited
                        //Daca parola e asteptata este citita si verificata cu parola secreta inregistrata pentru user
                        int len = sizeof(serv_addr);
                        int r = recvfrom(i, buffer, BUFLEN, 0, (struct sockaddr * ) & cli_addr, & len);
                        printf("UNLOCK> RECEIVED PASS FROM UDP: %s", buffer);
                        token = strtok(buffer, " \n");
                        char * pass = strdup(token);
                        token = strtok(NULL, " \n");
                        printf("UNLOCK> Card number attached to password is: %s\n", token);
                        int error_code = 0;
                        int user_index = 0;
                        for (int k = 0; k < users_num; k++) {
                            if (strcmp(token, users[k].nr_card) == 0) {
                                //we found the user
                                user_index = k;
                                if (users[k].blocked == 0) {
                                    error_code = 2; //user is not blocked *
                                } else {
                                    if (strcmp(users[k].secret_pass, pass) != 0) {
                                        //password is Wrong
                                        pass_awaited = 0;
                                        error_code = 1;
                                        //Daca se greseste nu se mai asteapta parola apoi
                                    }
                                }
                                if (error_code == 1) {
                                    sprintf(buffer, "UNLOCK> -7 : Deblocare esuata");
                                } else if (error_code == 2) {
                                    sprintf(buffer, "UNLOCK> -6 : Operatie esuata");
                                } else if (error_code == 0) {
                                    sprintf(buffer, "UNLOCK> Client deblocat");
                                    users[user_index].blocked = 0;
                                }
                            }
                        }
                        r = sendto(i, buffer, BUFLEN, 0, (struct sockaddr * ) & cli_addr, len);
                    }
                } else {
                    // am primit date pe unul din socketii cu care vorbesc cu clientii
                    //actiunea serverului: recv()
                    memset(buffer, 0, BUFLEN);
                    if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
                        if (n == 0) {
                            //conexiunea s-a inchis
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            error("ERROR in recv");
                        }
                        close(i);
                        FD_CLR(i, & read_fds); // scoatem din multimea de citire socketul pe care
                    } else { //recv intoarce >0
                        printf("Am primit de la clientul de pe socketul %d, mesajul: %s\n", i, buffer);
                        token = strtok(buffer, " \n");
                        printf("token = %s\n", token);
                        int user = 0;
                        //Cazuri login
                        if (strcmp(token, "login") == 0) {
                            char * nr_card = strtok(NULL, " \n");
                            char * pin_card = strtok(NULL, " \n");
                            for (int k = 0; k < users_num; k++) {
                                if (!strcmp(nr_card, users[k].nr_card)) {
                                    if (users[k].logged_flag == 1) {
                                      //Daca utilizatorul e deja logat
                                        printf("ATM> -2 : Sesiune deja deschisa\n");
                                        sprintf(buffer_send, "ATM> -2 : Sesiune deja deschisa");
                                        send(i, buffer_send, BUFLEN, 0);
                                        user = 1;
                                    } else {
                                        if (users[k].blocked == 1) {
                                          //Daca cardul este blocat
                                            printf("ATM> -5 : Card blocat\n");
                                            sprintf(buffer_send, "ATM> -5 : Card blocat");
                                            send(i, buffer_send, BUFLEN, 0);
                                            user = 1;
                                        } else {
                                            if (!strcmp(pin_card, users[k].pin)) {
                                              //Logare cu succes
                                                printf("Login Successful on socket %d\n", i);
                                                sprintf(buffer_send, "ATM> Welcome %s %s", users[k].nume, users[k].prenume);
                                                send(i, buffer_send, BUFLEN, 0);
                                                users[k].mistakes = 0;
                                                users[k].socket = i;
                                                users[k].logged_flag = 1;
                                                user = 1;
                                            } else {
                                                if (users[k].mistakes == 2) {
                                                  //Se blocheaza cardul cand se face a 3-a greseala
                                                    printf("ATM> -5 : Card blocat\n");
                                                    sprintf(buffer_send, "ATM> -5 : Card blocat");
                                                    send(i, buffer_send, BUFLEN, 0);
                                                    users[k].mistakes++;
                                                    users[k].blocked = 1;
                                                    user = 1;
                                                } else {
                                                  //Se anunta ca pin-ul este gresit
                                                    users[k].mistakes++;
                                                    printf("ATM> -3 : Pin gresit\n");
                                                    sprintf(buffer_send, "ATM> -3 : Pin gresit");
                                                    send(i, buffer_send, BUFLEN, 0);
                                                    user = 1;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            if (user == 0) {
                              //Daca nu se gaseste utilizatorul atunci cardul este inexistent
                                printf("ATM> -4 : Numar card inexistent\n");
                                sprintf(buffer_send, "ATM> -4 : Numar card inexistent");
                                send(i, buffer_send, BUFLEN, 0);
                            }
                            //Daca primim logout, delogam utilizatorul logat pe socketul pe care am primit logout
                        } else if (strcmp(token, "logout") == 0) {
                            int found_soc = 0;
                            for (int k = 0; k < users_num; k++) {
                                if (users[k].socket == i) {
                                    found_soc = 1;
                                    users[k].logged_flag = 0;
                                    users[k].socket = -1;
                                    printf("ATM> Deconectare de la bancomat\n");
                                    sprintf(buffer_send, "ATM> Deconectare de la bancomat");
                                    send(i, buffer_send, BUFLEN, 0);
                                }
                            }
                            //Daca nu gasim client logat pe acel socket inseamna ca nu este autentificat
                            if (found_soc == 0) {
                                printf("ATM> -1 : Clientul nu este autentificat\n");
                                sprintf(buffer_send, "ATM> -1 : Clientul nu este autentificat");
                                send(i, buffer_send, BUFLEN, 0);
                            }
                            //Listarea soldului
                        } else if (strcmp(token, "listsold") == 0) {
                            for (int k = 0; k < users_num; k++) {
                                if (users[k].socket == i) {
                                    printf("Listing Sold on socket %d\n", i);
                                    sprintf(buffer_send, "ATM> %.2f", users[k].sold);
                                    send(i, buffer_send, BUFLEN, 0);
                                }
                            }
                            //Retragerea de fonduri
                        } else if (strcmp(token, "getmoney") == 0) {
                            char * aux_sum = strtok(NULL, " \n");
                            int sum = atoi(aux_sum);
                            for (int k = 0; k < users_num; k++) {
                                if (users[k].socket == i) {
                                    if (sum % 10 != 0) {
                                        printf("ATM> -9 : Suma nu este multiplu de 10\n");
                                        sprintf(buffer_send, "ATM> -9 : Suma nu este multiplu de 10");
                                        send(i, buffer_send, BUFLEN, 0);
                                    } else if (sum > users[k].sold) {
                                        printf("ATM> -8 : Fonduri insuficiente\n");
                                        sprintf(buffer_send, "ATM> -8 : Fonduri insuficiente");
                                        send(i, buffer_send, BUFLEN, 0);
                                    } else {
                                        printf("ATM> Suma %d retrasa cu succes\n", sum);
                                        sprintf(buffer_send, "ATM> Suma %d retrasa cu succes", sum);
                                        send(i, buffer_send, BUFLEN, 0);
                                        users[k].sold = users[k].sold - sum;
                                    }
                                }
                            }
                            //Depunere de fonduri
                        } else if (strcmp(token, "putmoney") == 0) {
                            char * aux_sum = strtok(NULL, " \n");
                            double sum = atof(aux_sum);
                            for (int k = 0; k < users_num; k++) {
                                if (users[k].socket == i) {
                                    printf("ATM> Suma depusa cu succes\n");
                                    sprintf(buffer_send, "ATM> Suma depusa cu succes");
                                    send(i, buffer_send, BUFLEN, 0);
                                    users[k].sold = users[k].sold + sum;
                                }
                            }
                            //Cand se primeste quit de la client se inchide socketul respectiv si se reseteaza variabliele logged_flag si socket din structura
                        } else if (strcmp(token, "quit") == 0) {
                            for (int k = 0; k < users_num; k++) {
                                if (users[k].socket == i) {
                                    users[k].socket = -1;
                                    users[k].logged_flag = 0;
                                }
                            }
                            close(i);
                            //Se elimina si din lista de file descriptori
                            FD_CLR(i, & read_fds);
                            FD_CLR(i, & tmp_fds);
                        }
                    }
                }
            }
        }
    }

    close(sockfd);

    return 0;
}
