#include <sys/socket.h>
#include <stdint.h>

#ifndef TCP_H_   /* Include guard */
#define TCP_H_

typedef struct socket_infos {
    int socketfd;
    struct sockaddr* sockaddr;
    socklen_t length;
} socket_infos;

int createClient(int* side, char* adress, socket_infos* infos);
int createServer(int* side, socket_infos* infos);
void closeConnection(int socket);

void sendData(int socketfd, char* buff, uint64_t length);
int readData(int socketfd, char* buff, uint64_t length);

#endif