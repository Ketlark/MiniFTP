#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "DH.h"

#define MAXSIZE 100000
#define ITERATION 100

/* Function to compute (a ^ b) mod p */
uint64_t expm(uint128_t a, uint128_t b, uint128_t p) {
	uint128_t x = 1, y = a;
	while (b > 0) {
		if (b % 2 == 1)
			x = (x * y) % p;
		y = (y * y) % p;
		b /= 2;
	}
	return (uint64_t)(x % p);
}

/* Function to check primality of random generated numbers using Miller-Rabin Test */
int isPrime(int value, int iteration) {
	if (value < 2)
		return 0;
	int q = value - 1, k = 0;
	while (!(q % 2)) {
		q /= 2;
		k++;
	}
	for (int i = 0; i < iteration; i++) {
		int a = rand() % (value - 1) + 1;
		int current = q;
		int flag = 1;
		int mod_result = expm(a, current, value);
		for (int i = 1; i <= k; i++) {
			if (mod_result == 1 || mod_result == value - 1) {
				flag = 0;
				break;
			}
			mod_result = (int)((long long)mod_result * mod_result % value);
		}
		if (flag)
			return 0;
	}
	return 1;
}

/* Generate a prime number that is going to be shared 
 * globally between client and server
 */
uint64_t generatePrime() {
	srand(time(NULL));
	while(1) {
		uint64_t current_value = rand() % INT_MAX;
		if (!(current_value % 2))
			current_value++;
		if (isPrime(current_value, ITERATION) == 1)
			return current_value;
	}
}

/* Generate the primitive root by checking for random numbers */
int generateBase(int p) {
	/* Construct sieve of primes */
	int sieve[MAXSIZE];
	memset(sieve, 0, sizeof(sieve));
	sieve[0] = sieve[1] = 1;
	for (int i = 4; i < MAXSIZE; i += 2)
		sieve[i] = 1;
	for (int i = 3; i < MAXSIZE; i += 2) {
		if (!sieve[i]) {
			for (int j = 2 * i; j < MAXSIZE; j += i)
				sieve[j] = 1;
		}
	}
	while (1) {
		int a = rand() % (p - 2) + 2;
		int phi = p - 1, flag = 1, root = sqrt(phi);
		for (int i = 2; i <= root; i++) {
			if (!sieve[i] && !(phi % i)) {
				int mod_result = expm(a, phi / i, p);
				if (mod_result == 1) {
					flag = 0;
					break;
				}
				if (isPrime(phi / i, ITERATION) && !(phi % (phi / i))) {
					int mod_result = expm(a, phi / (phi / i), p);
					if (mod_result == 1) {
						flag = 0;
						break;
					}
				}
			}
		}
		if (flag) 
			return a;
	}
}

void generateKeyFirstStep(int server, uint64_t* privateNumber, uint64_t* publicNumber, uint64_t* primeNumber, uint64_t* primitiveRoot) {
    if(!server) {
		*primeNumber = generatePrime();
    	*primitiveRoot = generateBase(*primeNumber);

	}

    *privateNumber = rand() % (*primitiveRoot - 1) + 1;
	*publicNumber = expm(*primitiveRoot, *privateNumber, *primeNumber);
}

void generateKeySecondStep(uint64_t* key, uint64_t* privateNumber, uint64_t* publicNumberOther, uint64_t* primeNumber) {
	*key = expm(*publicNumberOther, *privateNumber, *primeNumber);
}