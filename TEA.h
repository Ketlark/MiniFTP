#include <stdint.h>

#ifndef TEA_H_   /* Include guard */
#define TEA_H_

int getKey(int sockfd, uint64_t* key, int server);
int getKeyAsClient(int sockfd, uint64_t* key);
int getKeyAsServer(int sockfd, uint64_t* key);

size_t getBlocksCountFromSize(size_t size);

void encryptData(uint32_t* keyData, uint32_t* datablock, int size, int lastBlock, short endianness);
void decryptData(uint32_t* keyData, uint32_t* datablock, int size, int lastBlock, short endianness);

#endif