#!/usr/bin/python


class TreeNode:
    def __init__(self):
        self.left = None;
        self.right = None;
        self.leaf = False;
        self.value = -1;


tree = TreeNode()


for x in xrange(0, 288):
    n = tree

    if 0 <= x and x <= 143:
        code = x + int("00110000", 2) - 0
        bits = 8 - 1
    elif 144 <= x and x <= 255:
        code = x + int("110010000", 2) - 144
        bits = 9 - 1
    elif 256 <= x and x <= 279:
        code = x + int("0000000", 2) - 256
        bits = 7 - 1
    elif 280 <= x and x <= 287:
        code = x + int("11000000", 2) - 280
        bits = 8 - 1

    #for y in xrange(0, bits+1):
    for y in xrange(bits, -1, -1):
        if (code & (1<<y)) >> y == 1:
            if n.right is None:
                n.right = TreeNode()
            n = n.right
        else:
            if n.left is None:
                n.left = TreeNode()
            n = n.left

    if n.leaf:
        print x, "collision with: ", n.value
    n.leaf = True
    n.value = x

#def walk(node, path):
#    if node.leaf:
#        print path, node.value
#    else:
#        if node.left is not None:
#            walk(node.left, path + "0")
#        else:
#            print "WHAT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
#        if node.right is not None:
#            walk(node.right, path + "1")
#        else:
#            print "WHATWHATWHAT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
#
#walk(tree, "")

# 122
# 122
# 168
# 256

code = "10101010" # -> 122 ('z')
code += "10101010" # -> 122 ('z')
code += "110000100000010000000000"

n = tree
for x in code:
    if x == '0':
        n = n.left
    else:
        n = n.right
    if n.leaf:
        if n.value < 256:
            print n.value, repr(chr(n.value))
        else:
            print n.value, hex(n.value)
        n = tree
