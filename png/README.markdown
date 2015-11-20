Portable Network Graphics (PNG)
===============================

Specifications
--------------

RFC2083
ISO/IEC 15948


Format
------

```
 +==============+ +============+  +====================+  +============+
 | 8 byte magic | | IHDR Chunk |  | CHUNKS...          |  | IEND Chunk |
 +==============+ +============+  +====================+  +============+
```

### File Magic

8 bytes long.

* Decimal values: { 137, 80, 78, 71, 13, 10, 26, 10 }
* Hex values: { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, }
* C string literal: "\x89PNG\r\n\x1a\n"


### Chunk order

* IHDR must come first.
* IEND must come last.
* Must contain one or more IDAT chunks.
* Optional chunk PLTE must precede first IDAT chunk.
  - Only zero or one PLTE chunk.

RFC2083 Section 4.3 specifies ancillary chunks, and positions.


### Chunk Format

```
 +---+---+---+---+---+---+---+---+=====================+---+---+---+---+
 |    Length     |     Type      |      Chunk Data     |      CRC
 +---+---+---+---+---+---+---+---+=====================+---+---+---+---+
```

#### Length Field

4 byte unsigned integer in network byte order.  Must be < (2^31) - 1.
0 is valid.

Length's value is the length of the data field.

#### Type Field

4 byte code specifying chunk type.  These codes are made from ascii
lower-case, upper-case, and digits (digits?).

This should be treated as an integer, not as an ascii string.

#####Standard Chunks:
* 0x49484452: "IHDR"
* 0x504c5445: "PLTE"
* 0x49444154: "IDAT"
* 0x49454e44: "IEND"

#### CRC Field

CRC run on the chunk but not on the Length field.
Algorithm is located in section 3.4 of RFC2083A


### Standard Chunks

#### IHDR Chunk

```
struct {
	uint32_t Width;
	uint32_t Height;
	uint8_t BitDepth;
	uint8_t ColorType;
	uint8_t CompressionMethod;
	uint8_t FilterMethod;
	uint8_t InterlaceMethod;
};
```

* __Width__: valid range: 0 < Width < (2^31) - 1
* __Height__: valid range: 0 < Height < (2^31) - 1
* __BitDepth__: bit depth per sample. valid values:
    1, 2, 4, 8, 16 (but not always, depends on color type)
* __ColorType__: bitflag field.
  - 1: palette used
  - 2: color used
  - 4: alpha used
* __CompressionMethod__:
  - 0: DEFLATE with 32k window
  - No other's in specifications
* __FilterMethod__:
  - 0: Adaptive method
  - No other methods are in the specification
* __InterlaceMethod__:
  - 0: No interlacing
  - 1: Adam7 interlacing

#### PLTE Chunk

Contains 1 to 256 palette entries.

An entry is 3 bytes.

The number of entries is determined by the length of the chunk.  Lengths
that are not divisible by 3 is an error.

```
struct {
	uint8_t Red;
	uint8_t Green;
	uint8_t Blue;
};
```

#### IDAT Chunk

The data of this chunk stores the compressed version of the pixel data.

Pixel data is composed of scanlines.  Scanlines are recorded from top to
bottom.  Scanlines contain an a filter byte, followed by the pixels
recorded from left to right (after filtering if one is specified).

There can be multiple IDAT chunks.  They must be consecutive, and not include
any other chunk types between them.  The compressed pixel data is the
concatenation of all IDAT chunks.  There is no meaning behind where IDAT
chunks split data.


#### IEND Chunk

Must occur as the last chunk.  The data field is left empty.
