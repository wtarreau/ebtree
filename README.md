# Elastic Binary Tree (ebtree)

## Abstract

EBTree is a form of binary search tree that combines intermediary and final
nodes into a single one so as never to have to allocate an intermediary node
during insertion: the added leaf node contains its own storage to place the
node into the tree. This comes with several benefits such as O(1) deletion
and fast insertion. These are particularly suited for fast-changing data such
as scheduler and excel at string and memory block lookups.

The more detailed model and properties are
[described here](http://wtarreau.blogspot.com/2011/12/elastic-binary-trees-ebtree.html).

A presentation of their application to the HAProxy load balancer was made at
QCon in 2019. Video and slides are available
[here](https://www.infoq.com/presentations/ebtree-design/).

## Versions, branches, variants

The default branch contains development code that's expected to stay. Some
more aggressive changes were made in various experimental branches (all named
with a date). Some of these changes include multiple addressing methods, and
a more generic naming of the functions depending on the key type and addressing
methods. They have not yet been merged as not yet considered satisfying.

The production branch is `stable-6.0` and it is actively maintained. It may
occasionally see a new function appear provided that it doesn't affect existing
code. **In case of doubt, this is the branch to be used**.

A more compact version called **Compact Elastic Binary Tree** (*cebtree*),
following the same indexing principles but with a much more compact form (only
requires two pointers per node) had been experimented on for many years and was
finally extracted as a [standalone project](https://github.com/wtarreau/cebtree).
It currently doesn't support duplicate keys, and all operations are in
`O(logN)`, and strings cannot be optimized to restart from the previous offset,
making this form slightly slower than EBTree for fast-changing data sets.
However they can very efficiently replace linked lists with the same storage
for anything related to configuration elements for example.
