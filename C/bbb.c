// bbb: Check output from the BEDBUG512 CSPRNG 
#include "bb512.h"
#include <stdio.h>
#include <stddef.h>
// mixing rounds: 7 recommended minimum
#define ROUNDS 7

int main(int argc, char *argv[]) {
	char seed[2048]="Bacon";
	// pull a seed from the command line
	if (argc>=2) strcpy(seed,argv[1]);
	// seed BEDBUG and mix
	bb512_SeedW(seed,512*ROUNDS);
	int i;
	// show some MOD 26 stream output
	for (i=0; i<70; i++) printf("%c",bb512_Random() % 26 + 'A');
	printf("\n");
	return 0;
}
