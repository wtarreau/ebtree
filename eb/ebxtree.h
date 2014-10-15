/*
 * Elastic Binary Trees - generic macros and structures.
 *
 * Copyright (C) 2000-2015 Willy Tarreau - w@1wt.eu
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */



/*
  General idea:
  -------------
  In a radix binary tree, we may have up to 2N-1 nodes for N keys if all of
  them are leaves. If we find a way to differentiate intermediate nodes (later
  called "nodes") and final nodes (later called "leaves"), and we associate
  them by two, it is possible to build sort of a self-contained radix tree with
  intermediate nodes always present. It will not be as cheap as the ultree for
  optimal cases as shown below, but the optimal case almost never happens :

  Eg, to store 8, 10, 12, 13, 14 :

             ultree          this theorical tree

               8                   8
              / \                 / \
             10 12               10 12
               /  \                /  \
              13  14              12  14
                                 / \
                                12 13

   Note that on real-world tests (with a scheduler), is was verified that the
   case with data on an intermediate node never happens. This is because the
   data spectrum is too large for such coincidences to happen. It would require
   for instance that a task has its expiration time at an exact second, with
   other tasks sharing that second. This is too rare to try to optimize for it.

   What is interesting is that the node will only be added above the leaf when
   necessary, which implies that it will always remain somewhere above it. So
   both the leaf and the node can share the exact value of the leaf, because
   when going down the node, the bit mask will be applied to comparisons. So we
   are tempted to have one single key shared between the node and the leaf.

   The bit only serves the nodes, and the dups only serve the leaves. So we can
   put a lot of information in common. This results in one single entity with
   two branch pointers and two parent pointers, one for the node part, and one
   for the leaf part :

              node's         leaf's
              parent         parent
                |              |
              [node]         [leaf]
               / \
           left   right
         branch   branch

   The node may very well refer to its leaf counterpart in one of its branches,
   indicating that its own leaf is just below it :

              node's
              parent
                |
              [node]
               / \
           left  [leaf]
         branch

   Adding keys in such a tree simply consists in inserting nodes between
   other nodes and/or leaves :

                [root]
                  |
               [node2]
                 / \
          [leaf1]   [node3]
                      / \
               [leaf2]   [leaf3]

   On this diagram, we notice that [node2] and [leaf2] have been pulled away
   from each other due to the insertion of [node3], just as if there would be
   an elastic between both parts. This elastic-like behaviour gave its name to
   the tree : "Elastic Binary Tree", or "EBtree". The entity which associates a
   node part and a leaf part will be called an "EB node".

   We also notice on the diagram that there is a root entity required to attach
   the tree. It only contains two branches and there is nothing above it. This
   is an "EB root". Some will note that [leaf1] has no [node1]. One property of
   the EBtree is that all nodes have their branches filled, and that if a node
   has only one branch, it does not need to exist. Here, [leaf1] was added
   below [root] and did not need any node.

   An EB node contains :
     - a pointer to the node's parent (node_p)
     - a pointer to the leaf's parent (leaf_p)
     - two branches pointing to lower nodes or leaves (branches)
     - a bit position (bit)
     - an optional key.

   The key here is optional because it's used only during insertion, in order
   to classify the nodes. Nothing else in the tree structure requires knowledge
   of the key. This makes it possible to write type-agnostic primitives for
   everything, and type-specific insertion primitives. This has led to consider
   two types of EB nodes. The type-agnostic ones will serve as a header for the
   other ones, and will simply be called "struct ebx_node". The other ones will
   have their type indicated in the structure name. Eg: "struct ebx32_node" for
   nodes carrying 32 bit keys.

   We will also node that the two branches in a node serve exactly the same
   purpose as an EB root. For this reason, a "struct ebx_root" will be used as
   well inside the struct ebx_node. In order to ease pointer manipulation and
   ROOT detection when walking upwards, all the pointers inside an ebx_node will
   point to the ebx_root part of the referenced EB nodes, relying on the same
   principle as the linked lists in Linux.

   Another important point to note, is that when walking inside a tree, it is
   very convenient to know where a node is attached in its parent, and what
   type of branch it has below it (leaf or node). In order to simplify the
   operations and to speed up the processing, it was decided in this specific
   implementation to use the lowest bit from the pointer to designate the side
   of the upper pointers (left/right) and the type of a branch (leaf/node).
   This practise is not mandatory by design, but an implementation-specific
   optimisation permitted on all platforms on which data must be aligned. All
   known 32 bit platforms align their integers and pointers to 32 bits, leaving
   the two lower bits unused. So, we say that the pointers are "tagged". And
   since they designate pointers to root parts, we simply call them
   "tagged root pointers", or "ebx_troot_t" in the code.

   Duplicate keys are stored in a special manner. When inserting a key, if
   the same one is found, then an incremental binary tree is built at this
   place from these keys. This ensures that no special case has to be written
   to handle duplicates when walking through the tree or when deleting entries.
   It also guarantees that duplicates will be walked in the exact same order
   they were inserted. This is very important when trying to achieve fair
   processing distribution for instance.

   Algorithmic complexity can be derived from 3 variables :
     - the number of possible different keys in the tree : P
     - the number of entries in the tree : N
     - the number of duplicates for one key : D

   Note that this tree is deliberately NOT balanced. For this reason, the worst
   case may happen with a small tree (eg: 32 distinct keys of one bit). BUT,
   the operations required to manage such data are so much cheap that they make
   it worth using it even under such conditions. For instance, a balanced tree
   may require only 6 levels to store those 32 keys when this tree will
   require 32. But if per-level operations are 5 times cheaper, it wins.

   Minimal, Maximal and Average times are specified in number of operations.
   Minimal is given for best condition, Maximal for worst condition, and the
   average is reported for a tree containing random keys. An operation
   generally consists in jumping from one node to the other.

   Complexity :
     - lookup              : min=1, max=log(P), avg=log(N)
     - insertion from root : min=1, max=log(P), avg=log(N)
     - insertion of dups   : min=1, max=log(D), avg=log(D)/2 after lookup
     - deletion            : min=1, max=1,      avg=1
     - prev/next           : min=1, max=log(P), avg=2 :
       N/2 nodes need 1 hop  => 1*N/2
       N/4 nodes need 2 hops => 2*N/4
       N/8 nodes need 3 hops => 3*N/8
       ...
       N/x nodes need log(x) hops => log2(x)*N/x
       Total cost for all N nodes : sum[i=1..N](log2(i)*N/i) = N*sum[i=1..N](log2(i)/i)
       Average cost across N nodes = total / N = sum[i=1..N](log2(i)/i) = 2

   This design is currently limited to only two branches per node. Most of the
   tree descent algorithm would be compatible with more branches (eg: 4, to cut
   the height in half), but this would probably require more complex operations
   and the deletion algorithm would be problematic.

   Useful properties :
     - a node is always added above the leaf it is tied to, and never can get
       below nor in another branch. This implies that leaves directly attached
       to the root do not use their node part, which is indicated by a NULL
       value in node_p. This also enhances the cache efficiency when walking
       down the tree, because when the leaf is reached, its node part will
       already have been visited (unless it's the first leaf in the tree).

     - pointers to lower nodes or leaves are stored in "branch" pointers. Only
       the root node may have a NULL in either branch, it is not possible for
       other branches. Since the nodes are attached to the left branch of the
       root, it is not possible to see a NULL left branch when walking up a
       tree. Thus, an empty tree is immediately identified by a NULL left
       branch at the root. Conversely, the one and only way to identify the
       root node is to check that it right branch is NULL. Note that the
       NULL pointer may have a few low-order bits set.

     - a node connected to its own leaf will have branch[0|1] pointing to
       itself, and leaf_p pointing to itself.

     - a node can never have node_p pointing to itself.

     - a node is linked in a tree if and only if it has a non-null leaf_p.

     - a node can never have both branches equal, except for the root which can
       have them both NULL.

     - deletion only applies to leaves. When a leaf is deleted, its parent must
       be released too (unless it's the root), and its sibling must attach to
       the grand-parent, replacing the parent. Also, when a leaf is deleted,
       the node tied to this leaf will be removed and must be released too. If
       this node is different from the leaf's parent, the freshly released
       leaf's parent will be used to replace the node which must go. A released
       node will never be used anymore, so there's no point in tracking it.

     - the bit index in a node indicates the bit position in the key which is
       represented by the branches. That means that a node with (bit == 0) is
       just above two leaves. Negative bit values are used to build a duplicate
       tree. The first node above two identical leaves gets (bit == -1). This
       value logarithmically decreases as the duplicate tree grows. During
       duplicate insertion, a node is inserted above the highest bit value (the
       lowest absolute value) in the tree during the right-sided walk. If bit
       -1 is not encountered (highest < -1), we insert above last leaf.
       Otherwise, we insert above the node with the highest value which was not
       equal to the one of its parent + 1.

     - the "ebx_next" primitive walks from left to right, which means from lower
       to higher keys. It returns duplicates in the order they were inserted.
       The "ebx_first" primitive returns the left-most entry.

     - the "ebx_prev" primitive walks from right to left, which means from
       higher to lower keys. It returns duplicates in the opposite order they
       were inserted. The "ebx_last" primitive returns the right-most entry.

     - a tree which has 1 in the lower bit of its root's right branch is a
       tree with unique nodes. This means that when a node is inserted with
       a key which already exists will not be inserted, and the previous
       entry will be returned.

 */

