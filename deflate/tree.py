#!/usr/bin/python

symbols = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
#encoded = [3, 3, 3, 3, 3, 2, 4, 4]
encoded = [4, 6, 7, 1, 1]
codes = [0,0,0,0,0,0,0,0]

bl_count = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0]
next_code = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]

for x in encoded:
    bl_count[x] += 1

print bl_count

code = 0
bl_count[0] = 0 # is this line right???
for bits in xrange(1, 16):
    code = (code + bl_count[bits-1]) << 1
    next_code[bits] = code

print next_code

for n in xrange(0, len(encoded)):
    l = encoded[n]
    if l != 0:
        codes[n] = next_code[l]
        next_code[l] += 1

for n in xrange(0, len(encoded)):
    l = encoded[n]
    s = symbols[n]
    c = codes[n]

    print "{:s} {:b}".format(s, l, c)
