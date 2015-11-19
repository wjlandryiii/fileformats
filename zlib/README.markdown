zlib Format
===========

Specification
-------------

Specs are found in RFC1950

```
           { present if FDICT set  }
+---+---+  {   +---+---+---+---+   }  +=================+---+---+---+---+
|CMF|FLG|  {   |     DICTID    |   }  | compressed data |    ADLER32    |
+---+---+  {   +---+---+---+---+   }  +=================+---+---+---+---+
           {                       }
```

CMF
---

* __CM (bits 0-3)__: Compression Method (8 == DEFLATE w/ <= 32k window)
* __CINFO (bits 4-7)__: Compression Info (CM==8 && 7 == 32k window)

FLG
---

* __FCHECK (bits 0-4)__:  Find (CMF*256 + FLG) % 31 == 0
  - CMF == 0x78 && FDICT == 0 && FLEVEL == 0: ??
  - CMF == 0x78 && FDICT == 0 && FLEVEL == 1: ??
  - CMF == 0x78 && FDICT == 0 && FLEVEL == 2: ??
  - CMF == 0x78 && FDICT == 0 && FLEVEL == 3: 26
* __FDICT (bit 5)__: DICTID is present
* __FLEVEL (bits 6-7)__: level of compression used by compressor

DICTID
------

Adler32 of a preset dictionary.  This value is passed to the decompressor
so it knows what dictionary to used to decompress the stream.  If this
value is set and the decompressor doesn't know the dictionary for the
DICTID, an error must be generate.  I don't think this field is used
in with DEFLATE.

ADLER32
-------

The Adler-32 checksum of the uncompressed data stored in network byte-order.
