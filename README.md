toy quad tree implmentation

'r' switches rendermode
'x' increases the size of the cursor
'z' decreases the size of the cursor
left mouse button carves empty space into the quadtree
right mouse button fills space in the quadtree

the number of nodes in level n is 4^n
the total number of nodes in a quadtree of size n is sum(4^i) i..n
the base node for level n can be calculated by summing the levels up to n-1

there's a few ways to allocate/address quadtree nodes
- creating them in a depth first fashion will mix the levels up in linear order. might have better cache coherency during traversal?
- calculating the offset to the level and then indexing the nodes by x,y will lay them out in linear order in memory. the child nodes of node x,y are x*2, y*2, x*2+1, y*2, x*2, y*2+1, x*2+1, y*2+1
- as above but the four children can be laid out in in a group of 4 so the children of node x,y would be base(x,y) + 0, base(x,y) + 1, where base is a function like x * y *4

each node can be uniquely address by level, x, y, it should be possible to calculate the addresses using the 2 addressing schemes above with these parameters