#ifndef _EBXTREE_H
#define _EBXTREE_H

#include <stdlib.h>
#include "compiler.h"

static inline int flsnz8_generic(unsigned int x)
{
	int ret = 0;
	if (x >> 4) { x >>= 4; ret += 4; }
	return ret + ((0xFFFFAA50U >> (x << 1)) & 3) + 1;
}

/* Note: we never need to run fls on null keys, so we can optimize the fls
 * function by removing a conditional jump.
 */
#if defined(__i386__) || defined(__x86_64__)
/* this code is similar on 32 and 64 bit */
static inline int flsnz(int x)
{
	int r;
	__asm__("bsrl %1,%0\n"
	        : "=r" (r) : "rm" (x));
	return r+1;
}

static inline int flsnz8(unsigned char x)
{
	int r;
	__asm__("movzbl %%al, %%eax\n"
		"bsrl %%eax,%0\n"
	        : "=r" (r) : "a" (x));
	return r+1;
}

#else
/* returns 1 to 32 for 1<<0 to 1<<31. Undefined for 0. */
#define flsnz(___a) ({ \
	register int ___x, ___bits = 0; \
	___x = (___a); \
	if (___x & 0xffff0000) { ___x &= 0xffff0000; ___bits += 16;} \
	if (___x & 0xff00ff00) { ___x &= 0xff00ff00; ___bits +=  8;} \
	if (___x & 0xf0f0f0f0) { ___x &= 0xf0f0f0f0; ___bits +=  4;} \
	if (___x & 0xcccccccc) { ___x &= 0xcccccccc; ___bits +=  2;} \
	if (___x & 0xaaaaaaaa) { ___x &= 0xaaaaaaaa; ___bits +=  1;} \
	___bits + 1; \
	})

