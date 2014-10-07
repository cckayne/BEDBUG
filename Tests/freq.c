/*
 * Measure whether all values appear with equal frequency
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

typedef  unsigned char      u1;
typedef  unsigned long      u4;
typedef  unsigned long long u8;

#define BUCKETS (1<<8)
// BEDBUG defines
#define STATESIZE 128
#define STM1 STATESIZE-1
#define B
#define RSW

typedef struct ranctx { u4 state[STATESIZE]; u4 rsl[STATESIZE]; u4 a; u4 b; u4 c; u4 d;} ranctx;

#define rot(x,k) ((x<<k)|(x>>(32-k)))

u4 rcnt=0; u8 fcount=0; u4 icount=0;

// BBXXX ROT switcher
u4 ri=0;
static const struct Rsw { u4 iii; u4 jjj; u4 kkk; };
static const struct Rsw rsw[3];
static void initrsw(void) {
// BB128
if (STATESIZE==128) {
#ifdef B
rsw[0].iii = 14; rsw[0].jjj =  9; rsw[0].kkk = 16; // avalanche: 17.31 bits (worst case)
rsw[1].iii = 17; rsw[1].jjj =  8; rsw[1].kkk = 14; // avalanche: 17.31 bits (worst case)
rsw[2].iii = 17; rsw[2].jjj = 19; rsw[2].kkk = 30; // avalanche: 17.19 bits (worst case)
rsw[3].iii = 15; rsw[3].jjj =  9; rsw[3].kkk = 24; // avalanche: 17.19 bits (worst case)
#else
rsw[0].iii = 12; rsw[0].jjj = 25; rsw[0].kkk =  4; // avalanche: 17.50 bits (worst case)
rsw[1].iii = 18; rsw[1].jjj = 24; rsw[1].kkk =  4; // avalanche: 17.44 bits (worst case)
rsw[2].iii = 15; rsw[2].jjj = 22; rsw[2].kkk =  6; // avalanche: 17.31 bits (worst case)
rsw[3].iii =  6; rsw[3].jjj =  6; rsw[3].kkk = 26; // avalanche: 17.31 bits (worst case)
#endif
// BB256
} else if (STATESIZE==256) {
#ifdef B
rsw[0].iii = 30; rsw[0].jjj = 13; rsw[0].kkk = 24; // avalanche: 17.00 bits (worst case)
rsw[1].iii =  5; rsw[1].jjj = 19; rsw[1].kkk = 20; // avalanche: 16.94 bits (worst case)
rsw[2].iii = 21; rsw[2].jjj =  4; rsw[2].kkk = 10; // avalanche: 16.84 bits (worst case)
rsw[3].iii = 29; rsw[3].jjj = 15; rsw[3].kkk =  6; // avalanche: 16.72 bits (worst case)
#else
rsw[0].iii = 20; rsw[0].jjj = 14; rsw[0].kkk =  8; // avalanche: 17.00 bits (worst case)
rsw[1].iii = 20; rsw[1].jjj =  2; rsw[1].kkk = 22; // avalanche: 16.97 bits (worst case)
rsw[2].iii = 22; rsw[2].jjj = 23; rsw[2].kkk = 24; // avalanche: 16.78 bits (worst case)
rsw[3].iii =  7; rsw[3].jjj =  8; rsw[3].kkk = 22; // avalanche: 16.72 bits (worst case)
#endif
// BB512
} else {
#ifdef B
rsw[0].iii = 17; rsw[0].jjj =  3; rsw[0].kkk =  4; // avalanche: 16.56 bits (worst case)
rsw[1].iii = 15; rsw[1].jjj = 18; rsw[1].kkk = 23; // avalanche: 16.52 bits (worst case)
rsw[2].iii =  3; rsw[2].jjj = 23; rsw[2].kkk = 24; // avalanche: 16.52 bits (worst case)
rsw[3].iii =  3; rsw[3].jjj = 15; rsw[3].kkk =  8; // avalanche: 16.53 bits (worst case)
#else
rsw[0].iii = 24; rsw[0].jjj =  4; rsw[0].kkk = 23; // avalanche: 16.80 bits (worst case)
rsw[1].iii = 18; rsw[1].jjj = 26; rsw[1].kkk =  2; // avalanche: 16.50 bits (worst case)
rsw[2].iii =  3; rsw[2].jjj =  6; rsw[2].kkk = 22; // avalanche: 16.50 bits (worst case)
rsw[3].iii = 20; rsw[3].jjj = 17; rsw[3].kkk =  8; // avalanche: 16.66 bits (worst case)
#endif
}
}

// BEDBUG PRNG
u4 ranval( ranctx *x) {
    register u4 i,r;
		r = x->rsl[rcnt];
		++rcnt; ++fcount;
		if (rcnt==STATESIZE) {
			for (i=0; i<STATESIZE; i++) {
				u4 e = x->state[x->d & STM1] - rot(x->b, rsw[ri].iii);
				x->state[x->d & STM1] = x->b ^ rot(x->c, rsw[ri].jjj);
				x->b = x->c + rot(x->d, rsw[ri].kkk);
				x->c = x->d + e;
				#ifdef B
				x->d = e + x->state[x->b & STM1];
				#else
				x->d = e + x->state[i];
				#endif
				x->a = x->d;
				x->rsl[i] = x->d;
			}
			rcnt = 0;
			#ifdef RSW
			ri = (x->c & 3);
			#endif
		}
		return r;
}

static u4 raninit(ranctx *x, u4 seed ) {
	register u4 i,r;
	icount++;
	initrsw();
	x->a = x->b = x->c = x->d = 0x9e3779b9;	// the golden ratio
	for (i=0; i<STATESIZE; i++) {
		x->state[i] = 0x9e3779b9; x->rsl[i] = 0;
	}
	// init with u4 seed
	x->state[0]=seed;
	for (i=0; i<20; i++) r = ranval(x);
	return r;
}


/* initialize the data collection array */
static void datainit( u8 *data)
{
  u4 i;
  for (i=0; i<BUCKETS; ++i) data[i] = (u8)0;
}

