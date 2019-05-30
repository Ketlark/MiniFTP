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

size_t readPadding(uint64_t* block) {
    if(isBigEndian() == 0) {
        return htobe64(*block) & 0x00000000000000FF;
    } else {
        return *block & 0x00000000000000FF;
    }
}

void encryptData(uint32_t* keyData, uint32_t* datablock, int size, int lastBlock, short endianness) {
    int padding = 0;
    if(((float)size / 8) != 0) {
       // abs((fileStat.st_size % 8) - 8)
    }

    if(padding > 0) {
        writePadding((char*)datablock, padding);
    }
    encrypt(datablock, keyData);
}

void decryptData(uint32_t* keyData, uint32_t* datablock, int size, int lastBlock, short endianness) {
    decrypt(datablock, keyData);
}

void encryptFile(int inFD, int outFD, uint32_t* keyData, short endianness) {
    uint32_t datablock[2];
    int bytesReaded = 0;
    int offsetBlock = 0;

    struct stat fileStat;
    if(fstat(inFD, &fileStat) < 0) {
        printf("Impossible de lire les informations du fichier");
        exit(5);
    }

    while(1) {
        memset(datablock, 0x0, sizeof(uint64_t));
        bytesReaded = read(inFD, datablock, sizeof(uint64_t));

        if(bytesReaded == 0){
            break; //EOF
        }

        if(offsetBlock >= fileStat.st_size / 8) {
            if(((float)fileStat.st_size / 8) != 0) {
                writePadding((char*)datablock, abs((fileStat.st_size % 8) - 8));
            }
        }


        encrypt(datablock, keyData);

        if(write(outFD, datablock, sizeof(uint64_t)) ==-1) {
             perror("write");
             exit(2);
        }

        offsetBlock++;
    }
}

void decryptFile(int inFD, int outFD, uint32_t* keyData, short endianness) {
    uint32_t datablock[2];
    int bytesReaded = 0;

    struct stat fileStat;
    if(fstat(inFD, &fileStat) < 0) {
        printf("Impossible de lire les informations du fichier");
        exit(5);
    }

    int offsetBlock = 1;
    int padding = 0;

    while(1) {
        memset(datablock, 0x0, sizeof(u_int64_t));

        bytesReaded = read(inFD, datablock, sizeof(uint64_t));

        if(bytesReaded == 0){
            break; //EOF
        }

        /* On check si les données ont un byte order spécifique */
        if(isBigEndian() == 1 && endianness == 0) {
            *datablock = htobe64((uint64_t)datablock);
        } else if (isBigEndian() == 0 && endianness == 1) {
            *datablock = be64toh((uint64_t)datablock);
        }

        decrypt(datablock, keyData);

        if(offsetBlock >= fileStat.st_size / 8) {
            padding = readPadding((uint64_t*)datablock);
        }

        if(write(outFD, datablock, sizeof(uint64_t) - padding) ==-1) {
            perror("write");
            exit(2);
        }    
    
        offsetBlock++;
    }
}

/*int main(int argc, char const *argv[]){

    int keyFD, inFD, outFD;
    uint32_t buff[BFSIZE];
    uint32_t buffBlocks[BFSIZE];

    uint32_t keyData[4];
    if(argc < 2) {
        printf("Usage : %s -e|-d <clé> <fichier_in> <fichier_out>");
        exit(1);
    }

    printf("Big Endiann : %d\n", isBigEndian());

    bzero(&buff, BUFSIZ);

    keyFD = open(argv[2], O_RDONLY);
    inFD = open(argv[3], O_RDWR);
    outFD = open(argv[4], O_CREAT|O_RDWR);

    for (int keyOff = 0 ; read(keyFD, &buff[keyOff], sizeof(uint32_t)) > 0; keyOff++) {
        keyData[keyOff] = buff[keyOff];
        printf("%llX\n", buff[keyOff]);
    }

    if(!strcmp(argv[1], "-d")) {
        decryptFile(inFD, outFD, keyData, isBigEndian());
        printf("dec");
    } else if(!strcmp(argv[1], "-e")) {
        encryptFile(inFD, outFD, keyData, isBigEndian());
        printf("enc");
    }

    close(inFD);
    close(outFD);


   /* uint32_t value[2] = {0x426f6e6a, 0x6f757200};
    uint32_t key[4]= {0x8922FFFF, 0x8922FFFF, 0x8922FFFF, 0x8922FFFF};
    printf("Mot : 0x%X\n", value[0]);
    encrypt(value, key);
    printf("Mot encrypt : 0x%X\n", value[0]);
    decrypt(value, key);
    printf("Mot decrypt : 0x%X\n", value[0]);
    return 0;
}*/