static inline int flsnz8(unsigned int x)
{
	return flsnz8_generic(x);
}


#endif

static inline int fls64(unsigned long long x)
{
	unsigned int h;
	unsigned int bits = 32;

	h = x >> 32;
	if (!h) {
		h = x;
		bits = 0;
	}
	return flsnz(h) + bits;
}

#define fls_auto(x) ((sizeof(x) > 4) ? fls64(x) : flsnz(x))

/* Linux-like "container_of". It returns a pointer to the structure of type
 * <type> which has its member <name> stored at address <ptr>.
 */
#ifndef container_of
#define container_of(ptr, type, name) ((type *)(((char *)(ptr)) - ((long)&((type *)0)->name)))
#endif

/* returns a pointer to the structure of type <type> which has its member <name>
 * stored at address <ptr>, unless <ptr> is 0, in which case 0 is returned.
 */
#ifndef container_of_safe
#define container_of_safe(ptr, type, name) \
	({ void *__p = (ptr); \
		__p ? (type *)(__p - ((long)&((type *)0)->name)) : (type *)0; \
	})
#endif

/* Number of bits per node, and number of leaves per node */
#define EB_NODE_BITS          1
#define EB_NODE_BRANCHES      (1 << EB_NODE_BITS)
#define EB_NODE_BRANCH_MASK   (EB_NODE_BRANCHES - 1)

/* Be careful not to tweak those values. The walking code is optimized for NULL
 * detection on the assumption that the following values are intact.
 */
#define EB_LEFT     0
#define EB_RGHT     1
#define EB_LEAF     0
#define EB_NODE     1

/* Tags to set in root->b[EB_RGHT] :
 * - EB_NORMAL is a normal tree which stores duplicate keys.
 * - EB_UNIQUE is a tree which stores unique keys.
 */
#define EB_NORMAL   0
#define EB_UNIQUE   1

/* This is the same as an ebx_node pointer, except that the lower bit embeds
 * a tag. See ebx_dotag()/ebx_untag()/ebx_gettag(). This tag has two meanings :
 *  - 0=left, 1=right to designate the parent's branch for leaf_p/node_p
 *  - 0=link, 1=leaf  to designate the branch's type for branch[]
 */
typedef void *ebx_link_t;
typedef ebx_link_t *ebx_troot_t;

/* The ebx_root connects the node which contains it, to two nodes below it, one
 * of which may be the same node. At the top of the tree, we use an ebx_root
 * too, which always has its right branch NULL (+/1 low-order bits).
 */
