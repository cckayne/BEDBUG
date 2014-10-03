motet - The MOTET Cipher & SE Scrambler
MOTET  Copyright (C) C.C.Kayne 2014, GNU GPL V.3, cckayne@gmail.com

This program comes with absolutely no warranty:
It is free software: You are welcome to redistribute it.

Usage  : >motet <msg> <key> <cipher mode> <cipher type> <output> <csprng>
Example: >motet "my message" "my strong key" e m a 4
(Encrypt "my message" on key "my strong key" with Caesar/MIX using MOTE8)
CSPRNGs: ISAAC(0) BB128(1) BB256(2) BB512(3) MOTE8(4) MOTE16(5) MOTE32(6)
Maximum message length: 20480 B; maximum key length: 2047 B.
Minimum message length:     2 B; minimum key length:    8 B.

[e] cipher mode: Encrypt
[d] cipher mode: Decrypt
[c] cipher type: Caesar/MOD
[m] cipher type: Caesar/MIX
[v] cipher type: Vernam/XOR
[a] output form: ASCII A-Z
[h] output form: Hexadecimal

Arguments must be passed in the usage order, and all are required.


What is MOTET?
--------------

MOTET is a tiny, fast super-encipherment application written in C, featuring the new CSPRNG/stream ciphers MOTE and BEDBUG in each of their three variants, as well as the "gold standard" among ciphers, ISAAC. 

MOTET brings multiple levels of encryption, including a ciphertext-hash, a Vigenere mixing function, a choice of Vernam, Caesar MOD, or Caesar MIX ciphering on the primary key-stream, plus a deeply scrambled "outer shell" as a final super-encryption stage. A unique nonce IV guarantees that each ciphertext will differ radically from every other, even on the same message and identical key.


Why MOTET?
----------

One of the aims of MOTET has been to demonstrate the portability of the basic SES cipher algorithm and its ease of implementation in a popular language other than Pascal and with CSPRNGs other than ISAAC (although ISAAC is included as a MOTET option). C may not have Free Pascal's advanced string-handling functions or object model, but it was surprising what could be accomplished in a couple of days translating most of the SES cipher routines to C.

MOTET lacks only the inner Vigenere keyword-loop and the Keccak (SHA-3) key derivation germane to SES. Despite this, MOTET still manages 4 super-encipherments on any message. The real elegance of MOTET is that it uses the new MOTE and BEDBUG CSPRNGs for everything, including hashing and key-stretching, gaining in execution speed while keeping it all simple and compact.

MOTET is a hot-rod SES stripped down to the metal.

Cryptographically, MOTET is _strong_ when measured against any of today's vaunted standards (AES and Triple DES included), though doubtless not quite as strong as SES in its present incarnation, lacking as it does the inner cipher cascade. 

However, "more than good enough" justly describes MOTET, if only because it is built upon Bob Jenkins' expert PRNG insights and this author's 30-plus years' experience. MOTET is certainly orders of magnitude superior to the likes of RC4. Use MOTET with confidence.

I think you'll find that MOTET confirms how _easy_ truly strong cryptography is to implement, while avoiding all those NIST/academic chestnuts with their NSA backdoors.

Please note that the nonce-scrambling and Vigenere mixing that tops off MOTET's cipher sequence is only available with MOD 26 ASCII output. MOD 95 hexadecimal and Vernam will do key-scheduling but will omit the signature SES super-encryption. So if you desire the diffusion and unique-ciphertext feature of SES when using MOTET, please make sure to set the "a" option with either "c" (Caesar/MOD) or "m" (Caesar/MIX).

Encipherment example:

Example: >motet "my message" "my strong key" e m a
Output : >AHFQZPHUZNKMCVKSMWKRKFACXOD
(Encrypt "my message" on key "my strong key" using Caesar/MIX and ASCII output.)

Decipherment example:

Example: >motet AHFQZPHUZNKMCVKSMWKRKFACXOD "my strong key" d m a
Output : >my message


Challenge
---------

You are cordially invited to try breaking the three short MOTET encipherments included in the file Challenge.txt. In the unlikely event you succeed, a negotiable cash prize is offered by way of remuneration. Please get in touch at the email address listed below, giving details of your decryption(s). To receive the money, a PayPal account will be required.


Compiling MOTET
---------------

MOTET compilation has been tested with tcc in Win32 and gcc on Linux.

To compile at the command prompt, type

tcc motet.c mystring.c iscutils.c ckcipher.c scrambler.c csprng.c ISAAC.c bb128.c bb256.c bb512.c mote8.c mote16.c mote32.c

or

gcc motet.c mystring.c iscutils.c ckcipher.c scrambler.c csprng.c ISAAC.c bb128.c bb256.c bb512.c mote8.c mote16.c mote32.c -o motet

or run make.bat or make.sh included with the source.

Please note that MS Visual C (cl) will not compile this version of MOTET without errors due to that compiler's non-C99 compliance.

If you'd like to view cipher-log output during test runs of MOTET, please uncomment "//#define LOG" in motet.c prior to re-building.

For your convenience, pre-compiled MOTET binaries are included in the relevant folders.
 
Enjoy!

C.C.Kayne
cckayne@gmail.com
https://plus.google.com/u/2/114160148833284687117
https://code.google.com/p/ses-super-encypherment-scrambler/
https://github.com/cckayne/ses-scrambler
https://sourceforge.net/projects/sessuperencyphermentscrambler/
https://github.com/cckayne/BEDBUG
https://code.google.com/p/bedbug-csprng-stream-cipher/
https://sourceforge.net/projects/bedbug-csprng-stream-cipher/
https://github.com/cckayne/MOTE
https://code.google.com/p/mote-csprng-stream-cipher/
