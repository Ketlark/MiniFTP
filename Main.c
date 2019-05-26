#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "TCP.h"
#include "DH.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    uint64_t prime = generatePrime();
	printf("** Global prime - %d\n", prime);
	int generator = generateBase(prime);
	printf("** Global primitive root - %d\n\n", generator);

    uint64_t a = rand() % (generator - 1) + 1;
	uint64_t A = expm(generator, a, prime);
    printf("** Client Key - %d\n\n", A);

    uint64_t b = rand() % (generator - 1) + 1;
	uint64_t B = expm(generator, b, prime);
    printf("** Server Key - %d\n\n", B);

    uint64_t K1 = (uint64_t)expm(B, a, prime);
    uint64_t K2 = (uint64_t)expm(A, b, prime);

    uint64_t key;
    (&key)[0] = K1;
    (&key)[1] = K2;


    printf("** Shared Key Client - %llx\n\n", K1);
    printf("** Shared Key Server - %llx\n\n", K2);

    printf("0x%lx%lx", (&key)[0], (&key)[1]);
    /*if(argc > 1) {
        char* adress = argv[1];
        createClient(adress);
    } else {
        createServer();
    }*/
    return 0;
}
