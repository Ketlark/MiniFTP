#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <math.h>
#include "Common.h"
#include <endian.h>
#include "DH.h"
#include "TEA.h"
#include "TCP.h"

#define DELTA 0x9e3779b9  /* a key schedule constant */
#define BFSIZE 512

int getKey(int sockfd, uint64_t* key, int server) {
    DHKeyInfos dh_infos;
    uint64_t privateNumber = 0;
    uint64_t publicNumberOther = 0; //B 
    size_t k;

    //Première étape DH :
    //Le client envoie A, g et p au serveur
    //Le server reçoie A, g et p et génère B et b
    if(server) {
        readData(sockfd, (char*)&dh_infos, sizeof(DHKeyInfos));
        publicNumberOther = dh_infos.publicNumber;
        generateKeyFirstStep(server, &privateNumber, &dh_infos.publicNumber, &dh_infos.primeNumber, &dh_infos.primitiveRoot);
    } else {
        generateKeyFirstStep(server, &privateNumber, &dh_infos.publicNumber, &dh_infos.primeNumber, &dh_infos.primitiveRoot);
        sendData(sockfd, (char*)&dh_infos, sizeof(DHKeyInfos));
    }

    if(server) {
        generateKeySecondStep(key, &privateNumber, &publicNumberOther, &dh_infos.primeNumber);
        sendData(sockfd, (char*)&dh_infos.publicNumber, sizeof(uint64_t));
    }   
    else {
        readData(sockfd, (char*)&publicNumberOther, sizeof(uint64_t));
        generateKeySecondStep(key, &privateNumber, &publicNumberOther, &dh_infos.primeNumber);
    }
}

int getKeyAsClient(int sockfd, uint64_t* key) {
    uint64_t k1, k2;

    getKey(sockfd, &k1, 0);
    getKey(sockfd, &k2, 0);

    key[0] = k1;
    key[1] = k2;

    return 0;
}

int getKeyAsServer(int sockfd, uint64_t* key) {
    uint64_t k1, k2;

    getKey(sockfd, &k1, 1);
    getKey(sockfd, &k2, 1);

    key[0] = k1;
    key[1] = k2;

    return 0;
}

void decrypt (uint32_t* v, uint32_t* k) {
    uint32_t v0=v[0], v1=v[1], sum= DELTA * 32, i;           /* set up */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i < 32; i++) {                       /* basic cycle start */
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= DELTA;
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}

void encrypt (uint32_t* v, uint32_t* k) {
    uint32_t v0=v[0], v1=v[1], sum=0, i;           /* set up */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i < 32; i++) {                       /* basic cycle start */
        sum += DELTA;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}

size_t getBlocksCountFromSize(size_t size) {
    size_t count;

    count = size / (size_t)(sizeof(uint64_t));
    if(size % (size_t)(sizeof(uint64_t)) == 0) count++;
    
    return count;
}

void writePadding(char* block, size_t padding) {
    char paddingChar[8];
    memset(paddingChar, 0x0, 8);
    paddingChar[7] = padding == 0 ? 8 : padding; 
    for (size_t i = 0; i < 8; i++) {
        block[i] |= paddingChar[i];
    }
}

void deletePadding(char* block, size_t padding) {
    int sizeFinalBlock = 8 - padding;
    char finalBlock[sizeFinalBlock];
    memset(finalBlock, 0x0, sizeFinalBlock);

    for (size_t i = 0; i < sizeFinalBlock; i++) {
        finalBlock[i] = block[i];
    }

    block = finalBlock;
}

size_t readPadding(uint64_t* block) {
    return be64toh(*block) & 0x00000000000000FF;
}

void encryptData(uint32_t* keyData, uint32_t* datablock, int padding, int lastBlock, short endianness) {
    if(lastBlock > 0) {
        writePadding((char*)datablock, padding);
    }
    encrypt(datablock, keyData);
}

void decryptData(uint32_t* keyData, uint32_t* datablock, int size, int lastBlock, short endianness) {
    decrypt(datablock, keyData);
    if(lastBlock > 0) {
        int padding = readPadding((uint64_t*)datablock);
        printf("Padding decrypt : %d\n", padding);
        deletePadding((char*)datablock, padding);
    }
}
