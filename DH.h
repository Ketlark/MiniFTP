#include <stdint.h>

#ifndef DH_H_   /* Include guard */
#define DH_H_

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

typedef struct DHKeyInfos {
    uint64_t primeNumber;
    uint64_t primitiveRoot;
    uint64_t publicNumber;
} DHKeyInfos;

uint64_t generatePrime();
int generateBase(int p);
uint64_t expm(uint128_t m, uint128_t e, uint128_t n);
void generateKeyFirstStep(int server, uint64_t* privateNumber, uint64_t* publicNumber, uint64_t* primeNumber, uint64_t* primitiveRoot);
void generateKeySecondStep(uint64_t* key, uint64_t* privateNumber, uint64_t* publicNumberOther, uint64_t* primeNumber);

#endif