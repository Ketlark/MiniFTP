#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "Common.h"
#include "TCP.h"
#include "TEA.h"
#include "Request.h"

int isBigEndian() {
    unsigned int x = 0x76543210;
    char *c = (char*) &x;
    
    if (*c == 0x10) {
        return 0;
    }
    else {
        return 1;
    }
}

int handleConnection(int* sockfd, struct sockaddr* sck, socklen_t* len) {
    int session_fd = accept(*sockfd, sck, len); 
    if (session_fd < 0) { 
        printf("\n---------------------------------\nserver acccept failed...\n"); 
        exit(0);  
    } else {
        printf("\n---------------------------------\nserver acccept the client...\n"); 
    }

    return session_fd;
}

void handleError(struct answer* answer) {
    
}

void sendList(uint32_t* keyData, socket_infos* connectionInfos, int pipefd) {
    uint32_t datablock[2];
    int blockReaded = 0;

    while(1) {
        blockReaded = read(pipefd, datablock, sizeof(uint64_t));
        encryptData(keyData, datablock, 0, 0, 0);
        if(blockReaded == 0) {
            break; //EOF
        }

        if(write(connectionInfos->socketfd, datablock, sizeof(uint64_t)) < 0) {
            perror("write stdout");
            exit(2);
        }
    }

    close(pipefd);
}

void getList(uint32_t* keyData, socket_infos* connectionInfos) {
    uint32_t datablock[2];
    int bytesReceived = 0;
    int flags = fcntl(connectionInfos->socketfd, F_GETFL, 0);

    while(1) {
        bytesReceived = recv(connectionInfos->socketfd, datablock, sizeof(uint64_t), MSG_WAITALL);

        if(bytesReceived < 8) {
            break; //EOF
        }

        decryptData(keyData, datablock, 0, 0, 0);
        if(write(1, (char*)datablock, sizeof(uint64_t)) < 0) {
            perror("write stdout");
            exit(2);
        }
        fflush(stdout);

        fcntl(connectionInfos->socketfd, F_SETFL, flags | O_NONBLOCK);
    }
}

int getFile(uint32_t* keyData, socket_infos* connectionInfos, int file_fd, int size, int padding) {
    uint32_t datablock[2];
    int bytesReceived = 0;

    printf("Blocks to receive : %d\n", getBlocksCountFromSize(size) + 1);
    for (size_t i = 0; i < getBlocksCountFromSize(size) + 1; i++){

        bytesReceived = read(connectionInfos->socketfd, datablock, sizeof(uint64_t));

        if(bytesReceived == 0) {
            break; //EOF
        }

        if(i + 1 >= getBlocksCountFromSize(size) + 1) {
            decryptData(keyData, datablock, 0, 0, 0);
            if(write(file_fd, datablock, sizeof(uint64_t) - padding) < 0) {
                perror("write");
                exit(2);
            }  
        } else {
            decryptData(keyData, datablock, 0, 0, 0);
            if(write(file_fd, datablock, sizeof(uint64_t)) < 0) {
                perror("write");
                exit(2);
            }  
        }
    }

    close(file_fd);
}

int sendFile(uint32_t* keyData, socket_infos* connectionInfos, int file_fd, int size, int padding) {
    uint32_t datablock[2];
    int bytesReceived = 0;

    printf("Blocks to send : %d\n", getBlocksCountFromSize(size) + 1);
    for (size_t i = 0; i < getBlocksCountFromSize(size) + 1; i++){

        bytesReceived = read(file_fd, datablock, sizeof(uint64_t));

        if(bytesReceived == 0) {
            break; //EOF
        }

        encryptData(keyData, datablock, 0, 0, 0);

        if(write(connectionInfos->socketfd, datablock, sizeof(uint64_t)) < 0) {
            perror("write");
            exit(2);
        }  
    }

    close(file_fd);
}


