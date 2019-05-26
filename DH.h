#include <stdint.h>

#ifndef DH_H_   /* Include guard */
#define DH_H_

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

uint64_t generatePrime();
int generateBase(int p);
uint64_t expm(uint128_t m, uint128_t e, uint128_t n);

#endif