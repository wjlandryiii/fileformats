#!/usr/bin/python

import zlib
import struct

message = "Hello World!"

compressed = zlib.compress(message, 9)

CMF = ord(compressed[0])
CM = CMF & 0x0f
CINFO = (CMF & 0xf0) >> 4

FLG = ord(compressed[1])
FCHECK = FLG & 0x1f
FDICT = (FLG & 0x20) >> 5
FLEVEL = (FLG & 0xc0) >> 6

print "CMF: 0x{:02x}".format(CMF)
print "  CM   :", CM
print "  CINFO:", CINFO
print ""

print "FLG: 0x{:02x}".format(FLG)
print "  FCHECK:", FCHECK, 31 - (((CMF * 256) + (FLG & 0xe0)) % 31)
print "  FDICT :", FDICT
print "  FLEVEL:", FLEVEL
print ""

if FDICT == 1:
    DICTID = struct.unpack("!I", compressed[2:6])[0]
    print "DICTID: 0x{:08x}".format(DICTID)

ADLER32 = struct.unpack("!I", compressed[-4:])[0]

print "ADLER32: 0x{:08x} (0x{:08x})".format(ADLER32, zlib.adler32(message))
