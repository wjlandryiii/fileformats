#!/usr/bin/python

pairs = [
        ('A', '010'),
        ('B', '011'),
        ('C', '100'),
        ('D', '101'),
        ('E', '110'),
        ('F', '00'),
        ('G', '1110'),
        ('H', '1111'),
]

NODE_INDEX = 0

class TreeNode:
    def __init__(self):
        global NODE_INDEX
        self.index = NODE_INDEX
        NODE_INDEX += 1
        self.left = None
        self.right = None
        self.isLeaf = False
        self.code = ''
        self.symbol = ''

tree = TreeNode()

for symbol, code in pairs:
    node = tree
    for bit in code:
        if bit == '0':
            if node.left is None:
                node.left = TreeNode()
            node = node.left
        else:
            if node.right is None:
                node.right = TreeNode()
            node = node.right
    node.isLeaf = True
    node.symbol = symbol
    node.code = code


def traverse(node):
    if node.isLeaf:
        """[shape=record, label="{{I|11}|000}"]"""
        print '\t{:d} [shape=record, label="{{{:s}| {:s}}}"];'.format(node.index, node.symbol, node.code)
    else:
        print '\t{:d} [label=""]'.format(node.index)
    if node.left is not None:
        print "\tedge [label=0];"
        print "\t{:d} -> {:d};".format(node.index, node.left.index)
        traverse(node.left)
    if node.right is not None:
        print "\tedge [label=1];"
        print "\t{:d} -> {:d};".format(node.index, node.right.index)
        traverse(node.right)

print "digraph graph0 {"
traverse(tree)
print "}"


