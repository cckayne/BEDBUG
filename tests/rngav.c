/*
 * By Bob Jenkins, public domain
 *
 * With a 4-term state, results are w, x+stuff, y+stuff, z+stuff, w+stuff.
 * Make sure we've mixed the state well enough that 1-bit differences at 
 * w are pretty different by the time we report w+stuff.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <time.h>

typedef  unsigned char      u1;
typedef  unsigned long      u4;

// RNG defines
//#define MOTE
//#define MITE
#define BEDBUG
//#define FLEA

#ifdef MOTE
	#define NAME "MOTE"
#endif
#ifdef MITE
	#define NAME "MITE"
#endif
#ifdef BEDBUG
	#define NAME "BEDBUG"
#endif
#ifdef FLEA
	#define NAME "FLEA"
#endif


#define BUCKETS 128
#define LOGLEN  16
/* 18.25 for MITE32; 17.5 for MITE64; 17.0 for MITE128; 19.0 for MITE16; 20.4 for MITE8; 17.0 for BB128; 16.6 for BB256; 16.4 for BB512 */
#define CUTOFF 20.0 
#define STSZ 8
#define STM1 STSZ-1
/* 2**32/phi, where phi is the golden ratio */
#define GOLDEN   0x9e3779b9
#define FLEASEED 0xf1ea5eed
#define NOT
#define B

typedef struct ranctx { u4 state[STSZ]; u4 rsl[STSZ]; u4 a; u4 b; u4 c; u4 d; u4 e;} ranctx;

#define rot(x,k) ((x<<(k))|(x>>(32-(k))))

static u4 iii, jjj, kkk;

static u4 rcnt=0; static u4 fcount=0; static u4 icount=0;

#ifdef MOTE
static u4 ranval(ranctx *x) {
	register u4 i,r;
	r = x->rsl[rcnt];
	++rcnt; ++fcount;
	if (rcnt==STSZ) {
		for (i=0; i<STSZ; i++) {
			#ifdef NOT
			x->state[x->c & STM1] = ~x->e;
			#else
			x->state[x->c & STM1] = x->e;
			#endif
			x->b = x->c ^ ((x->e>>iii) | (x->d<<jjj));
			x->c = x->d - rot(x->b,kkk);
			x->d = x->state[i] + x->b;
			x->e = x->c + x->d;
			x->rsl[i] = x->c;
		}
		rcnt = 0;
	}
	return r;
}
static void raninit(ranctx *x, u4 seed) {
	register u4 i,r;
	icount++;
	x->b = x->c = x->d = x->e = FLEASEED;
	for (i=0; i<STSZ; i++) { x->state[i]=GOLDEN; x->rsl[i]=0; }
	x->state[0] = seed;
	// init with u4 seed
	for (i=0; i<20; i++) r = ranval(x);
}
#endif
#ifdef MITE
static u4 ranval(ranctx *x) {
	register u4 i,r;
	r = x->rsl[rcnt];
	++rcnt; ++fcount;
	if (rcnt==STSZ) {
		for (i=0; i<STSZ; i++) {
			x->state[x->c & STM1] = x->b + x->d;
			x->a = x->b - x->c;
			x->b = x->c ^ (rot(x->e,iii) | rot(x->a,jjj));
			x->c = x->d - rot(x->b,kkk);
			x->d = x->state[x->c & STM1] + x->b;
			x->e = x->c + x->d;
			x->rsl[i] = x->e;
		}
		rcnt = 0;
	}
	return r;
}
static void raninit(ranctx *x, u4 seed) {
	register u4 i,r;
	icount++;
	x->d = seed;
	x->a = x->b = x->c = x->e = FLEASEED;
	for (i=0; i<STSZ; i++) { x->state[i]=GOLDEN; x->rsl[i]=0; }
	// init with u4 seed
	for (i=0; i<20; i++) r = ranval(x);
}
#endif
#ifdef BEDBUG
u4 ranval( ranctx *x) {
    register u4 i,r;
		r = x->rsl[rcnt];
		++rcnt; ++fcount;
		if (rcnt==STSZ) {
			for (i=0; i<STSZ; i++) {
				x->e = x->state[x->d & STM1] - rot(x->b, iii);
				x->state[x->d & STM1] = x->b ^ rot(x->c, jjj);
				x->b = x->c + rot(x->d, kkk);
				x->c = x->d + x->e;
				#ifdef B
				x->d = x->e + x->state[x->b & STM1];
				#else
				x->d = x->e + x->state[i];
				#endif
				x->a = x->d;
				x->rsl[i] = x->d;
			}
			rcnt = 0;
		}
		return r;
}
static u4 raninit(ranctx *x, u4 seed ) {
	register u4 i,r;
	icount++;
	x->a = x->b = x->c = x->d = x->e = 0x9e3779b9;	// the golden ratio
	for (i=0; i<STSZ; i++) {
		x->state[i] = 0x9e3779b9; x->rsl[i] = 0;
	}
	// init with u4 seed
	x->state[0]=seed;
	for (i=0; i<20; i++) r = ranval(x);
	return r;
}
#endif
#ifdef FLEA
static u4 ranval(ranctx *x) {
	register u4 i,r;
	r = x->rsl[rcnt];
	++rcnt; ++fcount;
	if (rcnt==STSZ) {
	for (i=0; i<STSZ; ++i) {
        x->e = x->state[x->d & STM1];
        x->state[x->d & STM1] = x->b;
        x->b = (x->c<<iii) + (x->c>>jjj) + x->d;
        x->c = x->d ^ x->state[i];
        x->d = x->e + x->b;
		x->rsl[i]=x->c;
		}
	rcnt=0;
	}
	return r;
}
static u4 raninit(ranctx *x, u4 seed) {
	register u4 i,r;
	icount++;
	x->b = x->c = x->d = x->e = 0x9e3779b9;	// the golden ratio
	for (i=0; i<256; i++) {
		x->state[i] = 0x9e3779b9; x->rsl[i] = 0;
	}
	// init with u4 seed
	x->state[0]=seed;
	for (i=0; i<20; i++) r = ranval(x);
	return r;
}

