#include <sys/socket.h>

#ifndef TCP_H_   /* Include guard */
#define TCP_H_

typedef struct socket_infos {
    int socketfd;
    struct sockaddr* scksrv;
    socklen_t lenght;
    int maximumBuffer;
} socket_infos;

void createClient(char* adress);
void createServer();

#endif