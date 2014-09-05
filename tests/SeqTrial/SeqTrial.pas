{$mode delphi}
{ $define TEST}
{ $define distrib}
{ $define SAM}
{ $define caesar}
{ $define VERBOSE}
PROGRAM SeqTrial;

USES rng, MyStrUtils, SysUtils, Math;

TYPE PArray = ARRAY Of EXTENDED;
	 CArray = ARRAY Of CARDINAL;
	 SArray = ARRAY Of STRING;
	 {$ifdef caesar}
	 TMCipher = (mEncipher, mDecipher, mNone);
	 {$endif}
// Default values
CONST 	VERSION = '1.0.0';
		COPYRT  = 'SeqTrial is Copyright (c) C.C.Kayne, 2014 - cckayne@gmail.com';
		PDOM	= 'SeqTrial is released into the Public Domain.';
		MAXCARD = $FFFFFFFF;
		MAXQ    = $FFFFFFFFFFFFFFFF;
		EX = 1000;
VAR TRIALS:	QWord = 100000000;
	MAXSEQ: CARDINAL = 100;
	{$ifdef distrib}
	MINTHROWS:	QWord = 1000000; // was 1000000
	{$else}
	MINTHROWS:	QWord = 100; // was 1000000
	{$endif}
	MODM1:		Cardinal = 25;
	MODULO:		Cardinal = 26;
	TM:			Cardinal = 25; // temp mod for set-checks
	START:		Cardinal = ord('A');
	seed: STRING = 'Monte Carlo Mod';
	RNG: TRNG = B512;
	
	i,j:   	CARDINAL;
	q  :	QWord = 0;
	r    : 	CARDINAL = MAXCARD-1;
	rlast: 	CARDINAL = MAXCARD-2;
	rpult: 	CARDINAL = MAXCARD-3;
	
	// sequences context
	TYPE TAllS = RECORD
		mean: EXTENDED;
		sigm: EXTENDED;
		vari: EXTENDED;
		chis: EXTENDED;
	END;
	VAR AllS: TAllS;
	
	TYPE TSeq = RECORD
		value: STRING[2];
		//raw experimental data
		smin:  Cardinal;
		smax:  Cardinal;
		snow:  Cardinal;		
		stot:  Cardinal;
		shi :  Cardinal;
		//total of observed sequences
		snum:  CArray;
		//probabilities of observed sequences
		sprob: PArray;
		// sorted probabilities
		sprbs: PArray;
		sptot: Extended;
		smean: Extended;
		smedi: Extended;
		ssigm: Extended;
		svari: Extended;
		schis: Extended;
	END;
		SeqArray = ARRAY OF TSeq;
	VAR Seq: SeqArray;
	

// sort an array (descending) using the selection-sort algorithm
PROCEDURE SelectionSort(mins,maxs: CARDINAL; VAR sArr: PArray);
    VAR current, j : Cardinal;
        large : Cardinal;
	PROCEDURE Swap(x,y: Cardinal);
		VAR temp: Extended;
		BEGIN
			temp := sArr[x];
			sArr[x] := sArr[y];
			sArr[y] := temp;
		END;
    BEGIN
		FOR current := mins TO maxs DO BEGIN
			large := maxs;
			FOR j := maxs DOWNTO current DO
				IF (sArr[j] > sArr[large]) THEN large := j;
			Swap(large,current);
		END;
    END;


// obtain the length of a sorted probability array (cutoff 0.0)
FUNCTION SPLength(mins: Cardinal; probs: PArray): Cardinal;
	VAR l: Cardinal;
	BEGIN
		l := mins;
		WHILE (probs[l]>0.0000000000) DO inc(l);
		SPLength := l;
	END;
	

// Median from a set of probabilities (returns array index & sorted probabilities)
FUNCTION MedianP(mins,maxs: Cardinal; VAR prob, sprob: PArray): CARDINAL;
	VAR i: Cardinal;
	BEGIN
		SetLength(sprob, Length(prob));
		FOR i := mins TO maxs DO sprob[i] := prob[i];
		SelectionSort(mins,maxs,sprob);
		MedianP := (maxs-mins) div 2;
	END;
	
