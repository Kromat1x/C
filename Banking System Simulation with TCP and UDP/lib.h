#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


typedef struct {
  char* nume;
  char* prenume;
  char* nr_card;
  char* pin;
  char* secret_pass;
  double sold;
  int mistakes;
  int blocked;
  int socket;
  int logged_flag;
} card_data;

//typedef struct {
//  card_data card;
//int socket;
//}
