/* countx.c
 * By Bob Jenkins, public domain, implementing Bob Blorp2's public domain test.
	
 * Given a subsequence of n 32-bit random numbers, compute the number
 * of bits set in each term, reduce that to low, medium or high number
 * of bits, and concatenate a bunch of those 3-item buckets.
 * Do this for len overlapping n-value sequences.  And report the chi-square
 * measure of the results compared to the ideal distribution.
 
 * Extended with multiple-round tabulation by C.C.Kayne, September 2014.
	Usage: 	>countx <# rounds> <min terms> {<max terms>} {<# trials>} 
	Example:>countx 32 2 12 3
	(Perform 3 trials, from 2**32 to 2**34 sequences, from 2 to 12 terms per sequence.)
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
// countx globals
static u4 loglen, terms;

#define LOGBUCKETS 2
#define BUCKETS (1<<LOGBUCKETS)
#define GRAY_CODE 1

// BB defines
//#define TEST
//#define VERBOSE
#define TABLE

//#define BB8
//#define BB16
//#define BB32
//#define BB64
#define BB128
//#define BB256
//#define BB512

#ifdef BB8
#define STSZ 8
#define NAME "BB8"
#endif
#ifdef BB16
#define STSZ 16
#define NAME "BB16"
#endif
#ifdef BB32
#define STSZ 32
#define NAME "BB32"
#endif
#ifdef BB64
#define STSZ 64
#define NAME "BB64"
#endif
#define STM1 STSZ-1
#ifdef BB128
#define STSZ 128
#define NAME "BB128"
#endif
#ifdef BB256
#define STSZ 256
#define NAME "BB256"
#endif
#ifdef BB512
#define STSZ 512
#define NAME "BB512"
#endif
#define STM1 STSZ-1

/* 2**32/phi, where phi is the golden ratio */
#define GOLDEN   0x9e3779b9
/* allow testing non-powers of 2 combos */
//#define MOD
//#define NOT
#define B
#define DORSW
#define RI 0


typedef struct ranctx { u4 state[STSZ]; u4 rsl[STSZ]; u4 b; u4 c; u4 d; } ranctx;

#define rot(x,k) ((x<<k)|(x>>(32-k)))

u4 rcnt=0; u8 fcount=0; u4 icount=0;

// BBXXX ROT switcher
u4 ri=0;
static struct Rsw { u4 iii; u4 jjj; u4 kkk; };
static struct Rsw rsw[4] = {
#ifdef BB128
#ifdef B
14,  9, 16, // avalanche: 17.31 bits (worst case)
17,  8, 14, // avalanche: 17.31 bits (worst case)
17, 19, 30, // avalanche: 17.19 bits (worst case)
15,  9, 24  // avalanche: 17.19 bits (worst case)
#else
12, 25,  4, // avalanche: 17.50 bits (worst case)
18, 24,  4, // avalanche: 17.44 bits (worst case)
15, 22,  6, // avalanche: 17.31 bits (worst case)
 6,  6, 26  // avalanche: 17.31 bits (worst case)
#endif
#endif
#ifdef BB256
#ifdef B
30, 13, 24, // avalanche: 17.00 bits (worst case)
 5, 19, 20, // avalanche: 16.94 bits (worst case)
21,  4, 10, // avalanche: 16.84 bits (worst case)
29, 15,  6  // avalanche: 16.72 bits (worst case)
#else
20, 14,  8, // avalanche: 17.00 bits (worst case)
20,  2, 22, // avalanche: 16.97 bits (worst case)
22, 23, 24, // avalanche: 16.78 bits (worst case)
 7,  8, 22  // avalanche: 16.72 bits (worst case)
#endif
#endif
#ifdef BB512
#ifdef B
17,  3,  4, // avalanche: 16.56 bits (worst case)
15, 18, 23, // avalanche: 16.52 bits (worst case)
 3, 23, 24, // avalanche: 16.52 bits (worst case)
 3, 15,  8  // avalanche: 16.53 bits (worst case)
#else
24,  4, 23, // avalanche: 16.80 bits (worst case)
18, 26,  2, // avalanche: 16.50 bits (worst case)
 3,  6, 22, // avalanche: 16.50 bits (worst case)
20, 17,  8  // avalanche: 16.66 bits (worst case)
#endif
#endif
};