// Maximum from a set of probabilities (returns array index)
FUNCTION MaxP(mins,maxs: Cardinal; prob: PArray): CARDINAL;
	VAR i: CARDINAL;
		m: EXTENDED;
	BEGIN
		m := 0.0;
		FOR i := mins TO maxs DO
			IF prob[i]>m THEN BEGIN m:=prob[i]; MaxP := i; END;
	END;
	
// Minimum from a set of probabilities (returns array index)
FUNCTION MinP(mins,maxs: Cardinal; prob: PArray): CARDINAL;
	VAR i: CARDINAL;
		m: EXTENDED;
	BEGIN
		m:= 1.0;
		FOR i := mins TO maxs DO
			IF prob[i]<m THEN BEGIN m:=prob[i]; MinP := i; END;
	END;

// Mean of a set of probabilities
FUNCTION Mean(mins,maxs: CARDINAL; prob: PArray): EXTENDED;
	VAR i: Cardinal;
		t: Extended;
	BEGIN
		t    := 0.0;
		FOR i:=mins TO maxs DO
			t += prob[i];
		Mean := t / i;
	END;

// Standard deviation of a set of probabilities
FUNCTION Sigma(minx,maxs: CARDINAL; prob: PArray): EXTENDED;
	VAR i: Cardinal;
		m: Extended;
		d: PArray;
	BEGIN
		SetLength(d,Length(prob));
		m := Mean(minx,maxs,prob);
		FOR i := minx TO maxs DO
			d[i] := (prob[i]-m)**2;
		// variance
		m := Mean(minx,maxs,d);
		Sigma := sqrt(m);
	END;


// Normalize a value in range [A..B] into range [C..D]
FUNCTION Normalize(value,A,B,C,D: EXTENDED): EXTENDED;
	BEGIN
		Normalize := C + ((value-A)*(D-C)) / (B-A);
	END;
	
	
// chi-square of a set of probabilities
FUNCTION ChiSquare(minx,maxs: CARDINAL; prob: PArray; norm: BOOLEAN): EXTENDED; OVERLOAD;
	VAR i,dof: Cardinal;
		m,chi: Extended;
	BEGIN
		chi := 0.0;
		// m-1 degrees of freedom
		dof := maxs-minx;
		// expected mean
		m := 1/maxs;
		FOR i := minx TO maxs DO
			chi += ((prob[i]-m)**2)/m;
		ChiSquare:=chi;
		IF norm then ChiSquare := (chi-dof)/(3*sqrt(dof));
	END;


// Normalized chi-square given expected and observed variance
FUNCTION Chi2(varexp,varobs: EXTENDED): EXTENDED;
	VAR dof: Cardinal;
	BEGIN
		// m-1 degrees of freedom
		dof := MODM1-1;
		Chi2 := ((varexp-varobs)-dof)/(3*sqrt(dof));
	END;


// Naive variance of a set of probabilities
FUNCTION Variance(minx,maxs: CARDINAL; prob: PArray): EXTENDED;
	VAR i: Cardinal;
		m: Extended;
		d: PArray;
	BEGIN
		SetLength(d,Length(prob));
		m := Mean(minx,maxs,prob);
		FOR i := minx TO maxs DO
			d[i] := (prob[i]-m)**2;
		// variance
		Variance := Mean(minx,maxs,d);
    END;


{$ifdef SAM}
// Obtain a value in [0..val-1] from a pseudo-random bitstream
//  by sampling b-bit segments as n until n is in range. 
// < Like other alternatives to MOD, SAM exhibits a very slightly 
//   inferior distribution over the range of possible values,
//   and is of course far slower in execution than MOD alone. 
//   Given a high quality RNG, SAM is otherwise unbiased. > 
function Sam(ng: TRNG; val: Cardinal): Cardinal;
	var r,m,n,b: Cardinal;
		i      : Longint;
		f      : Boolean;
	begin
		// calculate # of bits needed for value
		b:=0; n:=val;
		while (n > 0) do begin 
			inc(b);
			n := n shr 1;
		end;
		repeat
			r := rRandom(ng);
			i := -b;
			repeat
				i+=b;
				m := (1 shl b - 1) shl i;
				n := ((r and m) shr i);
				f := n in [0..val-1];
			until (f or (i>=(32-b)));
		until(f);
		Sam := n;
	end;
{$endif}


