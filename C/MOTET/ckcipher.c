// Caesar, Vernam & Vigenere ciphers, 
//  plus cryptographic string and hash functions.
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include "csprng.h"
#include "mystring.h"
#include "cktype.h"
#include "ckcipher.h"
	
char MOD = 95; char START = 32;


// Get a random 32-bit value in range [min..max]
ub4 rRand(enum CSPRNG ng, ub4 min, ub4 max) {
	ub4 r;
	do
		r=rRandom(ng) % max;
	while(r<min);
	return r;
}

// Get a random character in printable ASCII range
char rRandA(enum CSPRNG ng)
{	
	return rRandom(ng) % MOD + START;
}


// Get a random byte in [33..255]
/* < for C strings - must not contain '\0' > */
char rRandB(enum CSPRNG ng)
{	
	register ub4 r;
	do
		r=rRandom(ng) % 255;
	while (r<1);
	return r;
}


// Warm up the CSPRNG for <rounds>
void rStir(enum CSPRNG ng, ub4 rounds) {
	register ub4 i;
	for (i=0; i<rounds; i++) rRandom(ng);
}



// obtain a random stir-depth
ub4 rDepth(enum CSPRNG ng, char *str) {
	register ub4 i,l,r;
	register ub4 tot = 0;
	l = strlen(str);
	r = rRandom(ng) % 999;
	for (i=0; i<r; i++) 
		tot += (rRandom(ng) >> 21) * l;
	return tot / i;
}


// obtain a cryptographic ASCII hash on <str>, of length <len>
char* rHash(enum CSPRNG ng, char *s, int len) {
	register ub4 i,d;
	char h[MAXK];
	memset(h,'\0', len+1);
	rSeed(ng,s,1024);
	d = rDepth(ng,s);
	rStir(ng,d);
	for (i=0; i<len; i++) h[i] = rRandB(ng);
	return h;
}
	

// obtain a cryptographic ASCII string on <str>, of length <len>
char* rString(enum CSPRNG ng, char *s, int len) {
	register ub4 i,d;
	char str[MAXK];
	memset(str,'\0', len+1);
	rSeed(ng,s,1024);
	d = rDepth(ng,s);
	rStir(ng,d);
	for (i=0; i<len; i++) str[i] = rRandom(ng) % MOD + START;
	return str;
}


// obtain a cryptographic nonce/IV ASCII hash of NLEN or semi-random length
char* rNonce(enum CSPRNG ng, int israndlen) {
	register ub4 len;
	char non[MAXK]="";
	if (israndlen) len=rRand(ng,NLENMIN,NLENMAX);
		else len=NLEN;
	sprintf(non,"%d ",getpid());
	strcat(non,datetimestr());
	strcpy(non,rString(ng, non,len));
	return non;
}


// XOR cipher on random stream. Output: ASCII string
char* Vernam(enum CSPRNG ng, char *msg)
	{
		register ub4 i,l;
		l = strlen(msg);
		char v[MAXM];
		// zeroise v
		memset(v,'\0',l+1);
		// XOR message
		for (i=0; i<l; i++) 
			v[i] = rRandA(ng) ^ msg[i];
		return v;
	}

	
// Caesar-shift a character
char Caesar(enum ciphermode m, char ch, char shift, char modulo, char start)
	{
		register int n;
		if (m == cmDecipher) shift = -shift;
		n = (ch-start) + shift;
		n = n % modulo;
		if (n<0) n += modulo;
		return start+n;
	}

// Caesar shift a character, randomly selecting rectus or reversus mode 
char mCaesar(enum CSPRNG ng, enum ciphermode m, char ch, char shift, char modulo, char start)
	{
		if (rRandom(ng) % 100 > 49) {
			// rectus
			if (m == cmEncipher) 
				// shift
				return Caesar(cmEncipher, ch, shift, modulo, start);
			else
				// unshift
				return Caesar(cmDecipher, ch, shift, modulo, start);
		} else {
			// reversus
			if (m == cmEncipher) 
				// unshift
				return Caesar(cmDecipher, ch, shift, modulo, start);
			else
				// shift
				return Caesar(cmEncipher, ch, shift, modulo, start);
		}
	}	
	
	
	
// Caesar shift on a random character, randomly mixing modes 
char rmCaesar(enum CSPRNG ng, enum ciphermode m, char ch, char modulo, char start)
	{
		if (rRandom(ng) % 100 > 49) {
			// rectus
			if (m == cmEncipher) 
				// shift
				return Caesar(cmEncipher, ch, rRandom(ng) % modulo, modulo, start);
			else
				// unshift
				return Caesar(cmDecipher, ch, rRandom(ng) % modulo, modulo, start);
		} else {
			// reversus
			if (m == cmEncipher) 
				// unshift
				return Caesar(cmDecipher, ch, rRandom(ng) % modulo, modulo, start);
			else
				// shift
				return Caesar(cmEncipher, ch, rRandom(ng) % modulo, modulo, start);
		}
	}	
	
	
// Caesar-shift a string on a pseudo-random stream
char* rCaesarStr(enum CSPRNG ng, enum ciphermode m, char *msg, char modulo, char start)
	{
		register ub4 i,l;
		l = strlen(msg);
		char c[MAXM];
		// zeroise c
		memset(c,'\0',l+1);
		// Caesar-shift message
		for (i=0; i<l; i++) 
			c[i] = Caesar(m, msg[i], rRandA(ng), modulo, start);
		return c;
	}

// Caesar-shift a string, randomly mixing rectus and reversus
char* rmCaesarStr(enum CSPRNG ng, enum ciphermode m, char *msg, char modulo, char start)
	{
		register ub4 i,l;
		l = strlen(msg);
		char c[MAXM];
		// zeroise c
		memset(c,'\0',l+1);
		// Caesar-shift message
		for (i=0; i<l; i++) 
			c[i] = rmCaesar(ng, m, msg[i], modulo, start);
		return c;
	}

	
// Classical Vigenere: Caesar cipher on a repeated key, optionally mixing modes
char* Vig(enum CSPRNG ng, enum ciphermode m, char *pt, char *key, char modulo, char start, int mix) {
	register ub4 c,n,lp,lk;
	char kt[MAXM]=""; char ct[MAXM]="";
	lp=strlen(pt); lk=strlen(key);
	memset(kt,'\0',lp+1); memset(ct,'\0',lp+1);
	//construct the key-string from the key
	for (c=0;c<lp;c++) { n = c % lk; kt[c]=key[n]; }
	//cipher on the constructed key, optionally mixing modes
	if (mix) for (c=0;c<lp;c++) 
		ct[c] = mCaesar(ng, m, pt[c], kt[c], modulo, start);
	else	 for (c=0;c<lp;c++)
		ct[c] =  Caesar(m, pt[c], kt[c], modulo, start);
	return ct;
}
