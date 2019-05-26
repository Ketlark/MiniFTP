#include <stdlib.h>
#include <stdio.h>
#include "TCP.h"

int main(int argc, char* argv[]) {
    if(argc > 1) {
        char* adress = argv[1];
        createClient(adress);
    } else {
        createServer();
    }
    return 0;
}
