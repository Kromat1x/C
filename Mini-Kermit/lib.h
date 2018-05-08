#ifndef LIB
#define LIB

typedef struct {
    int len;
    char payload[1400];
} msg;

typedef struct {
    u_char SOH;
    u_char LEN;
    u_char SEQ;
    u_char TYPE;
    unsigned short CHECK;
    u_char MARK;
} kerm_pack;

typedef struct {
    u_char MAXL;
    u_char TIME;
    u_char NPAD;
    u_char PADC;
    u_char EOL;
    u_char QCTL;
    u_char QBIN;
    u_char CHKT;
    u_char REPT;
    u_char CAPA;
    u_char R;
} init_pack;

void init(char* remote, int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
msg* receive_message_timeout(int timeout); //timeout in milliseconds
unsigned short crc16_ccitt(const void *buf, int len);

#endif
