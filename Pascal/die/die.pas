// die: generate a bit-stream file of pseudo-random values for DIEHARD tests.
// Usage:   >die <rng #> <seed>
// Example: >die 4 "my seed"
//(seed BB512 with seed "my seed" and generate a file named BB512.32)
// rng # in [0..13] 0=CONG, 2=ISAAC, 3=BB256, 4=BB512, 6=KISS etc.
// Output: a file named <rng-name>.32 (Example: KISS.32)
program die;

uses rng, SysUtils;

const 	blen= 16777215;//33554431; // 16777215; // desired file-length (bytes*4)

var 	key: string = 'FSBiaccenRI';
		ng : TRNG = B128;
		words: array[0..blen] of Cardinal;
		P: pointer;
		i,nw,total,fSz: Cardinal;
		fOut: file;
		fname: string = 'die.32';

procedure Usage;
	begin
		Writeln('die: generate a bit-stream file of pseudo-random values for DIEHARD tests.');
		Writeln('Usage  : >die <rng #> <seed>');
		Writeln('Example: >die 4 "my seed"');
		Writeln('(seed BB512 with seed "my seed" and generate a file named BB512.32)');
		Writeln('(rng # in [0..14] 0=CONG, 2=ISAAC, 3=BB128, 4=BB256, 5=BB512)');
		Writeln('Output: a file named <rng-name>.32 (Example: KISS.32)');
	end;
	
begin
	// check the command line
	if ParamCount <1 then begin Usage; Halt; end;
	if ParamCount>=1 then if ParamStr(1)[1] in ['a'..'z','A'..'Z','-'] then begin Usage; Halt; end;
	if ParamCount>=1 then ng := TRNG(StrToInt(ParamStr(1)));
	if ParamCount>=2 then key := ParamStr(2);
	// set the output filename
	fname := rngs[ng] + '.32';
	// seed all available RNGs
	rSeedAll(key);
	// generate a pseudo-random bitstream from the selected PRNG
	for i := 0 to blen do
		words[i] := rRandom(ng);
	// prepare and write the bitstream to file
	P := @words[0];
	Assign(fOut,fname); Rewrite(fOut,1);
	fSz := SizeOf(words);
	total:=0;
	repeat
		BlockWrite(fOut,P^,fSz,nw);
		INC(total,nw);
	until (nw=0) or (total>=fSz);
	Close(fOut);
	Writeln(IntToStr(total)+' bytes written to <'+fname+'>. ');
end.
