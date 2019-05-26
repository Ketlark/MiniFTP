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
#define PORT 4056 
#define SA struct sockaddr


/* On recherche un element qui réussisse les appels systèmes
 * socket et bind à la suite.
 * On return le file_desc du socket et la structure de l'élément correspondant
 */

int sock_bind(struct addrinfo *res, struct addrinfo *s) {
    int r, sfd = -1;
    int option = 1;
    while (res != NULL) {
        sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sfd > 0) {
            if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,(char*)&option,sizeof(option)) < 0) {
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

void sendData(int socketfd, char* buff, uint64_t length) {
    int result = write(socketfd, buff, length);
	if(result < 0) {
        perror("Ecriture dans le flux socket");
    }
}

void createClient(char* adress) { 
    int sockfd, connfd, len, r; 
    char buff[MAX]; 

    struct sockaddr_storage scksrv, sckclt;

    struct addrinfo *res, s;
    struct addrinfo criteria;

    memset(&criteria, 0, sizeof(struct addrinfo));
    criteria.ai_family = AF_UNSPEC;
    criteria.ai_socktype = SOCK_STREAM;
    criteria.ai_flags = AI_NUMERICSERV;
    criteria.ai_protocol = 0;
    criteria.ai_canonname = NULL;
    criteria.ai_addr = NULL;
    criteria.ai_next = NULL;

    // remplissage de &res par getaddrinfo
    r = getaddrinfo(adress, "4056", &criteria, &res);
    if (r != 0) {
        fprintf(stderr, "getaddrinfo fails : %s\n", gai_strerror(r));
        exit(2);
    }
    printf("Retrieve network informations successfuly !\n");
 
    sockfd = sock_bind(res, &s);
    if (sockfd == -1) {
        fprintf(stderr, "Fails binding. Exiting...\n");
        exit(3);
    }
    printf("Socket bind successfuly to server !\n");
  
    scksrv = *(struct sockaddr_storage *)(s.ai_addr);
    freeaddrinfo(res);

    //Connexion au serveur
    int result;
    if (result = connect(sockfd, (struct sockaddr *)&scksrv, sizeof(scksrv)) < 0){
		perror("connect");
		exit(4);
	}
    printf("Connected to server ! : code -> %d", result);

    fflush(stdout);
  
    len = sizeof(struct sockaddr_storage); 
  
    int bytesReceived = 0;
    char* buffer = "Bonsoir paris\0";

    sendData(sockfd, buffer, 15);
    while (1) {
        /*if ((bytesReceived = recv(sockfd, buff, MAX, 0)) == -1) {
            perror("recv fails");
        }*/
    }
  
    // After chatting close the socket 
    close(sockfd); 
} 


void createServer() { 
    int sockfd, connfd, len, r; 
    char buff[MAX]; 

    struct sockaddr_storage scksrv, sckclt;

    struct addrinfo *res, s;
    struct addrinfo criteria;

    memset(&criteria, 0, sizeof(struct addrinfo));
    criteria.ai_family = AF_UNSPEC;
    criteria.ai_socktype = SOCK_STREAM;
    criteria.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    criteria.ai_protocol = 0;
    criteria.ai_canonname = NULL;
    criteria.ai_addr = NULL;
    criteria.ai_next = NULL;

    // remplissage de &res par getaddrinfo
    r = getaddrinfo(NULL, "4056", &criteria, &res);
    if (r != 0) {
        fprintf(stderr, "getaddrinfo fails : %s\n", gai_strerror(r));
        exit(2);
    }
    printf("Retrieve network informations successfuly !\n");

    sockfd = sock_bind(res, &s);
    if (sockfd == -1) {
        fprintf(stderr, "Fails binding. Exiting...\n");
        exit(3);
    }
    printf("Socket bind successfuly !\n");

    fflush(stdout);
  
    scksrv = *(struct sockaddr_storage *)(s.ai_addr);
    freeaddrinfo(res);

    if (listen(sockfd, 32) != 0) { //On autorise une file d'attente de 32 connexion maximum
        perror("listen");
        exit(4);
    }
  
    len = sizeof(struct sockaddr_storage); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (SA*)&sckclt, &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0);  
    } else {
        printf("server acccept the client...\n"); 
    }

    printf("Un client ??");
    fflush(stdout);
  
    int bytesReceived = 0;
    while (1) {
        bytesReceived = read(sockfd, buff, 100);
        if(bytesReceived < 0) {
            perror("Lecture flux serveur");
        }

        if(bytesReceived > 0) {
            printf("Message : %s", buff);
        }
    }
  
    // After chatting close the socket 
    close(sockfd); 
} 