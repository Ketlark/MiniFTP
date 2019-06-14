#include <stdint.h>
#include <string.h>
#include "TCP.h"
#include "Request.h"

#ifndef COMMON_H_   /* Include guard */
#define COMMON_H_

int isBigEndian();

int handleConnection(int* sockfd, struct sockaddr* sck, socklen_t* len);

void handleRequest(uint32_t* keyData, socket_infos* connectionInfos, struct request* request);
void sendRequest(uint32_t* keyData, socket_infos* connectionInfos, int type, struct request* request, char* fileName, char* distName);

void handleAnswer(uint32_t* keyData, socket_infos* connectionInfos, int typeRequest, struct answer* answer, char* fileName);
void sendAnswer(uint32_t* keyData, socket_infos* connectionInfos, int typeRequest, struct answer answer);

#endif // COMMON_H_