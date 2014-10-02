#ifndef SCRAMBLER_H_
#define SCRAMBLER_H_
#include "csprng.h"

// Fill pool with random values
int RandPool_Fill(enum CSPRNG ng);
// pull the next value from pool	
ub4 RandPool_Next(enum CSPRNG ng);
// pull from pool by external index	
ub4 RandPool_Get(ub4 i);
// Reset the index into Pool	
void RandPool_Reset(void);
// Zeroise the Random pool
void RandPool_Clear(void);
// Record pairs for swapping
void InitRandPairs(enum CSPRNG ng, ub4 dep, ub4 len);
// Swap two ASCII chars
// Scramble by repeatedly swapping elements in string
char* Scrambled(char *t, ub4 depth);
// unScramble by repeatedly swapping elements in string
char* unScrambled(char *t, ub4 depth);
// set the mean random scramble-depth based on key-length
ub4 SetDepth(enum CSPRNG ng, ub4 len);

#endif
