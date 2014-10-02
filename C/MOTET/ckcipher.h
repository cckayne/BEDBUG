// ckcipher.h  Copyright C.C.Kayne 2014, GNU GPL V.3, cckayne@gmail.com  
#ifndef CKCIPHER_H_ 
#define CKCIPHER_H_
#include "csprng.h"

// Get a random 32-bit value in range [min..max]
ub4 rRand(enum CSPRNG ng, ub4 min, ub4 max);
// Get a random character in printable ASCII range
char rRandA(enum CSPRNG ng);
// Get a random byte in [33..255]
char rRandB(enum CSPRNG ng);
// Warm up the CSPRNG for <rounds>
void rStir(enum CSPRNG ng, ub4 rounds);
// obtain a random stir-depth
ub4 rDepth(enum CSPRNG ng, char *str);
// obtain a cryptographic ASCII hash on <str>, of length <len>
char* rHash(enum CSPRNG ng, char *s, int len);
// obtain a cryptographic ASCII string on <str>, of length <len>
char* rString(enum CSPRNG ng, char *s, int len);
// obtain a cryptographic nonce/IV ASCII hash of NLEN or semi-random length
char* rNonce(enum CSPRNG ng, int israndlen); 
// XOR cipher on random stream. Output: ASCII string
char* Vernam(enum CSPRNG ng, char *msg);
// Caesar-shift a character
char Caesar(enum ciphermode m, char ch, char shift, char modulo, char start);
// Caesar shift a character, randomly selecting rectus or reversus mode 
char mCaesar(enum CSPRNG ng, enum ciphermode m, char ch, char shift, char modulo, char start);
// Caesar shift on a random character, randomly mixing modes 
char rmCaesar(enum CSPRNG ng, enum ciphermode m, char ch, char modulo, char start);
// Caesar-shift a string on a pseudo-random stream
char* rCaesarStr(enum CSPRNG ng, enum ciphermode m, char *msg, char modulo, char start);
// Caesar-shift a string, randomly mixing rectus and reversus
char* rmCaesarStr(enum CSPRNG ng, enum ciphermode m, char *msg, char modulo, char start);
// Classical Vigenere: Caesar cipher on a repeated key, optionally mixing modes
char* Vig(enum CSPRNG ng, enum ciphermode m, char *pt, char *key, char modulo, char start, int mix);

#endif
