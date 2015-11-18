#!/usr/bin/python

compressed = [
    ('repz', 32),
    ('lit', 4),
    ('repz', 11),
    ('lit', 7),
    ('lit', 0),
    ('lit', 6),
    ('repz', 18),
    ('lit', 9),
    ('lit', 0),
    ('lit', 8),
    ('lit', 9),
    ('repz', 4),
    ('lit', 9),
    ('lit', 0),
    ('lit', 0),
    ('lit', 8),
    ('lit', 9),
    ('lit', 9),
    ('lit', 0),
    ('lit', 9),
    ('lit', 0),
    ('lit', 0),
    ('lit', 9),
    ('lit', 0),
    ('lit', 9),
    ('repz', 11),
    ('lit', 4),
    ('lit', 8),
    ('lit', 5),
    ('lit', 5),
    ('lit', 4),
    ('lit', 7),
    ('lit', 6),
    ('lit', 7),
    ('lit', 4),
    ('lit', 0),
    ('lit', 0),
    ('lit', 5),
    ('rep', 3),
    ('lit', 6),
    ('lit', 8),
    ('lit', 4),
    ('lit', 4),
    ('lit', 4),
    ('lit', 5),
    ('lit', 6),
    ('repz', 137),
    ('lit', 8),
    ('lit', 3),
    ('lit', 5),
    ('lit', 6),
    ('lit', 7),
    ('lit', 6),
    ('lit', 7),
    ('lit', 8),
    ('lit', 7),
    ('lit', 8),
    ('lit', 9),
    ('repz', 8),
    ('lit', 9),
    ('repz', 6),
    ('lit', 5),
    ('lit', 0),
    ('lit', 0),
    ('lit', 5),
    ('lit', 5),
    ('lit', 4),
    ('lit', 4),
    ('lit', 4),
    ('lit', 3),
    ('lit', 4),
    ('lit', 2),
    ('lit', 3),
    ('lit', 3),
    ('lit', 5),
]

lengths = []

for code, value in compressed:
    if code == 'lit':
        lengths.append(value)
    elif code == 'rep':
        l = lengths[-1]
        for x in range(value):
            lengths.append(l)
    elif code == 'repz':
        for x in range(value):
            lengths.append(0)
    else:
        raise Exception('invalid code')

#lengths.append(-1)
#lengths.append(-1)
#lengths.append(-1)
print "LENGTH: ", len(lengths)
for line in xrange(len(lengths)/8):
    print lengths[line*8:line*8+8]
if 0 < len(lengths) % 8:
    print lengths[len(lengths)/8 * 8:]
