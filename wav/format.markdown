WAV
===

Wave format uses RIFF.

RIFF is little endian ("intel byte order").
RIFX is big endian ("motorola byte order").

```
$ xxd -g 1 sine.wav  | head
0000000: 52 49 46 46 ac 58 01 00 57 41 56 45 66 6d 74 20  RIFF.X..WAVEfmt
0000010: 10 00 00 00 01 00 01 00 44 ac 00 00 88 58 01 00  ........D....X..
0000020: 02 00 10 00 64 61 74 61 88 58 01 00 00 00 04 08  ....data.X......
0000030: 01 10 ee 17 c2 1f 77 27 04 2f 61 36 88 3d 71 44  ......w'./a6.=qD
0000040: 16 4b 6e 51 75 57 24 5d 75 62 63 67 ea 6b 03 70  .KnQuW$]ubcg.k.p
0000050: ac 73 e1 76 9e 79 e1 7b a7 7d ee 7e b7 7f fe 7f  .s.v.y.{.}.~....
0000060: c5 7f 0c 7f d2 7d 1a 7c e6 79 36 77 0f 74 73 70  .....}.|.y6w.tsp
0000070: 66 6c ec 67 0a 63 c4 5d 1f 58 22 52 d2 4b 36 45  fl.g.c.].X"R.K6E
0000080: 55 3e 34 37 dd 2f 55 28 a4 20 d3 18 e9 10 ed 08  U>47./U(. ......
0000090: e9 00 e5 f8 e7 f0 f8 e8 20 e1 67 d9 d6 d1 72 ca  ........ .g...r.
```



Chunks
------

A RIFF file is made of one or more chunks:

```
struct chunk {
	uint32_t ckID;
	uint32_t ckSize;
	uint8_t ckData[0]; // size is ckSize
};
```

* _ckID_: 4 character code. identifies structure of ckData.
* _ckSize_: little endian size of ckData in bytes. does not include any padding.
* _ckData_: 2-byte aligned data.  If the size is odd, pad end with null byte.

###ckID Values

When the ckID is 0x46464952 ('RIFF'), it is a `RIFF Chunk`


RIFF Forms
----------

The first four bytes of a RIFF chunk data section specifies its form.

Form Types:

* 'PAL '
* 'RDIB'
* 'RMID'
* 'RMMP'
* 'WAVE'

WAVE Form
---------

A WAVE Form is a collection of `struct chunk`'s with specific ckID values.

###ckID Values

* 'fmt '
* 'fact'
* 'cue '
* 'plst'
* associated data list?
* 'data'

###fmt Chunk

```
struct {
	uint16_t formatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSecond;
	uint32_t avgBytesPerSecond;
	uint16_t alignToSize;
};
```

* _formatTag_ : what format the data is in, PCM/etc.
* _nChannels_ : 1 mono, 2 stereo, etc.
* _nSamplesPerSecond_ : sample rate, e.g. 44100
* _avgBytesPerSecond_ : e.g 88200 for mono 16-bit samples at 44100