struct ebx_root {
	ebx_link_t       b[EB_NODE_BRANCHES]; /* left and right branches */
};

/* The ebx_node contains the two parts, one for the leaf, which always exists,
 * and one for the node, which remains unused in the very first node inserted
 * into the tree. This structure is 20 bytes per node on 32-bit machines. Do
 * not change the order, benchmarks have shown that it's optimal this way.
 */
struct ebx_node {
	struct ebx_root branches; /* branches, must be at the beginning */
	ebx_link_t       node_p;  /* link node's parent */
	ebx_link_t       leaf_p;  /* leaf node's parent */
	short int       bit;     /* link's bit position. */
	short unsigned int pfx; /* data prefix length, always related to leaf */
} __attribute__((packed));

/* Return the structure of type <type> whose member <member> points to <ptr> */
#define ebx_entry(ptr, type, member) container_of(ptr, type, member)

/* The root of a tree is an ebx_root initialized with both pointers NULL.
 * During its life, only the left pointer will change. The right one will
 * always remain NULL, which is the way we detect it.
 */
#define EB_ROOT						\
	(struct ebx_root) {				\
		.b = {[0] = 0, [1] = 0 },		\
	}

#define EB_ROOT_UNIQUE					\
	(struct ebx_root) {				\
		.b = {[0] = 0, [1] = (ebx_link_t)1 },	\
	}

#define EB_TREE_HEAD(name)				\
	struct ebx_root name = EB_ROOT


/***************************************\
 * Private functions. Not for end-user *
\***************************************/

/* Converts a root pointer to its equivalent ebx_troot_t pointer,
 * ready to be stored in ->branch[], leaf_p or node_p. NULL is not
 * conserved. To be used with EB_LEAF, EB_NODE, EB_LEFT or EB_RGHT in <tag>.
 */
static inline ebx_troot_t *ebx_dotag(const struct ebx_root *root, const int tag)
{
	return (ebx_troot_t *)((char *)root + tag);
}

/* Converts an ebx_troot_t pointer pointer to its equivalent ebx_root pointer,
 * for use with pointers from ->branch[], leaf_p or node_p. NULL is conserved
 * as long as the tree is not corrupted. To be used with EB_LEAF, EB_NODE,
 * EB_LEFT or EB_RGHT in <tag>.
 */
static inline struct ebx_root *ebx_untag(const ebx_troot_t *troot, const int tag)
{
	return (struct ebx_root *)((char *)troot - tag);
}

/* returns the tag associated with an ebx_troot_t pointer */
static inline int ebx_gettag(ebx_troot_t *troot)
{
	return (unsigned long)troot & 1;
}

/* Converts a root pointer to its equivalent ebx_troot_t pointer and clears the
 * tag, no matter what its value was.
 */
static inline struct ebx_root *ebx_clrtag(const ebx_troot_t *troot)
{
	return (struct ebx_root *)((unsigned long)troot & ~1UL);
}

/* Returns a pointer to the ebx_node holding <root>, where <root> is stored at <base> */
static inline struct ebx_node *ebx_root_to_node(struct ebx_root *root)
{
	return container_of(root, struct ebx_node, branches);
}

/* Assigns a pointer to a link */
#define ebx_setlink(dest, troot) do { *(dest) = (troot); } while (0)

/* Returns the pointer from a link */
#define ebx_getroot(a) (*(a))

/* Walks down starting at root pointer <start>, and always walking on side
 * <side>. It either returns the node hosting the first leaf on that side,
 * or NULL if no leaf is found. <start> may either be NULL or a branch pointer.
 * The pointer to the leaf (or NULL) is returned.
 */
static inline struct ebx_node *ebx_walk_down(ebx_troot_t *start, unsigned int side)
{
	/* A NULL pointer on an empty tree root will be returned as-is */
	while (ebx_gettag(start) == EB_NODE)
		start = ebx_getroot(&(ebx_untag(start, EB_NODE))->b[side]);
	/* NULL is left untouched (root==ebx_node, EB_LEAF==0) */
	return ebx_root_to_node(ebx_untag(start, EB_LEAF));
}

/* This function is used to build a tree of duplicates by adding a new node to
 * a subtree of at least 2 entries. It will probably never be needed inlined,
 * and it is not for end-user.
 */
