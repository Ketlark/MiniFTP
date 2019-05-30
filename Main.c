#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "TCP.h"
#include "TEA.h"
#include "DH.h"
#include "Request.h"
#include "Common.h"

int main(int argc, char *argv[])
{
    socket_infos connectionInfos;
    uint64_t key[2];

    struct request request;
    struct answer answer;
    int typeRequest = -1;

    memset(key, 0x0, sizeof(uint128_t)); 
    memset(&connectionInfos, 0x0, sizeof(connectionInfos)); 

    int server_socket, side = -1;
    if(argc > 4) {
        char* adress = argv[1];

        if(strcmp(argv[2], "get") == 0) {
            typeRequest = REQUEST_GET;
        } else if(strcmp(argv[2], "put") == 0) {
            typeRequest = REQUEST_PUT;
        } else if(strcmp(argv[2], "dir") == 0) {
            typeRequest = REQUEST_DIR;
        } else {
            fprintf(stderr, "Argument de requête mal formulé : (put, get ou dir)\n");
            exit(1);
        }

        createClient(&side, adress, &connectionInfos);

    } else if(argc < 2) {
        server_socket = createServer(&side, &connectionInfos);
    } else {
        exit(2);
    }

    while(1) {
        if(side) {
            while(1) {
                int session = handleConnection(&server_socket, connectionInfos.sockaddr, &connectionInfos.length);
                connectionInfos.socketfd = session;

                if(fork()) {
                    printf("Traitement client : %d\n", session);
                    getKeyAsServer(connectionInfos.socketfd, key);
                    printf("** Private Key : %llx%llx **\n", key[0], key[1]);

                    //RECV
                    struct request request;
                    handleRequest((uint32_t*)key, &connectionInfos, &request);

                    fflush(stdout);
                    sleep(20);
                }

            }
        } else {
            getKeyAsClient(connectionInfos.socketfd, key);
            printf("** Private Key : %llx%llx **\n", key[0], key[1]);

            if(typeRequest != -1) {
                switch (typeRequest) {
                    case REQUEST_PUT: {
                        printf("sending put : %d\n", typeRequest);
                        sendRequest((uint32_t*)key, &connectionInfos, typeRequest, &request, argv[3], argv[4]);
                        handleAnswer((uint32_t*)key, &connectionInfos, &answer);

                        printf("Réponse serveur : \n");
                        printf("Type : %d\n", answer.ack);
                        printf("Taille : %d\n", answer.nbbytes);
                        break;
                    }

                    case REQUEST_GET: {
                        printf("sending get : %d\n", typeRequest);
                        sendRequest((uint32_t*)key, &connectionInfos, typeRequest, &request, argv[4], argv[3]);
                        handleAnswer((uint32_t*)key, &connectionInfos, &answer);

                        printf("Réponse serveur : \n");
                        printf("Type : %d\n", answer.ack);
                        printf("Taille : %d\n", answer.nbbytes);
                        break;
                    }
                }
            }
            break;
        }
    }

    closeConnection(connectionInfos.socketfd);
    return 0;
}