void handleRequest(uint32_t* keyData, socket_infos* connectionInfos, struct request* request) {
    uint64_t* data = (uint64_t*) calloc(1024, 8);

    int bytesReceived = read(connectionInfos->socketfd, (char*)data, 1024);
    printf("Size of request received : %d\n", bytesReceived);
    for (size_t i = 0; i < getBlocksCountFromSize(bytesReceived) - 1; i++) {
        decryptData(keyData, (uint32_t*)&data[i], 0, 0, 0);
    }

    *request = *(struct request*)data;

    printf("\n********\n");
    printf("Requete : %d\n", request->kind);
    printf("chemin : %s\n", request->path);
    printf("poids : %d\n\n", request->nbbytes);

    struct answer answer;
    if(request->kind == REQUEST_GET) {
        int file_fd = open(request->path, O_RDWR, 0777);
        if(file_fd < 0) {
            perror("Open dist file (GET)");
        }
        printf("File %s - descriptor for get : %d\n", request->path, file_fd);

        struct stat fileStat;
        if(fstat(file_fd, &fileStat) < 0) {
            printf("Impossible de lire les informations du fichier");
            exit(5);
        }

        answer.ack = ANSWER_OK;
        answer.errnum = 0;
        answer.nbbytes = fileStat.st_size;
        answer._pad[0] = abs((fileStat.st_size % 8) - 8);

        sendAnswer(keyData, connectionInfos, request->kind, answer);
        printf("\n*** Answer sended \n");
        printf("size : %d\n", answer.nbbytes);
        printf("padding : %d\n", answer._pad[0]);

        sendFile(keyData, connectionInfos, file_fd, answer.nbbytes, answer._pad[0]);
    } else if (request->kind == REQUEST_PUT) {
        int file_fd = open(request->path, O_CREAT | O_RDWR , 0777);
        if(file_fd < 0) {
            perror("Create dist file (PUT)");
        }
        printf("File %s - descriptor for put : %d\n", request->path, file_fd);

        answer.ack = ANSWER_OK;
        answer.errnum = 0;
        answer.nbbytes = 0;
        answer._pad[0] = 0;

        sendAnswer(keyData, connectionInfos, request->kind, answer);
        printf("\n*** Answer sended \n");
        printf("size : %d\n", answer.nbbytes);
        printf("padding : %d\n", answer._pad[0]);

        getFile(keyData, connectionInfos, file_fd, request->nbbytes, abs((request->nbbytes % 8) - 8));
    } else if (request->kind == REQUEST_DIR) {
        answer.ack = ANSWER_OK;
        answer.errnum = 0;
        answer.nbbytes = 0;
        answer._pad[0] = 0;


        if(open(request->path, O_RDONLY) < 0) {
            printf("Path argument for ls is wrong\n");
            answer.ack = ANSWER_ERROR;
            answer.errnum = errno;
        }

        sendAnswer(keyData, connectionInfos, request->kind, answer);
        printf("\n*** Answer sended \n");
        printf("size : %d\n", answer.nbbytes);
        printf("padding : %d\n", answer._pad[0]);
    
        int pipefd[2];
        pipe(pipefd);
        if (fork() == 0) {
            close(pipefd[0]);    // close reading end in the child

            dup2(pipefd[1], 1);  // send stdout to the pipe
            close(pipefd[1]); 

            execl("/bin/ls", "ls", "-l", request->path);
        } else {
            close(pipefd[1]);
        }

        if(answer.ack == ANSWER_OK) sendList(keyData, connectionInfos, pipefd[0]);

    } else {
        printf("\nBit strange request, aborting ..");
        
        answer.ack = ANSWER_ERROR;
        answer.errnum = 0;
        answer.nbbytes = 0;
        answer._pad[0] = 0;

        sendAnswer(keyData, connectionInfos, request->kind, answer);
    }

    free(data);
}