static forceinline struct ebx_node *
__ebx_insert_dup(struct ebx_node *sub, struct ebx_node *new)
{
	struct ebx_node *head = sub;
	
	ebx_troot_t *new_left = ebx_dotag(&new->branches, EB_LEFT);
	ebx_troot_t *new_rght = ebx_dotag(&new->branches, EB_RGHT);
	ebx_troot_t *new_leaf = ebx_dotag(&new->branches, EB_LEAF);

	/* first, identify the deepest hole on the right branch */
	while (ebx_gettag(ebx_getroot(&head->branches.b[EB_RGHT])) != EB_LEAF) {
		struct ebx_node *last = head;
		head = container_of(ebx_untag(ebx_getroot(&head->branches.b[EB_RGHT]), EB_NODE),
				    struct ebx_node, branches);
		if (head->bit > last->bit + 1)
			sub = head;     /* there's a hole here */
	}

	/* Here we have a leaf attached to (head)->b[EB_RGHT] */
	if (head->bit < -1) {
		/* A hole exists just before the leaf, we insert there */
		new->bit = -1;
		sub = container_of(ebx_untag(ebx_getroot(&head->branches.b[EB_RGHT]), EB_LEAF),
				   struct ebx_node, branches);
		ebx_setlink(&head->branches.b[EB_RGHT], ebx_dotag(&new->branches, EB_NODE));

		ebx_setlink(&new->node_p, ebx_getroot(&sub->leaf_p));
		ebx_setlink(&new->leaf_p, new_rght);
		ebx_setlink(&sub->leaf_p, new_left);
		ebx_setlink(&new->branches.b[EB_LEFT], ebx_dotag(&sub->branches, EB_LEAF));
		ebx_setlink(&new->branches.b[EB_RGHT], new_leaf);
		return new;
	} else {
		int side;
		/* No hole was found before a leaf. We have to insert above
		 * <sub>. Note that we cannot be certain that <sub> is attached
		 * to the right of its parent, as this is only true if <sub>
		 * is inside the dup tree, not at the head.
		 */
		new->bit = sub->bit - 1; /* install at the lowest level */
		side = ebx_gettag(ebx_getroot(&sub->node_p));
		head = container_of(ebx_untag(ebx_getroot(&sub->node_p), side), struct ebx_node, branches);
		ebx_setlink(&head->branches.b[side], ebx_dotag(&new->branches, EB_NODE));
					
		ebx_setlink(&new->node_p, ebx_getroot(&sub->node_p));
		ebx_setlink(&new->leaf_p, new_rght);
		ebx_setlink(&sub->node_p, new_left);
		ebx_setlink(&new->branches.b[EB_LEFT], ebx_dotag(&sub->branches, EB_NODE));
		ebx_setlink(&new->branches.b[EB_RGHT], new_leaf);
		return new;
	}
}


/**************************************\
 * Public functions, for the end-user *
\**************************************/

/* Return non-zero if the tree is empty, otherwise zero */
static inline int ebx_is_empty(struct ebx_root *root)
{
	return !root->b[EB_LEFT];
}

/* Return non-zero if the node is a duplicate, otherwise zero */
static inline int ebx_is_dup(struct ebx_node *node)
{
	return node->bit < 0;
}

/* Return the first leaf in the tree starting at <root>, or NULL if none */
static inline struct ebx_node *ebx_first(struct ebx_root *root)
{
	return ebx_walk_down(ebx_getroot(&root->b[0]), EB_LEFT);
}

/* Return the last leaf in the tree starting at <root>, or NULL if none */
static inline struct ebx_node *ebx_last(struct ebx_root *root)
{
	return ebx_walk_down(ebx_getroot(&root->b[0]), EB_RGHT);
}

/* Return previous leaf node before an existing leaf node, or NULL if none. */
static inline struct ebx_node *ebx_prev(struct ebx_node *node)
{
	ebx_troot_t *t = ebx_getroot(&node->leaf_p);

	while (ebx_gettag(t) == EB_LEFT) {
		/* Walking up from left branch. We must ensure that we never
		 * walk beyond root.
		 */
		if (unlikely(ebx_clrtag(ebx_getroot(&(ebx_untag(t, EB_LEFT))->b[EB_RGHT])) == NULL))
			return NULL;
		t = ebx_getroot(&(ebx_root_to_node(ebx_untag(t, EB_LEFT)))->node_p);
	}
	/* Note that <t> cannot be NULL at this stage */
	t = ebx_getroot(&(ebx_untag(t, EB_RGHT))->b[EB_LEFT]);
	return ebx_walk_down(t, EB_RGHT);
}

