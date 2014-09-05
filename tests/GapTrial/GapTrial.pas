{$mode delphi}
{ $define distrib}
{ $define caesar}
{$define VERBOSE}
PROGRAM GapTrial;

USES rng, MyStrUtils, SysUtils, Math;

TYPE PArray = ARRAY Of EXTENDED;
	 CArray = ARRAY Of CARDINAL;
	 SArray = ARRAY Of STRING;
	 {$ifdef caesar}
	 TMCipher = (mEncipher, mDecipher, mNone);
	 {$endif}
// Default values
CONST 	VERSION = '1.0.0';
		COPYRT  = 'GapTrial is Copyright (c) C.C.Kayne, 2014 - cckayne@gmail.com';
		PDOM	= 'GapTrial is released into the Public Domain.';
		MAXCARD = $FFFFFFFF;
		MAXQ    = $FFFFFFFFFFFFFFFF;
		EX = 1000;
VAR TRIALS:	QWord = 100000000;
	MAXGAP: CARDINAL = 1000;
	{$ifdef distrib}
	MINTHROWS:	Cardinal = 1000000; // was 1000000
	{$else}
	MINTHROWS:	Cardinal = 100; // was 1000000
	{$endif}
	MODM1:		Cardinal = 25;
	MODULO:		Cardinal = 26;
	TM:			Cardinal = 25; // temp mod for set-checks
	START:		Cardinal = ord('A');
	seed: STRING = 'Monte Carlo Mod';
	RNG: TRNG = B512;
	
	i,j,r: 	CARDINAL;
	q:		QWord = 0;

	// spacings context
	TYPE TALLG = RECORD
		mean: EXTENDED;
		sigm: EXTENDED;
		vari: EXTENDED;
		chis: EXTENDED;
	END;
	VAR AllG: TAllG;
	
	TYPE TGap = RECORD
		value: STRING[2];
		//raw experimental data
		gmin:  Cardinal;
		gmax:  Cardinal;
		gnow:  Cardinal;		
		gtot:  Cardinal;
		ghi :  Cardinal;
		//total of observed spacings
		gnum:  CArray;
		//probabilities of observed spacings
		gprob: PArray;
		// sorted probabilities
		gprbs: PArray;
		gptot: Extended;
		gmean: Extended;
		gmedi: Extended;
		gsigm: Extended;
		gvari: Extended;
		gchis: Extended;
	END;
		GapArray = ARRAY OF TGap;
	VAR Gap: GapArray;
	

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
		Writeln('Usage:   >gaptrial <modulo> <# of trials> <seed> <RNG>');
		Writeln('Example: >gaptrial 26 32 "my seed" 2');
		Writeln('Permitted range for # trials: ',MINTHROWS,'-2**64.');
		Writeln('(If # given is in [8..64], 2**# is assumed.)');
		Writeln('Available RNGs: [0..14] 2=ISAAC 3=BB128 4=BB256 5=BB512.');
		Writeln('All command line parameters are optional.');
		Writeln('Default: mod 26 BB512 with 100000000 trials.');
		Writeln;
	END;

