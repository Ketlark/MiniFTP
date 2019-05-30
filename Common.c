#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
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

uint64_t htonll(uint64_t value)
{
     int num = 42;
     if(isBigEndian == 0)
          return ((uint64_t)htonl(value & 0xFFFFFFFF) << 32LL) | htonl(value >> 32);
     else 
          return value;
}

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

int processTransfer(uint32_t* keyData, socket_infos* connectionInfos, int file_fd) {

}

int handleRequest(uint32_t* keyData, socket_infos* connectionInfos, struct request* request) {
    uint64_t* data = (uint64_t*) calloc(1024, 8);

    int bytesReceived = read(connectionInfos->socketfd, (char*)data, 1024);
    printf("Size of request received : %d\n", bytesReceived);
    for (size_t i = 0; i < getBlocksCountFromSize(bytesReceived) - 1; i++) {
        if(i + 1 >= getBlocksCountFromSize(bytesReceived) - 1) {
            decryptData(keyData, (uint32_t*)&data[i], 0, 1, 0);
        } else {
            decryptData(keyData, (uint32_t*)&data[i], 0, 0, 0);
        }
    }

    *request = *(struct request*)data;

    int errno;
    printf("Requete : %d\n", request->kind);
    printf("chemin : %s\n", request->path);
    printf("poids : %d\n", request->nbbytes);

    if(request->kind == REQUEST_GET) {
        int file_fd = open(request->path, O_RDWR);
        if(file_fd < 0) {
            perror("Open dist file (GET)");
        }

        struct stat fileStat;
        if(fstat(file_fd, &fileStat) < 0) {
            printf("Impossible de lire les informations du fichier");
            exit(5);
        }

        struct answer answer;
        answer.ack = ANSWER_OK;
        answer.errnum = 0;
        answer.nbbytes = fileStat.st_size;
        answer._pad[1] = abs((fileStat.st_size % 8) - 8);

        sendAnswer(keyData, connectionInfos, request->kind, &answer);
        printf("Answer sended \n");
    }

    free(data);
}

int handleAnswer(uint32_t* keyData, socket_infos* connectionInfos, struct answer* answer) {
    uint64_t* data = (uint64_t*) calloc(1024, 8);

    int bytesReceived = read(connectionInfos->socketfd, (char*)data, 1024);
    printf("Size of answer received : %d\n", bytesReceived);

    for (size_t i = 0; i < getBlocksCountFromSize(bytesReceived) - 1; i++) {
        if(i + 1 >= getBlocksCountFromSize(bytesReceived) - 1) {
            decryptData(keyData, (uint32_t*)&data[i], 0, 1, 0);
        } else {
            decryptData(keyData, (uint32_t*)&data[i], 0, 0, 0);
        }
    }

    *answer = *(struct answer*)data;
}

void sendRequest(uint32_t* keyData, socket_infos* connectionInfos, int type, struct request* request, char* fileName, char* distName) {
    int file_fd;
    int requestSize = -1;

    request->kind = type;
    request->nbbytes = 0;

    if(type == REQUEST_PUT) {
        file_fd = open(fileName, O_RDWR);
        if(file_fd < 0) {
            perror("Erreur ouverture fichier pour put");
            exit(1);
        }

        struct stat fileStat;
        if(fstat(file_fd, &fileStat) < 0) {
            printf("Impossible de lire les informations du fichier");
            exit(5);
        }

        request->nbbytes = fileStat.st_size;
    }

    strncpy (request->path, distName, strlen(distName));

    requestSize = sizeof(request->kind) + strlen(distName) + sizeof(request->nbbytes);
    int requestWithPadding = requestSize + (8 - (requestSize % 8));

    uint64_t* data = (uint64_t*) malloc(requestWithPadding);
    data = (uint64_t*)request;
    for (size_t i = 0; i < getBlocksCountFromSize(requestWithPadding) - 1; i++) {
        if(i+1 >= getBlocksCountFromSize(requestWithPadding) - 1) {
            encryptData(keyData, (uint32_t*)&data[i], 8 - (requestSize % 8), 1, 0);
        } else {
            encryptData(keyData, (uint32_t*)&data[i], 0, 0, 0);
        }
    }

    int sended = write(connectionInfos->socketfd, (char*)data, requestWithPadding);
}

void sendAnswer(uint32_t* keyData, socket_infos* connectionInfos, int typeRequest, struct answer* answer) {
    int requestSize = -1;

    int answerWithPadding = sizeof(struct answer) + (8 - (sizeof(struct answer) % 8));

    uint64_t* data = (uint64_t*) malloc(answerWithPadding);
    data = (uint64_t*)answer;
    for (size_t i = 0; i < getBlocksCountFromSize(answerWithPadding) - 1; i++) {
        if(i+1 >= getBlocksCountFromSize(answerWithPadding) - 1) {
            encryptData(keyData, (uint32_t*)&data[i], 8 - (sizeof(struct answer) % 8), 1, 0);
        } else {
            encryptData(keyData, (uint32_t*)&data[i], 0, 0, 0);
        }
    }

    int sended = write(connectionInfos->socketfd, (char*)data, answerWithPadding);
}