/* Return next leaf node after an existing leaf node, or NULL if none. */
static inline struct ebx_node *ebx_next(struct ebx_node *node)
{
	ebx_troot_t *t = ebx_getroot(&node->leaf_p);

	while (ebx_gettag(t) != EB_LEFT)
		/* Walking up from right branch, so we cannot be below root */
		t = ebx_getroot(&(ebx_root_to_node(ebx_untag(t, EB_RGHT)))->node_p);

	/* Note that <t> cannot be NULL at this stage */
	t = ebx_getroot(&(ebx_untag(t, EB_LEFT))->b[EB_RGHT]);
	if (ebx_clrtag(t) == NULL)
		return NULL;
	return ebx_walk_down(t, EB_LEFT);
}

/* Return previous leaf node within a duplicate sub-tree, or NULL if none. */
static inline struct ebx_node *ebx_prev_dup(struct ebx_node *node)
{
	ebx_troot_t *t = ebx_getroot(&node->leaf_p);

	while (ebx_gettag(t) == EB_LEFT) {
		/* Walking up from left branch. We must ensure that we never
		 * walk beyond root.
		 */
		if (unlikely(ebx_clrtag(ebx_getroot(&(ebx_untag(t, EB_LEFT))->b[EB_RGHT])) == NULL))
			return NULL;
		/* if the current node leaves a dup tree, quit */
		if ((ebx_root_to_node(ebx_untag(t, EB_LEFT)))->bit >= 0)
			return NULL;
		t = ebx_getroot(&(ebx_root_to_node(ebx_untag(t, EB_LEFT)))->node_p);
	}
	/* Note that <t> cannot be NULL at this stage */
	if ((ebx_root_to_node(ebx_untag(t, EB_RGHT)))->bit >= 0)
		return NULL;
	t = ebx_getroot(&(ebx_untag(t, EB_RGHT))->b[EB_LEFT]);
	return ebx_walk_down(t, EB_RGHT);
}

/* Return next leaf node within a duplicate sub-tree, or NULL if none. */
static inline struct ebx_node *ebx_next_dup(struct ebx_node *node)
{
	ebx_troot_t *t = ebx_getroot(&node->leaf_p);

	while (ebx_gettag(t) != EB_LEFT) {
		/* Walking up from right branch, so we cannot be below root */
		/* if the current node leaves a dup tree, quit */
		if ((ebx_root_to_node(ebx_untag(t, EB_RGHT)))->bit >= 0)
			return NULL;
		t = ebx_getroot(&(ebx_root_to_node(ebx_untag(t, EB_RGHT)))->node_p);
	}

	/* Note that <t> cannot be NULL at this stage */
	if ((ebx_root_to_node(ebx_untag(t, EB_LEFT)))->bit >= 0)
		return NULL;
	t = ebx_getroot(&(ebx_untag(t, EB_LEFT))->b[EB_RGHT]);
	if (ebx_clrtag(t) == NULL)
		return NULL;
	return ebx_walk_down(t, EB_LEFT);
}

/* Return previous leaf node before an existing leaf node, skipping duplicates,
 * or NULL if none. */
static inline struct ebx_node *ebx_prev_unique(struct ebx_node *node)
{
	ebx_troot_t *t = ebx_getroot(&node->leaf_p);

	while (1) {
		if (ebx_gettag(t) != EB_LEFT) {
			node = ebx_root_to_node(ebx_untag(t, EB_RGHT));
			/* if we're right and not in duplicates, stop here */
			if (node->bit >= 0)
				break;
			t = ebx_getroot(&node->node_p);
		}
		else {
			/* Walking up from left branch. We must ensure that we never
			 * walk beyond root.
			 */
			if (unlikely(ebx_clrtag(ebx_getroot(&(ebx_untag(t, EB_LEFT))->b[EB_RGHT])) == NULL))
				return NULL;
			t = ebx_getroot(&(ebx_root_to_node(ebx_untag(t, EB_LEFT)))->node_p);
		}
	}
	/* Note that <t> cannot be NULL at this stage */
	t = ebx_getroot(&(ebx_untag(t, EB_RGHT))->b[EB_LEFT]);
	return ebx_walk_down(t, EB_RGHT);
}

/* Return next leaf node after an existing leaf node, skipping duplicates, or
 * NULL if none.
 */
