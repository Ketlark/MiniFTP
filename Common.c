#include "Common.h"
#include <stdint.h>
#include <arpa/inet.h>

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
