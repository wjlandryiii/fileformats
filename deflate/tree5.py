#!/usr/bin/python

lengths = [
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    4, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 7, 0, 6, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 9, 0, 8, 9, 0, 0, 0,
    0, 9, 0, 0, 8, 9, 9, 0,
    9, 0, 0, 9, 0, 9, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 4, 8, 5, 5, 4, 7, 6,
    7, 4, 0, 0, 5, 5, 5, 5,
    6, 8, 4, 4, 4, 5, 6, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    8, 3, 5, 6, 7, 6, 7, 8,
    7, 8, 9, 0, 0, 0, 0, 0,
    0, 0, 0, 9, 0, 0, 0, 0,
    0, 0, 5, 0, 0, 5, 5, 4,
    4, 4, 3, 4, 2, 3, 3, 5,
]

lengths = lengths[:276]

print "Length: ", len(lengths), 276

print "count: ", len(lengths)
alphabet = [str(i) for i, x in enumerate(lengths)]

print "Input:"
print ""
print "Symbol  Length"
print "------  ------"
for symbol, length in zip(alphabet, lengths):
    if length != 0:
        print "{:2s}      {:d}".format(symbol, length)
print ""
print ""


# RFC1951 3.2.2 page 8:
#   1)  Count the number of codes for each code length.  Let
#       bl_count[N] be the number of codes of length N, N >= 1.

bl_count = [0 for x in range(len(alphabet))]
for l in lengths:
    bl_count[l] += 1

print "After step 1:"
print ""
print "N      bl_count[N]"
print "-      -----------"
for i, c in enumerate(bl_count):
    if 0 < c:
        print "{:d}      {:d}".format(i, c)
print ""
print ""

# RFC1951 3.2.2 page 8:
#   2)  Find the numerical value of the smallest code for each
#       code length:

max_code = 0
for i, x in enumerate(bl_count):
    if x != 0:
        max_code = i + 1

next_code = [0 for x in range(max_code)]

code = 0
bl_count[0] = 0
for bits in xrange(1, max_code):
    code = (code + bl_count[bits-1]) << 1
    next_code[bits] = code

print "After step 2:"
print ""
print "N      next_code[N]"
print "-      ------------"
for i, n in enumerate(next_code):
    if 0 < n:
        print "{:d}      {:<2d}".format(i, n)
print ""
print ""


# RFC1951 3.2.2 page 8:
#    3)  Assign numerical values to all codes, using consecutive
#        values for all codes of the same length with the base
#        values determined at step 2. Codes that are never used
#        (which have a bit length of zero) must not be assigned a
#        value.

codes = ['x' for x in range(len(alphabet))]

for n in range(len(alphabet)):
    l = lengths[n]
    if l != 0:
        codes[n] = ("{:0" + str(l) + "b}" ).format(next_code[l])
        next_code[l] += 1

print "After step 3:"
print ""
print "Symbol Length   Code"
print "------ ------   ----"
for sym, code in zip(alphabet, codes):
    if code != 'x':
        print "{:2s}     {:2d}       {:>4s}".format(sym, len(code), code)
print ""
print ""