static inline struct ebx_node *ebx_next_unique(struct ebx_node *node)
{
	ebx_troot_t *t = ebx_getroot(&node->leaf_p);

	while (1) {
		if (ebx_gettag(t) == EB_LEFT) {
			if (unlikely(ebx_clrtag(ebx_getroot(&(ebx_untag(t, EB_LEFT))->b[EB_RGHT])) == NULL))
				return NULL;	/* we reached root */
			node = ebx_root_to_node(ebx_untag(t, EB_LEFT));
			/* if we're left and not in duplicates, stop here */
			if (node->bit >= 0)
				break;
			t = ebx_getroot(&node->node_p);
		}
		else {
			/* Walking up from right branch, so we cannot be below root */
			t = ebx_getroot(&(ebx_root_to_node(ebx_untag(t, EB_RGHT)))->node_p);
		}
	}

	/* Note that <t> cannot be NULL at this stage */
	t = ebx_getroot(&(ebx_untag(t, EB_LEFT))->b[EB_RGHT]);
	if (ebx_clrtag(t) == NULL)
		return NULL;
	return ebx_walk_down(t, EB_LEFT);
}


/* Removes a leaf node from the tree if it was still in it. Marks the node
 * as unlinked.
 */
static forceinline void __ebx_delete(struct ebx_node *node)
{
	unsigned int pside, gpside, sibtype;
	struct ebx_node *parent;
	struct ebx_root *gparent;

	if (!node->leaf_p)
		return;

	/* we need the parent, our side, and the grand parent */
	pside = ebx_gettag(ebx_getroot(&node->leaf_p));
	parent = ebx_root_to_node(ebx_untag(ebx_getroot(&node->leaf_p), pside));

	/* We likely have to release the parent link, unless it's the root,
	 * in which case we only set our branch to NULL. Note that we can
	 * only be attached to the root by its left branch.
	 */

	if (ebx_clrtag(ebx_getroot(&parent->branches.b[EB_RGHT])) == NULL) {
		/* we're just below the root, it's trivial. */
		parent->branches.b[EB_LEFT] = 0;
		goto delete_unlink;
	}

	/* To release our parent, we have to identify our sibling, and reparent
	 * it directly to/from the grand parent. Note that the sibling can
	 * either be a link or a leaf.
	 */

	gpside = ebx_gettag(ebx_getroot(&parent->node_p));
	gparent = ebx_untag(ebx_getroot(&parent->node_p), gpside);

	ebx_setlink(&gparent->b[gpside], ebx_getroot(&parent->branches.b[!pside]));
	sibtype = ebx_gettag(ebx_getroot(&gparent->b[gpside]));

	if (sibtype == EB_LEAF) {
		ebx_setlink(&ebx_root_to_node(ebx_untag(ebx_getroot(&gparent->b[gpside]), EB_LEAF))->leaf_p, ebx_dotag(gparent, gpside));
	} else {
		ebx_setlink(&ebx_root_to_node(ebx_untag(ebx_getroot(&gparent->b[gpside]), EB_NODE))->node_p, ebx_dotag(gparent, gpside));
	}
	/* Mark the parent unused. Note that we do not check if the parent is
	 * our own node, but that's not a problem because if it is, it will be
	 * marked unused at the same time, which we'll use below to know we can
	 * safely remove it.
	 */
	parent->node_p = 0;

	/* The parent node has been detached, and is currently unused. It may
	 * belong to another node, so we cannot remove it that way. Also, our
	 * own node part might still be used. so we can use this spare node
	 * to replace ours if needed.
	 */

	/* If our link part is unused, we can safely exit now */
	if (!node->node_p)
		goto delete_unlink;

	/* From now on, <node> and <parent> are necessarily different, and the
	 * <node>'s node part is in use. By definition, <parent> is at least
	 * below <node>, so keeping its key for the bit string is OK.
	 */

	ebx_setlink(&parent->node_p, ebx_getroot(&node->node_p));
	ebx_setlink(&parent->branches.b[EB_LEFT], ebx_getroot(&node->branches.b[EB_LEFT]));
	ebx_setlink(&parent->branches.b[EB_RGHT], ebx_getroot(&node->branches.b[EB_RGHT]));
	parent->bit = node->bit;

	/* We must now update the new node's parent... */
	gpside = ebx_gettag(ebx_getroot(&parent->node_p));
	gparent = ebx_untag(ebx_getroot(&parent->node_p), gpside);
	ebx_setlink(&gparent->b[gpside], ebx_dotag(&parent->branches, EB_NODE));

	/* ... and its branches */
	for (pside = 0; pside <= 1; pside++) {
		if (ebx_gettag(ebx_getroot(&parent->branches.b[pside])) == EB_NODE) {
			ebx_setlink(&ebx_root_to_node(ebx_untag(ebx_getroot(&parent->branches.b[pside]), EB_NODE))->node_p,
				ebx_dotag(&parent->branches, pside));
		} else {
			ebx_setlink(&ebx_root_to_node(ebx_untag(ebx_getroot(&parent->branches.b[pside]), EB_LEAF))->leaf_p,
				ebx_dotag(&parent->branches, pside));
		}
	}
 delete_unlink:
	/* Now the node has been completely unlinked */
	node->leaf_p = 0;
	return; /* tree is not empty yet */
}

