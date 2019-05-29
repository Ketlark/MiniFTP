#include <stdint.h>

#ifndef TEA_H_   /* Include guard */
#define TEA_H_

int getKey(int sockfd, uint64_t* key, int server);
int getKeyAsClient(int sockfd, uint64_t* key);
int getKeyAsServer(int sockfd, uint64_t* key);

#endif