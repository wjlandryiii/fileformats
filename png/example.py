#!/usr/bin/python

import struct
import sys
import zlib

class Chunk:
    def __init__(self, data, index):
        self.length = struct.unpack("!I", data[index:index+4])[0]
        self.ctype = struct.unpack("!I", data[index+4:index+8])[0]
        self.data = data[index+8:index+8+self.length]
        self.crc = struct.unpack("!I", data[index+8+self.length:index+8+self.length+4])[0]
        self.next_index = index + 8 + self.length + 4

def read_png(filename):
    with open(filename, "rb") as f:
        pngdata = f.read()

    magic = pngdata[:8]
    print "Magic: ", repr(magic)
    if magic != "\x89PNG\r\n\x1a\n":
        print >> sys.stderr, "Invalid file magic!"
        sys.exit(1)

    hdr_chunk = Chunk(pngdata, 8)

    if hdr_chunk.ctype != 0x49484452:
        print >> sys.stderr, "IHDR must be first chunk!"
        sys.exit(1)

    if hdr_chunk.length != 13:
        print >> sys.stderr, "IHDR length is invalid!"
        sys.exit(1)

    i = hdr_chunk.next_index

    chunk = Chunk(pngdata, i)
    while chunk.ctype != 0x49444154:
        chunk = Chunk(pngdata, chunk.next_index)
        if chunk.ctype == 0x504c5445:
            print >> sys.stderr, "Found PLTE chunk, but not implemented"
            sys.exit(1)

    data = ""
    while chunk.ctype == 0x49444154:
        data += chunk.data
        chunk = Chunk(pngdata, chunk.next_index)

    while chunk.ctype != 0x49454e44:
        if chunk.ctype == 0x504c5445:
            print >> sys.stderr, "Found PLTE chunk after IDAT chunk"
        if chunk.ctype == 0x49444154:
            print >> sys.stderr, "Found IDAT chunk after IDAT chunks"
        chunk = Chunk(pngdata, chunk.next_index)

    data = zlib.decompress(data)


    pixels = []
    for row in xrange(16):
        pixels.append([])
        r = (1 + 16 * 4) * row
        for col in xrange(16):
            i = (1 + 16 * 4) * row + 1 + col * 4
            raw_pixel = data[i:i+4]
            pixel = struct.unpack("BBBB", raw_pixel)
            pixels[row].append(pixel)
    return pixels





png = read_png("cursor1.png")

for scanline in png:
    for pixel in scanline:
        c = ' '
        if pixel[0] == 255 and pixel[1] == 255 and pixel[2] == 255 and pixel[3] == 255:
            c = '.'
        elif pixel[0] == 0 and pixel[1] == 0 and pixel[2] == 0 and pixel[3] == 255:
            c = '#'
        sys.stdout.write(c)
    sys.stdout.write("\n")
