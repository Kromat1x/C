#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "lib.h"

#define BUFLEN 256

void error(char * msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char * argv[]) {
    int sockfd, n, udpsock;
    int login = 0;
    int pass_awaited = 0;
    struct sockaddr_in serv_addr;
    struct hostent * server;
    char * last_login_nr;
    fd_set read_fds;
    fd_set tmp_fds;
    char * token;

    FILE * file;
    char * pid = calloc(10, 1);
    sprintf(pid, "%d", getpid());
    file = fopen(strcat(strcat(strdup("client-"), pid), strdup(".log")), "w");

    int fdmax;
    FD_ZERO( & read_fds);
    FD_ZERO( & tmp_fds);

    char buffer[BUFLEN];
    char buffer_rec[BUFLEN];

    if (argc < 3) {
        fprintf(stderr, "Usage %s server_address server_port\n", argv[0]);
        exit(0);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    udpsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    inet_aton(argv[1], & serv_addr.sin_addr);

    int len = sizeof(serv_addr);
    if (sockfd < 0)
        error("ERROR opening socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    FD_SET(sockfd, & read_fds);
    FD_SET(udpsock, & read_fds);
    FD_SET(0, & read_fds);
    fdmax = sockfd;
    if (udpsock > sockfd) {
        fdmax = udpsock;
    }

    if (connect(sockfd, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    while (1) {
        tmp_fds = read_fds;
        if (select(fdmax + 1, & tmp_fds, NULL, NULL, NULL) == -1)
            error("ERROR in select");

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, & tmp_fds)) {
                if (i == 0) {
                    //citesc de la tastatura
                    memset(buffer, 0, BUFLEN);
                    fgets(buffer, BUFLEN - 1, stdin);
                    fprintf(file, "%s", buffer);
                    char * aux_buf = strdup(buffer);
                    if (pass_awaited == 0) {
                      //Daca parola nu e asteptata, interpretam ce primim drept o comanda normala
                        if (strstr(buffer, "login") != NULL && login == 1) {
                          //Verificam din client daca nu cumva utilizatorul este deja logat pe sesiunea curenta
                            printf("-2 : Sesiune deja existenta\n");
                            fprintf(file, "-2 : Sesiune deja existenta\n");
                        } else {
                            if (strstr(buffer, "login") != NULL) {
                                token = strtok(aux_buf, " \n");
                                token = strtok(NULL, " \n");
                                last_login_nr = token;
                                //Retinem ultimul login pentru unlock
                            } else if (strstr(buffer, "logout") != NULL) {
                                login = 0;
                                //Cand dam logout resetam variabila de login la 0
                            }
                            if (strstr(buffer, "quit") != NULL) {
                                sprintf(buffer, "quit");
                                login = 0;
                                //Daca primim quit anuntam serverul, inchidem socketul si dam return 0 pentru a inchide clientul
                                n = send(sockfd, buffer, strlen(buffer), 0);
                                close(sockfd);
                                fclose(file);
                                return 0;
                            } else if (strstr(buffer, "unlock") != NULL) {
                                //Daca primim unlock atasam numarul de card si trimitem pe udp la server
                                sprintf(buffer, "unlock");
                                strcat(buffer, strdup(" "));
                                strcat(buffer, last_login_nr);
                                int x = sendto(udpsock, buffer, BUFLEN, 0, (struct sockaddr * ) & serv_addr, sizeof(serv_addr));

                            } else {
                                n = send(sockfd, buffer, strlen(buffer), 0);
                            }
                        }
                    } else {
                        //pass is awaited
                        //primim parola atasam din nou numarul de card si trimitem la server
                        token = strtok(buffer, "\n");
                        sprintf(buffer, "%s", token);
                        strcat(buffer, strdup(" "));
                        strcat(buffer, last_login_nr);
                        pass_awaited = 0;
                        int x = sendto(udpsock, buffer, BUFLEN, 0, (struct sockaddr * ) & serv_addr, sizeof(serv_addr));
                    }
                } else if (i == sockfd) {
                    //Daca primim de la server
                    n = recv(sockfd, buffer_rec, BUFLEN, 0);
                    if (n == 0) {
                        close(i);
                        fclose(file);
                        FD_CLR(i, & read_fds);
                        return 0;
                    } else {
                        printf("%s\n", buffer_rec);
                        if (strstr(buffer_rec, "quit") == NULL)
                            fprintf(file, "%s\n", buffer_rec);
                        if (strstr(buffer_rec, "Welcome") != NULL) {
                            //Daca primim Welcome inseamna ca ne-am logat cu succes
                            login = 1;
                        }
                        if (strstr(buffer_rec, "quit") != NULL) {
                            //Daca serverul da quit
                            login = 0;
                            close(sockfd);
                            return 0;
                        }
                    }
                } else if (i == udpsock) {
                    n = recvfrom(i, buffer_rec, BUFLEN, 0, (struct sockaddr * ) & serv_addr, & len);
                    printf("%s\n", buffer_rec);
                    fprintf(file, "%s\n", buffer_rec);
                    if (strstr(buffer_rec, "Trimite") != NULL) {
                        //we need to signal that we wait for password
                        pass_awaited = 1;
                    } else {
                        pass_awaited = 0;
                    }
                }
            }
        }
    }
    return 0;
}
