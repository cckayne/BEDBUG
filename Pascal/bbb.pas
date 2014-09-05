// bbb: Check output from the BEDBUG512 CSPRNG 
program bbb;
uses bb512;
// mixing rounds: 7 is recommended minimum
const ROUNDS = 7;
var n: Cardinal;
	seed: string = 'Bacon';
begin
	// pull a seed from the command line
	if ParamCount>0 then seed:=ParamStr(1);
	// seed BEDBUG and mix
	bbSeedW(seed,STSZ*ROUNDS);
	// show some MOD 26 stream output
	for n:=1 to 70 do
		write(chr(bbRandom mod 26 + ord('A')));
	writeln;
end.
