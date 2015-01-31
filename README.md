# toy quad tree implmentation

- 'r' switches rendermode
- 'x' increases the size of the cursor
- 'z' decreases the size of the cursor
- left mouse button carves empty space into the quadtree
- right mouse button fills space in the quadtree

# quad trees
- The width/height in nodes of level n is 2^n
- The number of nodes in level n is 4^n
- The total number of nodes in a quadtree of size n is sum(4^i) i..n
- Tthe base node for level n can be calculated by summing the levels up to n-1

There's a few ways to allocate/address quadtree nodes:
1/ Creating them in a depth first fashion will mix the levels up in memory. Might have better cache coherency during depth first traversal?

2/ Calculating the offset to the level and then indexing the nodes by x,y will lay them out in linear order in memory. The child nodes of node x,y are x*2, y*2, x*2+1, y*2, x*2, y*2+1, x*2+1, y*2+1

3/ As above but the four children can be laid out in in a group of 4 so the children of node x,y would be base(x,y) + 0 .. 4, where base is a function like x * y * 4. This would be useful to keep node size down as only a single child pointer is needed.

Each node can be uniquely address by (level, x, y), it should be possible to calculate the addresses using the 2 addressing schemes above with these parameters

Larger trees might work well with a scheme similar to virtual texturing if the entire tree doesn't need to be resident in memory?

It's possible to do very quick contents and distance queries on a quadtree.

# node contents

Each node can contain 'content.' At the moment these are either solid or empty. Each node stores two content fields. These are the 'and'd' and 'or'd' fields of the node's child contents. These fields will be the same on a leaf node as they are combined with 'nothing'

To query the contents of a point just recurse down the tree until a leaf node is hit.

# distance queries

Fast distance queries are possible so long as the distance is trying to find the boundary edge of the contents and not the internal distance.

The distance query searches for the minimal distance to a node of a given content. The query can be accelerated by the fact that:
- 1/ If all nodes below the node have the same contents then we can just test the current node and discard the rest of the subtree (and'd contents field)
- 2/ if all nodes below the node don't have any of the contents that we're searching for then we can discard the subtree without testing (or'd contents field)

This will only work if searching for the bondary edges of the boxes, if the point is within the content the test will break down as it returns the distance to the local box rather than the global box. To fix this it's possible to switch the content test depending on if the node is solid or empty space

Find the minimal distance on a node equates to closest feature detection on a box. To do this the code first:
1/ Translates point relative to the box's origin.
2/ Since the box is symmetrical we only need to test the positive quadrant. If the points aren't in the positive qudrant the x and y coordinates are mirrored about the axis so that they are.
3/ There are now four regions the point could be in. +y, +xy, +x and inside the box. Rather than test the width and height of the box it's easier to translate the point realative to the upper corner of the box.
4/ The test now equates to testing if the x and y coordinates are negative
