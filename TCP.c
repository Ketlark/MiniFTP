#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include "TCP.h"

#define MAX 1024 
#define PORT "8006" 
#define SA struct sockaddr


//Server socket connect
int server_bind(struct addrinfo *res, struct addrinfo *s) {
    int r, sfd = -1;
    int option = 1;
    while (res != NULL) {
        sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sfd > 0) {
            if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option)) < 0) {
                printf("setsockopt failed\n");
                close(sfd);
                exit(2);
            }
            r = bind(sfd, res->ai_addr, res->ai_addrlen);
            if (r == 0) {
                *s = *res;
                break;
            }
            else {
                close(sfd);
                sfd = -1;
            }
        }
        res = res->ai_next;
    }
    return (sfd);
}

//Client socket connect
int client_bind(struct addrinfo *res, struct addrinfo *s){
  int sfd=-1;
  while(res!=NULL) {
    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd > 0) {
        *s = *res;
        break;
    }
    res = res->ai_next;
  }
  return(sfd);
}

void sendData(int socketfd, char* buff, uint64_t length) {
    int result = write(socketfd, buff, length);
	if(result < 0) {
        perror("Ecriture dans le flux socket");
    }
}

void readData(int socketfd, char* buff, uint64_t length) {
    int result = read(socketfd, buff, length);
	if(result < 0) {
        perror("Lecture dans le flux socket");
    }
}

int createClient(int* side, char* adress, socket_infos* infos) { 
    *side = 0;
    int sockfd, len; 
    char buff[MAX]; 

    struct sockaddr_storage scksrv, sckclt;

    struct addrinfo *res, s;
    struct addrinfo criteria;
    

    memset(&criteria, 0, sizeof(struct addrinfo));
    criteria.ai_family = AF_UNSPEC;
    criteria.ai_socktype = SOCK_STREAM;
    criteria.ai_flags = AI_NUMERICSERV;
    criteria.ai_protocol = 0;

    // remplissage de &res par getaddrinfo
    int r;
    if (r = getaddrinfo(adress, PORT, &criteria, &res) != 0) {
        fprintf(stderr, "getaddrinfo fails : %s\n", gai_strerror(r));
        exit(2);
    }
    printf("Retrieve network informations successfuly !\n");
 
    sockfd = client_bind(res, &s);
    if (sockfd == -1) {
        fprintf(stderr, "Fails binding. Exiting...\n");
        exit(3);
    }
    printf("Got socket descriptor ...\n");
  
    scksrv = *(struct sockaddr_storage *)(s.ai_addr);
    freeaddrinfo(res);

    //Connexion au serveur
    if (connect(sockfd, (struct sockaddr *)&scksrv, sizeof(scksrv)) < 0){
		perror("connect");
		exit(4);
	}
  
    len = sizeof(struct sockaddr_storage); 
  
    int bytesReceived = 0;
    char* buffer = "Bonsoir paris\0";

    infos->socketfd = sockfd;
    infos->sockaddr = (SA*)&scksrv;
    infos->length = sizeof(scksrv);
  
    return sockfd; 
} 

int createServer(int* side, socket_infos* infos) { 
    int server_fd, session_fd, len, r; 
    char buff[MAX]; 

    struct sockaddr_storage scksrv, sckclt;

    struct addrinfo *res, s;
    struct addrinfo criteria;

    memset(&criteria, 0, sizeof(struct addrinfo));
    criteria.ai_family = AF_UNSPEC;
    criteria.ai_socktype = SOCK_STREAM;
    criteria.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    criteria.ai_protocol = 0;

    // remplissage de &res par getaddrinfo
    r = getaddrinfo(NULL, PORT, &criteria, &res);
    if (r != 0) {
        fprintf(stderr, "getaddrinfo fails : %s\n", gai_strerror(r));
        exit(2);
    }
    printf("Retrieve network informations successfuly !\n");

    server_fd = server_bind(res, &s);
    if (server_fd == -1) {
        fprintf(stderr, "Fails binding. Exiting...\n");
        exit(3);
    }
    printf("Socket bind successfuly !\n");
  
    scksrv = *(struct sockaddr_storage *)(s.ai_addr);
    freeaddrinfo(res);

    if (listen(server_fd, 32) != 0) { //On autorise une file d'attente de 32 connexion maximum
        perror("listen");
        exit(4);
    }
  
    len = sizeof(struct sockaddr_storage); 

    infos->sockaddr = (SA*)&sckclt;
    infos->length = len;

    return server_fd;
} 

void closeConnection(int socket) {
    close(socket);
}