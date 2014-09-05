/*
 * By Bob Jenkins, public domain implementing Bob Blorp2's public domain test.
 *
 * Given a subsequence of n 32-bit random numbers, compute the number
 * of bits set in each term, reduce that to low, medium or high number
 * of bits, and concatenate a bunch of those 3-item buckets.
 * Do this for len overlapping n-value sequences.  And report the chi-square
 * measure of the results compared to the ideal distribution.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <time.h>

typedef  unsigned char      u1;
typedef  unsigned long      u4;
typedef  unsigned long long u8;

#define LOGBUCKETS 2 // was 2
#define BUCKETS (1<<LOGBUCKETS)
#define TERMS 6
#define GRAY_CODE 1
// BEDBUG defines
//#define VERBOSE
#define STATESIZE 256
#define STM1 STATESIZE-1
#define B
#define RSW

typedef struct ranctx { u4 state[STATESIZE]; u4 rsl[STATESIZE]; u4 a; u4 b; u4 c; u4 d;} ranctx;

#define rot(x,k) ((x<<k)|(x>>(32-k)))

u4 rcnt=0; u8 fcount=0; u4 icount=0;

// BBXXX ROT switcher
u4 ri=0;
static struct Rsw { u4 iii; u4 jjj; u4 kkk; };
static struct Rsw rsw[3];
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


/* count how many bits are set in a 32-bit integer, returns 0..32 */
static u4 count(u4 x)
{
  u4 c = x;

  if (GRAY_CODE) c = c^(c<<1);

  c = (c & 0x55555555) + ((c>>1 ) & 0x55555555);
  c = (c & 0x33333333) + ((c>>2 ) & 0x33333333);
  c = (c & 0x0f0f0f0f) + ((c>>4 ) & 0x0f0f0f0f);
  c = (c & 0x00ff00ff) + ((c>>8 ) & 0x00ff00ff);
  c = (c & 0x0000ffff) + ((c>>16) & 0x0000ffff);
  return c;
}

/* somehow covert 0..32 into 0..BUCKETS-1 */
static u4 ftab[] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 1,
  1, 
  1, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2
};

/* initialize the data collection array */
static void datainit2( u8 *data, u4 index, u4 depth, u4 terms)
{
  u4 i;
  index *= 3;
  if (depth == terms-1) {
    for (i=0; i<3; ++i)
      data[index+i] = 0;
  } else {
    for (i=0; i<3; ++i)
      datainit2(data, index+i, depth+1, terms);
  }
}

static void datainit( u8 *data, u4 terms)
{
  u4 i;
  for (i=0; i<(1<<(LOGBUCKETS*terms)); ++i)
    data[i] = 0;
}

/* gather statistics on len overlapping subsequences of "terms" values each */
static void gather( ranctx *r, u8 *data, u8 len, u4 terms)
{
  u8 i;
  u4 val = 0;
  u4 mask = (1<<(LOGBUCKETS*terms))-1;
  for (i=0; i<BUCKETS; ++i)
    val = ((val<<LOGBUCKETS)&mask) + ftab[count(ranval(r))];
  for (i=0; i<len; ++i) {
    val = ((val<<LOGBUCKETS)&mask) + ftab[count(ranval(r))];
    ++data[val];
  }
}

/* figure out the probability of 0..BUCKETS-1=ftab[count(u4)] */
static void probinit( double *pc, u4 maxbits)
{
  u8 i,j,k;
  for (i=0; i<=maxbits; ++i) {
    pc[i] = 0.0;
  }
  for (i=0; i<=maxbits; ++i) {
    k = 1;
    for (j=1; j<=i; ++j) {
      k = (k * (maxbits+1-j)) / j;
    }
    pc[ftab[i]] += ldexp((double)k,-32);
  }
}

#define MAXBITS 32
static void chi( u8 *data, u8 len, u4 terms)
{
  u4 i,j,k;                 /* counters */
  double pc[MAXBITS+1];     /* pc[i] is probability of a bitcount of i */
  double expect = 0.0;      /* expected fullness of current bucket */
  double expectother = 0.0; /* expected fullness of "other" bucket */
  double var = 0.0;         /* total variance */
  double temp;              /* used to calculate variance of a bucket */
  u8 buckets = 0;           /* number of buckets used */
  u8 countother = 0;
  
  probinit(pc, MAXBITS);

  /* handle the nonnegligible buckets */
  for (i=0; i < (1<<(LOGBUCKETS*terms)); ++i) {
    
    /* determine the expected frequency of this bucket */
    expect = (double)len;
    k = i;
    for (j=0; j<terms; ++j) {
      expect *= pc[k&(BUCKETS-1)];
      k >>= LOGBUCKETS;
    }

    /* calculate the variance for this bucket */
    if (expect < 5.0) {
      expectother += expect;
      countother += data[i];
    } else {
      ++buckets;
      temp = (double)data[i] - expect;
      temp = temp*temp/expect;
      if (temp > 20.0) {
	k = i;
	for (j=0; j<terms; ++j) {
	  #ifdef VERBOSE
	  printf("%2d ", k&(BUCKETS-1));
	  #endif
	  k >>= LOGBUCKETS;
	}
	#ifdef VERBOSE
	printf("%14.4f %14.4f %14.4f\n",
	       (float)temp,(float)expect,(float)data[i]);
	#endif
      }
      var += temp;
    }
  }

  /* lump all the others into one bucket */
  if (expectother > 5.0) {
    ++buckets;
    temp = (double)countother - expectother;
    temp = temp*temp/expectother;
    if (temp > 20.0) {
      #ifdef VERBOSE
	  printf("other %14.4f %14.4f %14.4f\n",
	     (float)temp,(float)expectother,(float)countother);
	  #endif	
    }
    var += temp;
  }
  --buckets;

  /* calculate the total variance and chi-square measure */
  printf("expected variance: %11.4f   got: %11.4f   chi-square: %6.4f\n",
         (float)buckets, (float)var, 
	 (float)((var-buckets)/sqrt((float)buckets)));
}

int main( int argc, char **argv)
{
  u8 len;
  u8 *data;
  u4 i, loglen, terms;
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
  if (argc == 3) {
    sscanf(argv[1], "%d", &loglen);
    printf("sequence length: 2^^%d\n", loglen);
    len = (((u8)1)<<loglen);

    sscanf(argv[2], "%d", &terms);
    printf("terms in subsequences: %d\n", terms);
  } else {
    fprintf(stderr, "usage: \"countx 24 6\" means use 2^^24 sequences of length 6\n");
    return 1;
  }

  data = (u8 *)malloc(sizeof(u8)*(1<<(LOGBUCKETS*terms)));
  if (!data) {
    fprintf(stderr, "could not malloc data\n");
    return 1;
  }

  for (i=0; i<=MAXBITS; ++i) {
    if (ftab[i] > BUCKETS) {
      fprintf(stderr, "ftab[%d]=%d needs a bigger LOGBUCKETS\n", i, ftab[i]);
      return 1;
    }
  }

  datainit(data, terms);
  raninit(&r, 0);
  gather(&r, data, len, terms);
  chi(data, len, terms);

  free(data);

  time(&z);
  printf("\nnumber of seconds      : %12d\n", (size_t)(z-a));
  printf("number of ranval calls : %12llu\n", fcount);
  printf("number of raninit calls: %12u\n", icount);
}