{$ifdef caesar}	
{ Caesar-shift a byte <shift> places }
FUNCTION Caesar(m: TMCipher; ch, shift, modulo: Longint): Longint;
	VAR n: Longint;
	BEGIN
		IF m = mDecipher THEN shift := -shift;
		n := (ch + shift) MOD modulo;
		IF n<0 THEN n += modulo;
		Caesar := n;
	END;
{$endif}
	
	
PROCEDURE Usage;
	BEGIN
		Writeln;
		Writeln('Usage:   >seqtrial <modulo> <# of trials> <seed> <RNG>');
		Writeln('Example: >seqtrial 26 32 "my seed" 2');
		Writeln('Permitted range for # trials: ',MINTHROWS,'-2**64.');
		Writeln('(If # given is in [8..64], 2**# is assumed.)');
		Writeln('Available RNGs: [0..14] 2=ISAAC 3=BB128 4=BB256 5=BB512.');
		Writeln('All command line parameters are optional.');
		Writeln('Default: mod 26 BB512 with 100000000 trials.');
		Writeln;
	END;

PROCEDURE Banner;
	BEGIN
		Writeln('SeqTrial: Same-value sequences emitted by a PRNG under the ',{$ifdef SAM}'SAM'{$else}'MOD'{$endif},' operation.');
		{$ifdef distrib}
		Writeln(COPYRT);
		Writeln(PDOM);
		Writeln('SeqTrial version ',VERSION);
		{$endif}
	END;
	
