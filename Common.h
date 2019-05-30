#include <stdint.h>
#include <string.h>
#include "TCP.h"
#include "Request.h"

#ifndef COMMON_H_   /* Include guard */
#define COMMON_H_

int isBigEndian();
uint64_t htonll();

int handleConnection(int* sockfd, struct sockaddr* sck, socklen_t* len);

int handleRequest(uint32_t* keyData, socket_infos* connectionInfos, struct request* request);
void sendRequest(uint32_t* keyData, socket_infos* connectionInfos, int type, struct request* request, char* fileName, char* distName);

int handleAnswer(uint32_t* keyData, socket_infos* connectionInfos, struct answer* answer);
void sendAnswer(uint32_t* keyData, socket_infos* connectionInfos, int typeRequest, struct answer* answer);

#endif // COMMON_H_