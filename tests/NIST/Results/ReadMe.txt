Results of the NIST industry-standard suite of statistical tests for randomness on CSPRNGs ISAAC, BB128, BB256 and BB512, plus Marsaglia's PRNG KISS. ISAAC is used here as the "gold standard" of cryptographically watertight comparison and KISS as another sanity check. CONG (a linear congruential generator) is included as an example of a wanting PRNG which fails a good many of the tests in the NIST battery.

Various keys were used to seed the PRNGs and the test results for each placed in separate folders. Changing the seed changed the p-values significantly in every case.

1)64MB - seed = "something very silly"
2)64MB - seed = "FSBiaccenRI"
3)64MB - seed = "BOte-swaine."
4)64MB - seed = "Heere Master: What cheere?"

The parameters for all tests were as follows:

	[1] Block Frequency Test - block length(M):         128
	[2] NonOverlapping Template Test - block length(m): 9
	[3] Overlapping Template Test - block length(m):    9
	[4] Approximate Entropy Test - block length(m):     10
	[5] Serial Test - block length(m):                  16
	[6] Linear Complexity Test - block length(M):       500

100 binary streams of length 671088 bytes (total 67108864 bytes) from each RNG tested were used as input to <assess>. The package used was sts-2.1.2, compiled from source under Linux.

The program used to generate the bitstream files was die(.exe), which can be found in the BEDBUG distribution under the Pascal directory.

C.C.Kayne
cckayne@gmail.com