BEGIN
	TRY
		// Obtain modulo, trials, seed and RNG from the command line
		IF ParamCount >= 1 THEN BEGIN
			IF ParamStr(1)[1] IN ['h','H','i','u','U','-'] THEN BEGIN Banner; Usage; HALT; END;
			IF ParamStr(1) <> '' THEN MODULO := StrToInt(ParamStr(1));
			IF ParamStr(2) <> '' THEN TRIALS := StrToInt64(ParamStr(2));
			IF ParamStr(3) <> '' THEN seed := ParamStr(3);
			IF ParamStr(4) <> '' THEN RNG := TRNG(StrToInt(ParamStr(4)));
		END;
		// if number given is in this range, assume 2**number
		IF TRIALS IN [8..64] THEN TRIALS:=2**TRIALS-1;
		// Sanity checks
		IF TRIALS<MINTHROWS THEN TRIALS:=MINTHROWS;
		TM := MODULO-1; MODM1 := TM;
		IF NOT TM IN [1,3,7,15,25,94,127,255] THEN MODULO:=26;
		// Set default start-char based on chosen modulo
		IF MODULO=26 THEN START := ord('A') ELSE
		IF MODULO=95 THEN START := ord(' ') 
		ELSE START := 0;
		// Set up dynamic arrays
		SetLength(Seq,MODULO);
		MAXSEQ := MODM1 * MODM1 + 100;
		FOR i := 0 to MODM1 DO
			WITH Seq[i] DO BEGIN
				SetLength(snum ,MAXSEQ);
				SetLength(sprob,MAXSEQ);
			END;
		
		// Seed the RNGs
		rSeedAll(seed);
		// warm them up
		rStirAll(65536);
		
		Banner;
		{$ifdef VERBOSE}
		Usage;
		{$else}
		Writeln;
		{$endif}
		Writeln('Experiment: ',TRIALS,' ',RNGs[RNG],' sequence trials using ',{$ifdef SAM}'SAM '{$else}'MOD '{$endif},IntToStr(MODULO));
		Writeln('Experiment starts');
    
		// initialize sequences arrays
		FOR i:=0 TO MODM1 DO
			WITH Seq[i] DO BEGIN
				value := '';
				snow:=1; smin:=MAXCARD; smax:=0; stot:=0;
				smean:=0.0; ssigm:=0.0; svari:=0.0; schis:=0.0; sptot:=0.0;
				FOR j := 1 TO MAXSEQ DO
					snum[j] := 0;
			END;
		WITH AllS DO BEGIN
			mean:=0.0; sigm:=0.0; vari:=0.0; chis:=0.0;
		END;
				
		// initiate Monte Carlo experiment
		REPEAT
			inc(q);
			rpult:=rlast;
			rlast:=r;
			// Cast the die
			{$ifdef caesar}
			r:=Caesar(mDecipher, rRandom(RNG) mod MODULO, rRandom(RNG) mod MODULO, modulo);
			{$else}
			{$ifdef SAM}
			r:=SAM(RNG,MODULO);
			{$else}
			r:=rRandom(RNG) mod MODULO;
			{$endif}
			{$endif}
			// we're in a sequence
			IF (r=rlast) THEN
				// increment sequence count for r-value
				inc(Seq[r].snow)
			// now we're not in a sequence
			ELSE
				// rlast is our sequence value
				IF rpult=rlast THEN
					WITH Seq[rlast] DO BEGIN
						IF snow>smax THEN
							FOR i:=1 TO snow DO Write(chr(rlast+ord('A')));
						IF snow<smin THEN smin:=snow;
						IF snow>smax THEN smax:=snow;
						// update totals for each sequence
						inc(snum[snow]); 
						inc(stot);
						// reset sequence counter for all values
						FOR j := 0 TO MODM1 DO
							Seq[j].snow:=1;
					END;
			// show that we haven't expired...
			IF q mod 5000000  = 0 THEN Write('.');
		UNTIL (q=TRIALS);
		
		Writeln;
		Writeln('Experiment ends.');
		
		FOR j:=0 TO MODM1 DO BEGIN
			// sequences data collation
			WITH AllS DO
			WITH Seq[j] DO BEGIN
				FOR i := 1 TO smax DO BEGIN
					smean += snum[i];
					sprob[i] := snum[i]/stot;
					sptot += sprob[i];
				END;
				// sort probabilities
				smedi := MedianP(1,smax,sprob,sprbs);
				// use sorted array for statistics
				shi   := SPLength(1,sprbs);
				smean := smean / shi;
				ssigm := Sigma(1,shi,sprbs);
				svari := Variance(1,shi,sprbs);
				schis := ChiSquare(1,shi,sprbs,FALSE);
				sigm += ssigm; vari += svari; chis+=schis;
				// collect value-names & decide output format
				IF MODULO IN [26,95] THEN value := chr(j mod MODULO + START)
					ELSE value := Ascii2Hex(chr(j mod MODULO + START));
			END;
		END;
		// calculate averages for all values
		WITH AllS DO BEGIN
			sigm := sigm/modulo;
			vari := vari/modulo;
			chis := chis/modulo;
		END;
		// Sequences test results 
		Writeln;
		Writeln('Value  Num-s   Max-s     Sigma        Sigma**2     Chi**2');
		FOR i := 0 TO MODM1 DO
			WITH Seq[i] DO BEGIN
				Writeln(value:3,'  ',shi:6,'  ',smax:6,'    ',
					(ssigm):1:7,'    ',(svari):1:9,'   ',schis:8:5);
			END;
		Writeln('Mean:                  ',(AllS.sigm):1:7,'    ',
									(AllS.vari):1:9,'   ',AllS.chis:8:5);
		Writeln('Num-s is the number of sequences observed, Max-s the longest.');
		{$ifdef VERBOSE}
		Writeln;
		Writeln('Sample: sequence probabilities * 10E7 for each value in the set.');
		Write('Seq: ');
		FOR i := 2 TO 8 DO Write(i:9);
		Writeln;
		FOR i := 0 TO MODM1 DO
			WITH Seq[i] DO BEGIN
				Write(value:3,'  ');
				FOR j := 2 TO 8 DO
					Write(trunc(sprob[j]*10000000):9);
				Writeln;
			END;
		{$endif}
		
	EXCEPT
		Writeln('Input error. Please check your command switches.');
		Usage;
	END;

END.