#endif

/* count how many bits are set in a 32-bit integer, returns 0..32 */
static u4 count(u4 x)
{
  u4 c = x;

  c = (c & 0x55555555) + ((c>>1 ) & 0x55555555);
  c = (c & 0x33333333) + ((c>>2 ) & 0x33333333);
  c = (c & 0x0f0f0f0f) + ((c>>4 ) & 0x0f0f0f0f);
  c = (c & 0x00ff00ff) + ((c>>8 ) & 0x00ff00ff);
  c = (c & 0x0000ffff) + ((c>>16) & 0x0000ffff);
  return c;
}

/* initialize the data collection array */
static void datainit( u4 *data, u4 *data2)
{
  u4 i;
  for (i=0; i<BUCKETS; ++i) {
    data[i] = 0;   /* look for poor XOR mixing */
    data2[i] = 0;  /* look for poor additive mixing */
  }
}

/* gather statistics on len overlapping subsequences of length 5 each */
static void gather( ranctx *x, u4 *data, u4 *data2, u4 length)
{
  u4 i, j, h;
  u4 k;
  ranctx y;

  for (i=0; i<BUCKETS; ++i) {
    for (k=0; k<length; ++k) {
      y = *x;
      if (i<32)
	y.a ^= (1<<i);
      else if (i<64)
	y.b ^= (1<<(i-32));
      else if (i<96)
	y.c ^= (1<<(i-64));
      else
	y.d ^= (1<<(i-96));
      for (j=0; j<4; ++j) {
	h = ranval(x) ^ ranval(&y);         /* look for poor mixing */
      }
      data[i] += count(h);
      h ^= (h<<1);     /* graycode to look for poor additive mixing */
      data2[i] += count(h);
    }
  }
}


static int report( u4 *data, u4 *data2, u4 length, int print)
{
  u4 i;
  double worst = data[0];
  for (i=1; i<BUCKETS; ++i) {
    if (worst > data[i]) {
      worst = data[i];
    }
    if (worst > 32-data[i]) {
      worst = 32-data[i];
    }
    if (worst > data2[i]) {
      worst = data2[i];
    }
    if (worst > 32-data2[i]) {
      worst = 32-data2[i];
    }
  }
  worst /= length;
  if (worst > CUTOFF) {
    if (print) {
      printf("%2d, %2d, %2d, // avalanche: %2.2f bits (worst case)\n", 
	     iii, jjj, kkk, (float)worst);
    }
    return 1;
  } else {
    return 0;
  }
}

void driver()
{
  u4 i;
  u4 data[BUCKETS];
  u4 data2[BUCKETS];
  ranctx r;

  (void)raninit(&r, 0);
  datainit(data, data2);
  gather(&r, data, data2, (1<<6));
  for (i=6; i<LOGLEN; ++i) {
    gather(&r, data, data2, (1<<i));
    if (!report(data, data2, (1<<(i+1)), ((i+1)==LOGLEN))) {
      break;
    }
  }
}

int main( int argc, char **argv)
{
  u4 i, j, k;
  time_t a,z;

  printf("Testing %s w/ STATESIZE %d ...\n\n",NAME,STSZ);
  
  time(&a);

  for (i=0; i<30; ++i) {
    for (j=0; j<30; ++j) {
      for (k=0; k<30; ++k) {
	kkk = k+1;
	jjj = j+1;
    iii = i+1;
	driver();
      }
    }
  }

  time(&z);

  puts("");
  printf("number of seconds      : %12d\n", (size_t)(z-a));
  printf("number of ranval calls : %12u\n", fcount);
  printf("number of raninit calls: %12u\n", icount);
}