// BEDBUG PRNG
u4 ranval( ranctx *x) {
    register u4 i,r;
	r = x->rsl[rcnt];
	++rcnt; 
	#ifdef VERBOSE
	++fcount;
	#endif
	if (rcnt==STSZ) {
		for (i=0; i<STSZ; i++) {
			u4 e = x->state[x->d & STM1] - rot(x->b, rsw[ri].iii);
			x->state[x->d & STM1] = x->b ^ rot(x->c, rsw[ri].jjj);
			x->b = x->c + rot(x->d, rsw[ri].kkk);
			x->c = x->d + e;
			#ifdef B
			x->d = e + x->state[x->b & STM1];
			#else
			x->d = e + x->state[i];
			#endif
			x->rsl[i] = x->d;
		}
		rcnt = 0;
		#ifdef DORSW
		ri = (x->c & 3);
		#endif
	}
	return r;
}

static u4 raninit(ranctx *x, u4 seed ) {
	register u4 i,r;
	icount++;
	x->b = x->c = x->d = GOLDEN;
	for (i=0; i<STSZ; i++) {
		x->state[i] = GOLDEN; x->rsl[i] = 0;
	}
	// warmup
	x->state[0]=seed;
	for (i=0; i<20; i++) r = ranval(x);
	return r;
}


/* count how many bits are set in a 32-bit integer, returns 0..32 */
static u4 count(u4 x)
{
  u4 c = x;

  if (GRAY_CODE) c = c^(c<<1);

   c = c - ((c >> 1) & 0x55555555);
   c = (c & 0x33333333) + ((c >> 2) & 0x33333333);
   c = (c + (c >> 4)) & 0x0F0F0F0F;
   c = c + (c >> 8);
   c = c + (c >> 16);
   return c & 0x0000003F;
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
  #ifdef TABLE
  printf("2**%2d	%2d	%10.2f	%10.2f	%8.4f",loglen,terms,(float)buckets,(float)var, 
	 (float)((var-buckets)/sqrt((float)buckets)));
  #else
  printf("expected variance: %11.4f   got: %11.4f   chi-square: %6.4f\n",
         (float)buckets, (float)var, 
	 (float)((var-buckets)/sqrt((float)buckets)));
  #endif
}


int main( int argc, char **argv)
{
  u8 len;
  u8 *data;
  u4 i,round,rounds,minterms,maxterms,trial,trials=1;
  ranctx r;
  time_t a,z;
  
  if (argc >= 3) {
	sscanf(argv[1], "%d", &loglen);
	#ifdef VERBOSE
	printf("sequence length: 2^^%d\n", loglen);
	#endif

	sscanf(argv[2], "%d", &terms);
	#ifdef VERBOSE
	printf("terms in subsequences: %d\n", terms);
	#endif
	} else {
		fprintf(stderr, "usage: \"countx 24 6\" means use 2^^24 sequences of length 6\n");
		return 1;
	}
	if (argc>=4) { maxterms=atoi(argv[3]); rounds=maxterms-terms+1; } else rounds=1;
	if (argc>=5) trials=atoi(argv[4]);
	
	minterms=terms;
	
	// trials loop
	for (trial=0; trial<trials; trial++) {
		printf("\n%s: ",argv[0]);
		printf("%s",NAME);
		#ifdef B
		printf("B");
		#endif
		#ifdef DORSW
		printf(" RSW(%d) ",4);
		for (i=0;i<4;i++) printf("{%2d,%2d,%2d}",rsw[i].iii,rsw[i].jjj,rsw[i].kkk);
		#else
		printf(" (RI=%d) {%2d,%2d,%2d}",RI,rsw[RI].iii,rsw[RI].jjj,rsw[RI].kkk);
		#endif
		puts("");

	
		#ifdef TABLE
		printf("Trial	Terms	Expected V	Observed V	Chi-square	 Time\n");
		printf("----------------------------------------------------------------------\n");
		#endif
		
		len = (((u8)1)<<loglen);
		terms=minterms;
		
		// sequences loop
		for (round=0; round<rounds; round++) {
			time(&a);

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
			#ifdef TABLE
			printf("	%4d s\n",(size_t)(z-a));
			#else
			printf("\nnumber of seconds      : %12d\n", (size_t)(z-a));
			#endif
			#ifdef VERBOSE
			printf("number of ranval calls : %12llu\n", fcount);
			printf("number of raninit calls: %12u\n", icount);
			#endif
		
			terms++;
		}
		loglen++;
	}

	#ifdef TEST
	#ifdef DORSW
	puts(""); for (i=0; i<=RSW; i++) printf("RI[%u] %u; ",i,ricnt[i]); puts("");
	#endif
	#endif
	return 0;
}

