DEFLATE
=======

Spec: RFC1951

General lossless compression format in the LZ77 family.

The DEFLATE algorithm is used by:
* zlib
* png
* zip format (?)
* gzip (?)

A simple implementation of INFLATE is in the zlib source under /contrib/puff


RFC1951 section 3.1.1:

Bits are read from least significant to most significant.
(TODO: what multi-bit integer ?)

```

These two bytes:
+--------+--------+
|00001111|10101010|
+--------+--------+

are read as the bit stream:

"1111000001010101"

```
