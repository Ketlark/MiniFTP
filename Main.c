#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "string.h"
#include "TCP.h"
#include "TEA.h"
#include "DH.h"
#include "Request.h"
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

int handleRequest(uint32_t* keyData, socket_infos* connectionInfos, struct request* request) {
    char bufferRequest[1024];
    uint64_t* data = malloc(1024);

    int bytesReceived = read(connectionInfos->socketfd, (char*)data, 1024);
    printf("Size of request received : %d\n", bytesReceived);
    for (size_t i = 0; i < getBlocksCountFromSize(bytesReceived) - 1; i++) {
        decryptData(keyData, (uint32_t*)&data[i], 1, 0, 0);
    }

    *request = *(struct request*)data;
}

void sendPutRequest(uint32_t* keyData, socket_infos* connectionInfos, struct request* request, char* fileName, char* distName) {
    int file_fd;
    char bufferRequest[1024];
    int requestSize = -1;

    file_fd = open(fileName, O_RDWR);
    if(file_fd < 0) {
        perror("Erreur ouverture fichier pour put");
        exit(1);
    }

    request->kind = REQUEST_PUT;
    request->nbbytes = 45;
    strncpy (request->path, distName, strlen(distName));

    requestSize = sizeof(request->kind) + strlen(distName) + sizeof(request->nbbytes);
    int requestWithPadding = requestSize + (8 - (requestSize % 8));

    uint64_t* data = malloc(requestWithPadding);
    data = (uint64_t*)request;
    for (size_t i = 0; i < getBlocksCountFromSize(requestWithPadding) - 1; i++) {
        encryptData(keyData, (uint32_t*)&data[i], 1, 0, 0);
    }

    write(connectionInfos->socketfd, (char*)data, requestWithPadding);
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

    struct request request;
    struct answer answer;
    int typeRequest = -1;

    memset(key, 0x0, sizeof(uint128_t)); 
    memset(&connectionInfos, 0x0, sizeof(connectionInfos)); 

    int server_socket, side = -1;
    if(argc > 4) {
        char* adress = argv[1];

        if(strcmp(argv[2], "put") == 0) {
            typeRequest = REQUEST_PUT;
        } else {
            fprintf(stderr, "Argument de requête mal formulé : (put, get ou dir)\n");
            exit(1);
            return 1;
        }

        createClient(&side, adress, &connectionInfos);


    } else if(argc < 2) {
        server_socket = createServer(&side, &connectionInfos);
    } else {
        exit(2);
        return 1;
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
                   
                    printf("type : %d\n", request.kind);
                    printf("chemin : %s\n", request.path);

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
                        printf("sending put \n");
                        sendPutRequest((uint32_t*)key, &connectionInfos, &request, argv[3], argv[4]);
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