/* gather statistics on len overlapping subsequences of length 5 each */
#define INCREMENT 0x10000
static void gather( ranctx *r, u8 *data, u8 len)
{
  u8 i;
  u4 j;
  for (i=0; i<len; i+=INCREMENT) {
    for (j=0; j<INCREMENT; j+=4) {
      ++data[ranval(r)&(BUCKETS-1)];
      ++data[ranval(r)&(BUCKETS-1)];
      ++data[ranval(r)&(BUCKETS-1)];
      ++data[ranval(r)&(BUCKETS-1)];
    }
  }
}

static void chi( u8 *data, u8 len)
{
  u4 i;
  double var = 0.0;         /* total variance */
  double temp;              /* used to calculate variance of a bucket */
  double expect = ((double)len)/BUCKETS;
  
  for (i=0; i<BUCKETS; ++i) {
    double temp = (double)data[i] - expect;
    var += temp*temp/expect;
  }

  /* calculate the total variance and chi-square measure */
  printf("expected variance: %11.4f   got: %11.4f   chi-square: %6.4f\n",
         (float)(BUCKETS-1), (float)var, 
	 (float)((var-(BUCKETS-1))/sqrt((float)(BUCKETS-1))));
}


int main( int argc, char **argv)
{
  u8 len;
  u8 data[BUCKETS];
  u4 loglen = 0;
  ranctx r;
  time_t a,z;
  
  if (STATESIZE==128) printf("Testing BB128"); else
  if (STATESIZE==256) printf("Testing BB256"); else printf("Testing BB512");
  #ifdef B
	printf("B");
  #endif
  #ifdef RSW
	printf(" RSW");
  #endif
  puts(" ...");

  time(&a);
  if (argc == 2) {
    sscanf(argv[1], "%d", &loglen);
    printf("log_2 sequence length: %d\n", loglen);
    len = (((u8)1)<<loglen);
  } else {
    fprintf(stderr, "usage: \"freq 24\" means run for 2^^24 values\n");
    return 1;
  }

  datainit(data);
  raninit(&r, 0);
  gather(&r, data, len);
  chi(data, len);

  time(&z);
  printf("\nnumber of seconds      : %12d\n", (size_t)(z-a));
  printf("number of ranval calls : %12llu\n", fcount);
  printf("number of raninit calls: %12u\n", icount);
}
