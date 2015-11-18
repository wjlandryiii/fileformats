Example 1: BTYPE 00b (uncompressed)
===================================

Example DEFLATE data:

```
$ echo -n Hello World! \
> | python -c 'import zlib; import sys; sys.stdout.write(zlib.compress(sys.stdin.read(), 0)[2:-4])' \
> | xxd -b
0000000: 00000001 00001100 00000000 11110011 11111111 01001000  .....H
0000006: 01100101 01101100 01101100 01101111 00100000 01010111  ello W
000000c: 01101111 01110010 01101100 01100100 00100001           orld!
```


Format
------

Data in an uncompressed block is in the format:

```
          +-------+--------+------+-------+=======================+
 (header) |      LEN       |    NLEN      |   LEN bytes of DATA   |
          +-------+--------+------+-------+=======================+
```

Note that the block header does not have to be byte aligned but LEN, NLEN, and
the uncompressed DATA is.  In this example, the header is found at the
beginning of the file and is byte aligned.



Block Header
------------

As per RFC1951, the block header is 3 bits long.  The first bit is BFINAL,
and the following two bits is BTYPE.

BFINAL is 1b when it is the last block in the stream, and 0b when there
are more block to follow.


BTYPE is 00b for uncompressed data.


###header of the example:

```
+------------+
| 00000 00 1 |
+-------^^-^-+
        |  |
        |   \ BFINAL: 1b
        |
         \ BTYPE: 00b

````

The bits are read from least significant to most significant. This is contrary
to most binary data formats, and important to note.

To extract the block header from the example data:

```
	bfinal = data[0] & 1;
	btype = (data[0] >> 1) & 3;

	assert(final == 1);
	assert(btype == 0);
```


LEN and NLEN Fields
-------------------

LEN and NLEN are byte aligned.  All bits after the header up to the next byte
are ignored.

LEN is a 16-bit integer in little-endian byte-order and value is the
length of the uncompressed data.

NLEN is the bitwise inverse of LEN.


### Example LEN and NLEN

```
+----------+----------+----------+----------+
|       LEN=0x000c    |      NLEN=0xfff3    |
| LSB      | MSB      | LSB      | MSB      |
| 00001100 | 00000000 | 11110011 | 11111111 |
+----------+----------+----------+----------+
```

To extract LEN and NLEN from the example data:

```
	len = (data[2] << 8) | data[1];
	nlen = (data[4] << 8) | data[3];

	assert(len == (~nlen & 0xffff));
	assert(len == 0x000c);
	assert(nlen == 0xfff3);
```


