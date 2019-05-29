#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "string.h"
#include "TCP.h"
#include "TEA.h"
#include "DH.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int handleConnection(int* sockfd, struct sockaddr* sck, socklen_t* len) {
    //Le serveur accepte la connexion et prépare le fork
    int session_fd = accept(*sockfd, sck, len); 
    if (session_fd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0);  
    } else {
        printf("server acccept the client...\n"); 
    }

    return session_fd;
}

void processRequest() {
    printf("Process request\n");
    int bytesReceived = 0;

    /*while (1) {
        bytesReceived = read(session_fd, buff, 100);
        if(bytesReceived < 0) {
            perror("Lecture flux serveur");
        }

        if(bytesReceived > 0) {
            printf("Message : %s\n", buff);
        }
    }*/
}

int main(int argc, char *argv[])
{
    socket_infos connectionInfos;
    uint64_t key[2];

    memset(key, 0x0, sizeof(uint128_t)); 
    memset(&connectionInfos, 0x0, sizeof(connectionInfos)); 

    int server_socket, side = -1;
    if(argc > 1) {
        char* adress = argv[1];
        createClient(&side, adress, &connectionInfos);
    } else {
        server_socket = createServer(&side, &connectionInfos);
    }

    while(1) {
        if(side) {
            while(1) {
                int session = handleConnection(&server_socket, connectionInfos.sockaddr, &connectionInfos.length);
                connectionInfos.socketfd = session;

                if(fork()) {
                    printf("Traitement client : %d\n", session);
                    getKeyAsServer(connectionInfos.socketfd, key);
                    fflush(stdout);
                    sleep(20);
                }

            }
        } else {
            getKeyAsClient(connectionInfos.socketfd, key);
            printf("** Private Key : %llx **\n", key[0]);
            break;
        }
    }

    closeConnection(connectionInfos.socketfd);
    return 0;
}