/* Compare blocks <a> and <b> byte-to-byte, from bit <ignore> to bit <len-1>.
 * Return the number of equal bits between strings, assuming that the first
 * <ignore> bits are already identical. It is possible to return slightly more
 * than <len> bits if <len> does not stop on a byte boundary and we find exact
 * bytes. Note that parts or all of <ignore> bits may be rechecked. It is only
 * passed here as a hint to speed up the check.
 */
static forceinline int equal_bits(const unsigned char *a,
				  const unsigned char *b,
				  int ignore, int len)
{
	for (ignore >>= 3, a += ignore, b += ignore, ignore <<= 3;
	     ignore < len; ) {
		unsigned char c;

		a++; b++;
		ignore += 8;
		c = b[-1] ^ a[-1];

		if (c) {
			/* OK now we know that old and new differ at byte <ptr> and that <c> holds
			 * the bit differences. We have to find what bit is differing and report
			 * it as the number of identical bits. Note that low bit numbers are
			 * assigned to high positions in the byte, as we compare them as strings.
			 */
			ignore -= flsnz8(c);
			break;
		}
	}
	return ignore;
}

/* check that the two blocks <a> and <b> are equal on <len> bits. If it is known
 * they already are on some bytes, this number of equal bytes to be skipped may
 * be passed in <skip>. It returns 0 if they match, otherwise non-zero.
 */
static forceinline int check_bits(const unsigned char *a,
				  const unsigned char *b,
				  int skip,
				  int len)
{
	int bit, ret;

	/* This uncommon construction gives the best performance on x86 because
	 * it makes heavy use multiple-index addressing and parallel instructions,
	 * and it prevents gcc from reordering the loop since it is already
	 * properly oriented. Tested to be fine with 2.95 to 4.2.
	 */
	bit = ~len + (skip << 3) + 9; /* = (skip << 3) + (8 - len) */
	ret = a[skip] ^ b[skip];
	if (unlikely(bit >= 0))
		return ret >> bit;
	while (1) {
		skip++;
		if (ret)
			return ret;
		ret = a[skip] ^ b[skip];
		bit += 8;
		if (bit >= 0)
			return ret >> bit;
	}
}


/* Compare strings <a> and <b> byte-to-byte, from bit <ignore> to the last 0.
 * Return the number of equal bits between strings, assuming that the first
 * <ignore> bits are already identical. Note that parts or all of <ignore> bits
 * may be rechecked. It is only passed here as a hint to speed up the check.
 * The caller is responsible for not passing an <ignore> value larger than any
 * of the two strings. However, referencing any bit from the trailing zero is
 * permitted. Equal strings are reported as a negative number of bits, which
 * indicates the end was reached.
 */
static forceinline int string_equal_bits(const unsigned char *a,
					 const unsigned char *b,
					 int ignore)
{
	int beg;
	unsigned char c;

	beg = ignore >> 3;

	/* skip known and identical bits. We stop at the first different byte
	 * or at the first zero we encounter on either side.
	 */
	while (1) {
		unsigned char d;

		c = a[beg];
		d = b[beg];
		beg++;

		c ^= d;
		if (c)
			break;
		if (!d)
			return -1;
	}
	/* OK now we know that a and b differ at byte <beg>, or that both are zero.
	 * We have to find what bit is differing and report it as the number of
	 * identical bits. Note that low bit numbers are assigned to high positions
	 * in the byte, as we compare them as strings.
	 */
	return (beg << 3) - flsnz8(c);
}

static forceinline int cmp_bits(const unsigned char *a, const unsigned char *b, unsigned int pos)
{
	unsigned int ofs;
	unsigned char bit_a, bit_b;

	ofs = pos >> 3;
	pos = ~pos & 7;

	bit_a = (a[ofs] >> pos) & 1;
	bit_b = (b[ofs] >> pos) & 1;

	return bit_a - bit_b; /* -1: a<b; 0: a=b; 1: a>b */
}

static forceinline int get_bit(const unsigned char *a, unsigned int pos)
{
	unsigned int ofs;

	ofs = pos >> 3;
	pos = ~pos & 7;
	return (a[ofs] >> pos) & 1;
}

/* These functions are declared in ebxtree.c */
void ebx_delete(struct ebx_node *node);
REGPRM1 struct ebx_node *ebx_insert_dup(struct ebx_node *sub, struct ebx_node *new);

#endif /* _EBX_TREE_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 */