Example 2: BTYPE 01 (fixed huffman codes)
=========================================

Huffman Coding
==============

Example from RFC1951
```
                          /\              Symbol    Code
                         0  1             ------    ----
                        /    \                A      00
                       /\     B               B       1
                      0  1                    C     011
                     /    \                   D     010
                    A     /\
                         0  1
                        /    \
                       D      C
```

maybe some nonsense...

Binary Tree Structure
=====================

Note: Most implementations of DEFLATE decompressors do not actually build
tree structures, but they are clever solutions not in the scope of a naive
solution.

A binary tree structure can be stored in an array.

To find the index of a left child of an element:
	left_child = index * 2 + 1;

To find the right child:
	right_child = index * 2 + 2;

tree size:
2 ^ (n + 1) - 1
```
Bits    Nodes
----    -----
 1       3
 2       7
 3       15
```



DEFLATE codes
=============

DEFLATE uses sets of codes.  One set codes for literal values and length
values, the other set codes for distance values.