PROCEDURE Banner;
	BEGIN
		Writeln('GapTrial: Spacings between values emitted by a PRNG under the MOD operation.');
		{$ifdef distrib}
		Writeln(COPYRT);
		Writeln(PDOM);
		Writeln('GapTrial version ',VERSION);
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
		SetLength(gap,MODULO);
		MAXGAP := MODM1 * MODM1 + 1000;
		FOR i := 0 to MODM1 DO
			WITH gap[i] DO BEGIN
				SetLength(gnum ,MAXGAP);
				SetLength(gprob,MAXGAP);
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
		Writeln('Experiment: ',TRIALS,' ',RNGs[RNG],' spacing trials using MOD ',IntToStr(MODULO));
		Writeln('Experiment starts');
    
		// initialize spacings arrays
		FOR i:=0 TO MODM1 DO
			WITH gap[i] DO BEGIN
				value := '';
				gnow:=0; gmin:=MAXCARD; gmax:=0; gtot:=0;
				gmean:=0.0; gsigm:=0.0; gvari:=0.0; gchis:=0.0; gptot:=0.0;
				FOR j := 1 TO MAXGAP DO
					gnum[j] := 0;
			END;
		WITH AllG DO BEGIN
			mean:=0.0; sigm:=0.0; vari:=0.0; chis:=0.0;
		END;
				
		// initiate Monte Carlo experiment
		REPEAT
			inc(q);
			// Cast the die
			{$ifdef caesar}
			r:=Caesar(mDecipher, rRandom(RNG) mod MODULO, rRandom(RNG) mod MODULO, modulo);
			{$else}
			r:=rRandom(RNG);
			r:=r mod MODULO;
			{$endif}
			// update gap results
			FOR j := 0 TO MODM1 DO
				inc(gap[j].gnow);
			WITH gap[r] DO BEGIN
				{$ifdef VERBOSE}
				IF gnow>gmax THEN
					Write(chr(r+ord('A')),gnow,'.');
				{$endif}
				IF gnow<gmin THEN gmin:=gnow;
				IF gnow>gmax THEN gmax:=gnow;
				// update totals for each gap
				inc(gnum[gnow]); 
				inc(gtot);
				// reset gap counter
				gnow := 0;
			END;
			// show that we haven't expired...
			IF q mod 5000000  = 0 THEN Write('.');
		UNTIL (q=TRIALS);
		
		Writeln;
		Writeln('Experiment ends.');
		
		FOR j:=0 TO MODM1 DO BEGIN
			// spacings data collation
			WITH AllG DO
			WITH gap[j] DO BEGIN
				FOR i := 1 TO gmax DO BEGIN
					gmean += gnum[i];
					gprob[i] := gnum[i]/gtot;
					gptot += gprob[i];
				END;
				// sort probabilities
				gmedi := MedianP(1,gmax,gprob,gprbs);
				// use sorted array for statistics
				ghi   := SPLength(1,gprbs);
				gmean := gmean / ghi;
				gsigm := Sigma(1,ghi,gprbs);
				gvari := Variance(1,ghi,gprbs);
				gchis := ChiSquare(1,ghi,gprbs,FALSE);
				sigm += gsigm; vari += gvari; chis+=gchis;
				// collect value-names & decide output format
				IF MODULO IN [26,95] THEN value := chr(j mod MODULO + START)
					ELSE value := Ascii2Hex(chr(j mod MODULO + START));
			END;
		END;
		// calculate averages for all values
		WITH AllG DO BEGIN
			sigm := sigm/modulo;
			vari := vari/modulo;
			chis := chis/modulo;
		END;
		// Spacings test results 
		Writeln;
		Writeln('Value  Num-g   Max-g     Sigma        Sigma**2     Chi**2');
		FOR i := 0 TO MODM1 DO
			WITH gap[i] DO BEGIN
				Writeln(value:3,'  ',ghi:6,'  ',gmax:6,'    ',
					(gsigm):1:7,'    ',(gvari):1:9,'   ',gchis:8:5);
			END;
		Writeln('Mean:                  ',(AllG.sigm):1:7,'    ',
									(AllG.vari):1:9,'   ',AllG.chis:8:5);
		Writeln('Num-g is the number of spacings observed, Max-g the widest.');
		{$ifdef VERBOSE}
		Writeln;
		Writeln('Sample: spacing probabilities * 10E5 for each value in the set.');
		Write('Gap: ');
		FOR i := 1 TO 12 DO Write(i:5);
		Writeln;
		FOR i := 0 TO MODM1 DO
			WITH gap[i] DO BEGIN
				Write(value:3,'  ');
				FOR j := 1 TO 12 DO
					Write(trunc(gprob[j]*100000):5);
				Writeln;
			END;
		{$endif}
		
	EXCEPT
		Writeln('Input error. Please check your command switches.');
		Usage;
	END;

END.