void handleAnswer(uint32_t* keyData, socket_infos* connectionInfos, int typeRequest, struct answer* answer, char* fileName) {
    uint64_t* data = (uint64_t*) calloc(1024, 8);

    int bytesReceived = read(connectionInfos->socketfd, (char*)data, 1024);
    printf("Size of answer received : %d\n", bytesReceived);

    for (size_t i = 0; i < getBlocksCountFromSize(bytesReceived) - 1; i++) {
        if(i + 1 >= getBlocksCountFromSize(bytesReceived) - 1) {
            decryptData(keyData, (uint32_t*)&data[i], 0, 0, 0);
        } else {
            decryptData(keyData, (uint32_t*)&data[i], 0, 0, 0);
        }
    }

    *answer = *(struct answer*)data;


    if(answer->ack == ANSWER_UNKNOWN || answer->ack == ANSWER_ERROR) {
        printf("\n********\n");
        printf("Answer is : %d\n", answer->ack);
        if(answer->ack == ANSWER_ERROR) printf("Error number : %s\n", strerror(answer->errnum));
        printf("End of communication ..\n\n");
        return;
    } else {
        printf("\n********\n");
        printf("Answer ack : %d\n", answer->ack);
        printf("Size : %d\n", answer->nbbytes);
        printf("Padding : %d\n", answer->_pad[0]);
        printf("Errno : %d\n\n", answer->errnum);
        printf("Request kind : %d\n\n", typeRequest);
    }

    if(typeRequest == REQUEST_PUT) {
        printf("File %s - to put\n", fileName);
        int file_fd = open(fileName, O_RDWR, 0777);
        if(file_fd < 0) {
            perror("Open to put file");
            exit(8);
        }

        struct stat fileStat;
        if(fstat(file_fd, &fileStat) < 0) {
            printf("Impossible de lire les informations du fichier");
            exit(5);
        }

        sendFile(keyData, connectionInfos, file_fd, fileStat.st_size, abs((fileStat.st_size % 8) - 8));
        
    } else if (typeRequest == REQUEST_GET) {
        printf("File %s - to get\n", fileName);
        int file_fd = open(fileName, O_CREAT|O_RDWR, 0777);
        if(file_fd < 0) {
            perror("Open to get file");
            exit(8);
        }

        getFile(keyData, connectionInfos, file_fd, answer->nbbytes, answer->_pad[0]);
    } else if(typeRequest == REQUEST_DIR) {
        printf("\nDirectory answer received\n");
        getList(keyData, connectionInfos);
    }
}

void sendRequest(uint32_t* keyData, socket_infos* connectionInfos, int type, struct request* request, char* fileName, char* distName) {
    int file_fd;
    int requestSize = -1;

    request->kind = type;
    snprintf(request->path, strlen(distName) + 1, distName);
    request->nbbytes = 0;

    if(type == REQUEST_PUT) {
        file_fd = open(fileName, O_RDONLY);
        if(file_fd < 0) {
            perror("Erreur ouverture fichier pour put");
            exit(1);
        }

        struct stat fileStat;
        if(fstat(file_fd, &fileStat) < 0) {
            printf("Impossible de lire les informations du fichier");
            exit(5);
        }

        printf("Size of file to put : %d\n", fileStat.st_size);
        request->nbbytes = fileStat.st_size;
    }

    requestSize = sizeof(struct request);
    int requestWithPadding = requestSize + (8 - (requestSize % 8));

    uint64_t* data = (uint64_t*) malloc(requestWithPadding);
    data = (uint64_t*)request;
    for (size_t i = 0; i < getBlocksCountFromSize(requestWithPadding) - 1; i++) {
        encryptData(keyData, (uint32_t*)&data[i], 0, 0, 0);
    }

    int sended = write(connectionInfos->socketfd, (char*)data, requestWithPadding);
}

void sendAnswer(uint32_t* keyData, socket_infos* connectionInfos, int typeRequest, struct answer answer) {
    int requestSize = -1;

    int answerWithPadding = sizeof(struct answer) + (8 - (sizeof(struct answer) % 8));

    uint64_t* data = (uint64_t*) malloc(answerWithPadding);
    data = (uint64_t*)&answer;
    for (size_t i = 0; i < getBlocksCountFromSize(answerWithPadding) - 1; i++) {
        if(i+1 >= getBlocksCountFromSize(answerWithPadding) - 1) {
            encryptData(keyData, (uint32_t*)&data[i], 8 - (sizeof(struct answer) % 8), 1, 0);
        } else {
            encryptData(keyData, (uint32_t*)&data[i], 0, 0, 0);
        }
    }

    int sended = write(connectionInfos->socketfd, (char*)data, answerWithPadding);
}
