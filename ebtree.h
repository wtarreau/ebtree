/*
 * Elastic Binary Trees - macros and structures.
 * (C) 2002-2007 - Willy Tarreau <w@1wt.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Short history :
 *
 * 2007/07/08: merge back cleanups from kernel version.
 * 2007/07/01: merge into Linux Kernel (try 1).
 * 2007/05/27: version 2: compact everything into one single struct
 * 2007/05/18: adapted the structure to support embedded nodes
 * 2007/05/13: adapted to mempools v2.
 */



/*
  General idea:
  In a radix binary tree, we may have up to 2N-1 nodes for N values if all of
  them are leaves. If we find a way to differentiate intermediate nodes (called
  "link nodes") and final nodes (called "leaf nodes"), and we associate them
  by two, it is possible to build sort of a self-contained radix tree with
  intermediate nodes always present. It will not be as cheap as the ultree for
  optimal cases as shown below, but the optimal case almost never happens :

  Eg, to store 8, 10, 12, 13, 14 :

      ultree       this tree

        8              8
       / \            / \
      10 12          10 12
        /  \           /  \
       13  14         12  14
                     / \
                    12 13

   Note that on real-world tests (with a scheduler), is was verified that the
   case with data on an intermediate node never happens. This is because the
   population is too large for such coincidences to happen. It would require
   for instance that a task has its expiration time at an exact second, with
   other tasks sharing that second. This is too rare to try to optimize for it.

   What is interesting is that the link will only be added above the leaf when
   necessary, which implies that it will always remain somewhere above it. So
   both the leaf and the link can share the exact value of the node, because
   when going down the link, the bit mask will be applied to comparisons. So we
   are tempted to have one value for both nodes.

   The bit only serves the links, and the dups only serve the leaves. So we can
   put a lot of information in common. This results in one single node with two
   leaves and two parents, one for the link part, and one for the leaf part.
   The link may refer to its leaf counterpart in one of its leaves, which will
   be a solution to quickly distinguish between different nodes and common
   nodes.

   Here's what we find in an eb_node :

   struct eb_node {
       struct list    dup;      // leaf duplicates
       struct eb_node *node_p;  // link node's parent
       struct eb_node *leaf_p;  // leaf node's parent
       struct eb_node *leaf[2]; // link's leaf nodes
       int            bit;      // link's bit position. Maybe we should use a char ?
   };

   struct eb32_node {
       struct eb_node node;
       u32 val;
   };

   struct eb64_node {
       struct eb_node node;
       u64 val;
   };


   Algorithmic complexity (max and avg computed for a tree full of distinct values) :
     - lookup              : avg=O(logN), max = O(logN)
     - insertion from root : avg=O(logN), max = O(logN)
     - insertion of dups   : O(1) after lookup
     - moves               : not implemented yet, O(logN)
     - deletion            : max = O(1)
     - prev/next           : avg = 2, max = O(logN)
       N/2 nodes need 1 hop  => 1*N/2
       N/4 nodes need 2 hops => 2*N/4
       N/8 nodes need 3 hops => 3*N/8
       ...
       N/x nodes need log(x) hops => log2(x)*N/x
       Total cost for all N nodes : sum[i=1..N](log2(i)*N/i) = N*sum[i=1..N](log2(i)/i)
       Average cost across N nodes = total / N = sum[i=1..N](log2(i)/i) = 2

   Useful properties (outdated) :
     - links are only provided above the leaf, never below. This implies that
       the nodes directly attached to the root do not use their link. It also
       enhances the probability that the link directly above a leaf are from
       the same node.

     - a link connected to its own leaf will have leaf_p = node = leaf[0|1].

     - leaf[0] can never be equal to leaf[1] except for the root which can have
       them both NULL.

     - node_p can never be equal to the same node.

     - two leaves can never point to the same location

     - duplicates do not use their link part, nor their leaf_p pointer.

     - links do not use their dup list.

     - those cannot be merged because a leaf at the head of a dup list needs
       both a link above it and a dup list.

     - a leaf-only node should have some easily distinguishable info in the
       link part, such as NULL or a pointer to the same node (which cannot
       happen in normal case). The NULL might be better to identify the root.

     - bit is necessarily > 0.

   New properties (2007/08/04) :
     - pointers to lower nodes are stored in "branch" pointers (currently 2,
       but may be extended to 4).
     - pointers to higher nodes are stored in "parent" pointers, one being used
       by the leaf part of the eb_node, the other one by the node part.
     - root <=> (branch[right]==NULL)
     - (root || dup) <=> (parent[L]==parent[N]) && (parent[*]==NULL)
     
   Basic definitions (subject to change) :
     - for duplicate leaf nodes, leaf_p = NULL.
     - use bit == 0 to indicate a leaf node which is not used as a link
     - root->bit = INTBITS for the represented data type (eg: 32)
     - root->node_p = root->leaf_p = NULL
     - root->leaf[0,1] = NULL if branch is empty

   Deletion is not very complex:
     - it only applies to leaves
     - if the leaf is a duplicate, simply remove it from the list.
     - when a leaf is deleted, its parent must be unlinked (unless it is the root)
     - when a leaf is deleted, the link provided with the same node must be
       replaced if used, because it will not be available anymore. We put
       the one we freed instead.

   It is important to understand that once a link node is removed, it will
   never be needed anymore. If another node comes above and needs a link, it
   will provide its own.

   Also, when we delete a leaf attached to the root, we get no link back. It's
   not a problem because by definition, since a node can only provide links
   above it, it has no link in use.

   New properties (2007/09/26)
   ---------------------------
     - nodes are of distinct types :
       - structure nodes, whose type is "eb_node"
       - data nodes, whose type is eb32_node/eb64_node, etc... They contain
         an eb_node and a value.
     - an eb_node has two parts :
       - the "node" part, which is the one constituting the tree structure
       - the "leaf" part, which is the one designating the position of the
         value in the tree.
     - pointers to lower nodes are stored in "branch" pointers of a node
       (currently 2, but may be extended to 4).
     - pointer to the node's upper node is stored in the node_p pointer
     - pointer to the leaf's upper node is stored in the leaf_p pointer
     - root <=> (branch[right] == NULL)
     - only root may have a NULL in a branch
     - an eb_node with the node part unused has node_p == NULL
     - the bit value designates the bit position for which branch[] applies.
       By definition, if it is negative, it is because we are on a duplicates tree.
     - ALL nodes in duplicate trees have their bit value < 0. During
       duplicate insertion, a node is inserted above the highest bit value
       (the lowest absolute value) in the tree during the right walk. If bit -1
       is not encountered (highest < -1), we insert above last leaf. Otherwise,
       we insert above the node with highest value which was not equal to the
       one of its parent + 1.

     - root does not need leaf_p, node_p, bit. So it only needs branches[]. A
       new "struct eb_root" type has been created for this.

 */


#include <stdlib.h>

/* Note: we never need to run fls on null values, so we can optimize the fls
 * function by removing a conditional jump.
 */
#if defined(__i386__)
static inline int flsnz(int x)
{
	int r;
	__asm__("bsrl %1,%0\n"
	        : "=r" (r) : "rm" (x));
	return r+1;
}
#else
// returns 1 to 32 for 1<<0 to 1<<31. Undefined for 0.
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
#endif

static inline int fls64(unsigned long long x)
{
	unsigned int h;

	h = x >> 32;
	if (h)
		return flsnz(h) + 32;
	return flsnz(x);
}

#define fls_auto(x) ((sizeof(x) > 4) ? fls64(x) : flsnz(x))

#ifndef container_of
#define container_of(ptr, type, name) ((type *)(((void *)(ptr)) - ((long)&((type *)0)->name)))
#endif

/*
 * Gcc >= 3 provides the ability for the programme to give hints to the
 * compiler about what branch of an if is most likely to be taken. This
 * helps the compiler produce the most compact critical paths, which is
 * generally better for the cache and to reduce the number of jumps.
 */
#if __GNUC__ < 3
#define __builtin_expect(x,y) (x)
#endif

#define likely(x) (__builtin_expect((x) != 0, 1))
#define unlikely(x) (__builtin_expect((x) != 0, 0))


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Each eb_node contains two views, one node and one leaf. Both may be
 * referenced in a pointer.
 */
#define EB_LEAF     0
#define EB_NODE     1

/* Number of bits per node, and number of leaves per node */
#define EB_NODE_BITS          1
#define EB_NODE_BRANCHES      (1 << EB_NODE_BITS)
#define EB_NODE_BRANCH_MASK   (EB_NODE_BRANCHES - 1)

#define EB_LEFT     0
#define EB_RGHT     1

/* This is the same as an eb_node pointer, except that the lower bit embeds
 * a tag. This tag has two meanings :
 *  - 0=left, 1=right for leaf_p/node_p
 *  - 0=link, 1=leaf  for branch[]
 */
typedef void eb_troot_t;

/* The eb_root connects the node which contains it to two nodes below it, one
 * of which may be the same node. At the top of the tree, we use an eb_root
 * too, which always has its right branch NULL.
 */
struct eb_root {
	eb_troot_t    *b[EB_NODE_BRANCHES]; /* left and right branches */
};

/* The eb_node contains the two parts, one for the leaf, which always exists,
 * and one for the node, which remains unused in the very first node inserted
 * into the tree.
 * This structure is 20 bytes per node on 32-bit machines.
 */
struct eb_node {
	struct eb_root branches; /* branches, must be at the beginning */
	eb_troot_t    *node_p;  /* link node's parent */
	eb_troot_t    *leaf_p;  /* leaf node's parent */
	int           bit;     /* link's bit position. */
};

/* Those structs carry nodes and data. They must start with the eb_node so that
 * any eb*_node can be cast into an eb_node.
 */
struct eb32_node {
	struct eb_node node; /* the tree node, must be at the beginning */
	u32 val;
};

struct eb64_node {
	struct eb_node node; /* the tree node, must be at the beginning */
	u64 val;
};

/* The root of a tree is an eb_root initialized with both pointers NULL.
 * During its life, only the left pointer will change. The right one will
 * always remain NULL, which is the way we detect it.
 */

#define EB_ROOT						\
	(struct eb_root) {				\
		.b = {[0] = NULL, [1] = NULL },		\
	}

#define EB_TREE_HEAD(name) 				\
	struct eb_root name = EB_ROOT


#ifndef DEPRECATED
/********************************************************************/
#define EB64_ROOT							\
	(struct eb64_node) {						\
		.node = { .bit = 64, 					\
			  .node_p = NULL, .leaf_p = NULL,		\
			  .branches = { .b = {[0] = NULL, [1] = NULL }}}, \
		.val = 0,						\
	}

#define EB64_TREE_HEAD(name) 						\
	struct eb64_node name = EB64_ROOT
/********************************************************************/
#endif /* DEPRECATED */


/* Converts a root pointer pointer to its equivalent eb_troot_t pointer,
 * ready to be stored in ->branch[], leaf_p or node_p. NULL is not
 * conserved. To be used with EB_LEAF, EB_NODE, EB_LEFT or EB_RGHT in <tag>.
 */
static inline eb_troot_t *
eb_dotag(const struct eb_root *root, const int tag)
{
	return (eb_troot_t *)((void *)root + tag);
}

/* Converts an eb_troot_t pointer pointer to its equivalent eb_root pointer,
 * for use with pointers from ->branch[], leaf_p or node_p. NULL is conserved
 * as long as the tree is not corrupted. To be used with EB_LEAF, EB_NODE,
 * EB_LEFT or EB_RGHT in <tag>.
 */
static inline struct eb_root *
eb_untag(const eb_troot_t *troot, const int tag)
{
	return (struct eb_root *)((void *)troot - tag);
}

/* returns the tag associated with an eb_troot_t pointer */
static inline int
eb_gettag(eb_troot_t *troot)
{
	return (unsigned long)troot & 1;
}

/* Returns a pointer to the eb_node holding <root> */
static inline struct eb_node *
eb_root_to_node(struct eb_root *root)
{
	return container_of(root, struct eb_node, branches);
}

/* Walks down starting at root pointer <start>, and always walking on side
 * <side>. It either returns the link to the first leaf on that side, or NULL if no leaf
 * is left. <start> may either be NULL, a link pointer, but not a dup. The
 * link pointer to the leaf (or NULL) is returned.
 */
static inline struct eb_node *
eb_walk_down(eb_troot_t *start, unsigned int side)
{
	/* A NULL pointer on an empty tree root will be returned as-is */
	while (eb_gettag(start) == EB_NODE)
		start = (eb_untag(start, EB_NODE))->b[side];
	/* NULL is left untouched (root==eb_node, EB_LEAF==0) */
	return eb_root_to_node(eb_untag(start, EB_LEAF));
}

/* Returns the first leaf in the tree starting at <root>, or NULL if none */
static inline struct eb_node *
__eb_first(struct eb_root *root)
{
	return eb_walk_down(root->b[0], EB_LEFT);
}

/* Returns the last leaf in the tree starting at <root>, or NULL if none */
static inline struct eb_node *
__eb_last(struct eb_root *root)
{
	return eb_walk_down(root->b[0], EB_RGHT);
}

/* Returns previous leaf node before an existing leaf node, or NULL if none. */
static inline struct eb_node *
__eb_prev(struct eb_node *node)
{
	eb_troot_t *t = node->leaf_p;

	while (eb_gettag(t) == EB_LEFT) {
		/* Walking up from left branch. We must ensure that we never
		 * walk beyond root.
		 */
		if (unlikely((eb_untag(t, EB_LEFT))->b[EB_RGHT] == NULL))
			return NULL;
		t = (eb_root_to_node(eb_untag(t, EB_LEFT)))->node_p;
	}
	/* Note that <t> cannot be NULL at this stage */
	t = (eb_untag(t, EB_RGHT))->b[EB_LEFT];
	return eb_walk_down(t, EB_RGHT);
}

/* Returns next leaf node after an existing leaf node, or NULL if none. */
static inline struct eb_node *
__eb_next(struct eb_node *node)
{
	eb_troot_t *t = node->leaf_p;

	while (eb_gettag(t) != EB_LEFT)
		/* Walking up from right branch, so we cannot be below root */
		t = (eb_root_to_node(eb_untag(t, EB_RGHT)))->node_p;

	/* Note that <t> cannot be NULL at this stage */
	t = (eb_untag(t, EB_LEFT))->b[EB_RGHT];
	return eb_walk_down(t, EB_LEFT);
}


/* Removes a leaf node from the tree, and returns zero after deleting the
 * last node. Otherwise, non-zero is returned.
 */
static inline int
__eb_delete(struct eb_node *node)
{
	unsigned int pside, gpside, sibtype;
	struct eb_node *parent;
	struct eb_root *gparent;

	/* we need the parent, our side, and the grand parent */
	pside = eb_gettag(node->leaf_p);
	parent = eb_root_to_node(eb_untag(node->leaf_p, pside));

	/* We likely have to release the parent link, unless it's the root,
	 * in which case we only set our branch to NULL. Note that we can
	 * only be attached to the root by its left branch.
	 */

	if (parent->branches.b[EB_RGHT] == NULL) {
		/* we're just below the root, it's trivial. */
		parent->branches.b[EB_LEFT] = NULL;
		return 0;
	}

	/* To release our parent, we have to identify our sibling, and reparent
	 * it directly to/from the grand parent. Note that the sibling can
	 * either be a link or a leaf.
	 */

	gpside = eb_gettag(parent->node_p);
	gparent = eb_untag(parent->node_p, gpside);

	gparent->b[gpside] = parent->branches.b[!pside];
	sibtype = eb_gettag(gparent->b[gpside]);

	if (sibtype == EB_LEAF) {
		eb_root_to_node(eb_untag(gparent->b[gpside], EB_LEAF))->leaf_p =
			eb_dotag(gparent, gpside);
	} else {
		eb_root_to_node(eb_untag(gparent->b[gpside], EB_NODE))->node_p =
			eb_dotag(gparent, gpside);
	}
	/* Mark the parent unused. Note that we do not check if the parent is
	 * our own node, but that's not a problem because if it is, it will be
	 * marked unused at the same time, which we'll use below to know we can
	 * safely remove it.
	 */
	parent->node_p = NULL;

	/* The parent node has been detached, and is currently unused. It may
	 * belong to another node, so we cannot remove it that way. Also, our
	 * own node part might still be used. so we can use this spare node
	 * to replace ours if needed.
	 */

	/* If our link part is unused, we can safely exit now */
	if (!node->node_p)
		return 1; /* tree is not empty yet */

	/* From now on, <node> and <parent> are necessarily different, and the
	 * <node>'s node part is in use. By definition, <parent> is at least
	 * below <node>, so keeping its value for the bit string is OK.
	 */

	parent->node_p = node->node_p;
	parent->branches = node->branches;
	parent->bit = node->bit;

	/* We must now update the new node's parent... */
	gpside = eb_gettag(parent->node_p);
	gparent = eb_untag(parent->node_p, gpside);
	gparent->b[gpside] = eb_dotag(&parent->branches, EB_NODE);

	/* ... and its branches */
	for (pside = 0; pside <= 1; pside++) {
		if (eb_gettag(parent->branches.b[pside]) == EB_NODE) {
			eb_root_to_node(eb_untag(parent->branches.b[pside], EB_NODE))->node_p =
				eb_dotag(&parent->branches, pside);
		} else {
			eb_root_to_node(eb_untag(parent->branches.b[pside], EB_LEAF))->leaf_p =
				eb_dotag(&parent->branches, pside);
		}
	}

	/* Now the node has been completely unlinked */
	return 1; /* tree is not empty yet */
}

///*
// * This generic insert macro may be used as a building block for other integer
// * types. Do not put any comments in this one to avoid any problem. Please
// * refer to __eb32_insert() for comments.
// */
//#define __eb_insert(root, new) do {                                           \
//	__label__ __out_insert;                                               \
//	typeof(root) __ro = root;                                             \
//	typeof(new)  __n  = new;                                              \
//	typeof(new)  __nxt;                                                   \
//	unsigned int __lf = (__n->val >> (__ro->node.bit - 1)) & 1;           \
//	__nxt = (typeof(__nxt))__ro->node.branch[__lf];                         \
//	if (unlikely(__nxt == NULL)) {                                        \
//		__ro->node.branch[__lf] = (struct eb_node *)__n;                \
//		__n->node.leaf_p = (struct eb_node *)__ro;                    \
//		__n->node.bit = 0;                                            \
//		goto __out_insert;                                            \
//	}                                                                     \
//	while (1) {                                                           \
//		if (unlikely(__nxt->node.leaf_p == (struct eb_node *)__ro)) { \
//			if (__nxt->val == __n->val) {                         \
//				__n->node.leaf_p = NULL;                      \
//				__n->node.bit = 0;                            \
//				goto __out_insert;                            \
//			}                                                     \
//			__nxt->node.leaf_p = (struct eb_node *)__n;           \
//			break;                                                \
//		}                                                             \
//		if (((__n->val ^ __nxt->val) >> __nxt->node.bit) != 0) {      \
//			__nxt->node.node_p = (struct eb_node *)__n;           \
//			break;                                                \
//		}                                                             \
//		__ro = __nxt;                                                 \
//		__lf = (__n->val >> (__nxt->node.bit - 1)) & 1;               \
//		__nxt = (typeof(__nxt))__nxt->node.branch[__lf];                \
//	}                                                                     \
//	__ro->node.branch[__lf] = (struct eb_node *)__n;                        \
//	__n->node.node_p = (struct eb_node *)__ro;                            \
//	__n->node.leaf_p = (struct eb_node *)__n;                             \
//	__n->node.bit = fls_auto(__n->val ^ __nxt->val);                      \
//	__lf = (__n->val > __nxt->val);                                       \
//	__n->node.branch[__lf ^ 1] = (struct eb_node *)__nxt;                   \
//	__n->node.branch[__lf] = (struct eb_node *)__n;                         \
// __out_insert:                                                                \
//	;                                                                     \
//} while (0)


///*
// * This generic lookup macro may be used as a building block for other integer
// * types. Do not put any comments in this one to avoid any problem. Please
// * refer to __eb32_lookup() for comments.
// */
//#define __eb_lookup(root, x) ({                                               \
//	__label__ __out_lookup;                                               \
//	typeof(root)    __ro = root;                                          \
//	typeof(x)       __x = x;                                              \
//	struct eb_node *__par = (struct eb_node *)__ro;                       \
//	__ro = (typeof(root))__par->branch[(__x >> (__par->bit - 1)) & 1];      \
//	if (unlikely(!__ro))                                                  \
//		goto __out_lookup;                                            \
//	while (1) {                                                           \
//		if (unlikely(__ro->node.leaf_p == __par)) {                   \
//			if (__ro->val != __x)                                 \
//				__ro = NULL;                                  \
//			break;                                                \
//		}                                                             \
//		if (unlikely((__x ^ __ro->val) == 0))                         \
//			break;                                                \
//		if (unlikely((__x ^ __ro->val) >> __ro->node.bit)) {          \
//			__ro = NULL;                                          \
//			break;                                                \
//		}                                                             \
//		__par = (struct eb_node *)__ro;                               \
//		if ((__x >> (__par->bit - 1)) & 1)                            \
//			__ro = (typeof(root))__par->branch[1];                  \
//		else                                                          \
//			__ro = (typeof(root))__par->branch[0];                  \
//	}                                                                     \
// __out_lookup:                                                                \
//	__ro;                                                                 \
//})


/********************************************************************/
/*         The following functions are data type-specific           */
/********************************************************************/

/*********** FIXME ************/
/*
 * Finds the first occurence of a value in the tree <root>. If none can be
 * found, NULL is returned.
 */
static inline struct eb32_node *
__eb32_lookup(struct eb32_node *root, unsigned long x)
{
	struct eb_node *parent = (struct eb_node *)root;

	root = (struct eb32_node *)parent->branches.b[x >> 31];
	if (unlikely(!root))
		return NULL;

	while (1) {
		if (unlikely(root->node.leaf_p == parent)) {
			/* reached a leaf */
			if (root->val == x)
				return root;
			else
				return NULL;
		}
#if 1
		/* Optimization 1: if x is equal to the exact value of the node,
		 * it implies that this node contains a leaf with this exact
		 * value, so we can return it now.
		 * This can boost by up to 50% on randomly inserted values, but may
		 * degrade by 5-10% when values have been carefully inserted in order,
		 * which is not exactly what we try to use anyway.
		 */
		if (unlikely((x ^ root->val) == 0))
			return root;
#endif
#if 1
		/* Optimization 2: if there are no bits in common anymore, let's
		 * stop right now instead of going down to the leaf.
		 * This one greatly improves performance in sparse trees, but it
		 * appears that repeating this test at every level in complete
		 * trees instead degrades performance by about 5%. Anyway, it
		 * generally is worth it.
		 */
		if (unlikely((x ^ root->val) >> root->node.bit))
			return NULL;
#endif

		parent = (struct eb_node *)root;

		// Don't ask why this slows down like hell ! Gcc completely
		// changes all the loop sequencing !
		// root = (struct eb32_node *)parent->branch[((x >> (parent->bit - 1)) & 1)];

		if ((x >> (parent->bit - 1)) & 1)
			root = (struct eb32_node *)parent->branches.b[1];
		else
			root = (struct eb32_node *)parent->branches.b[0];
	}
}


/* Inserts eb32_node <new> into subtree starting at node root <root>.
 * Only new->val needs be set with the value. The eb32_node is returned.
 */
static inline struct eb32_node *
__eb32_insert(struct eb_root *root, struct eb32_node *new) {
	struct eb32_node *old;
	unsigned int side;
	eb_troot_t *troot;
	u32 newval; /* caching the value saves approximately one cycle */

	side = EB_LEFT;
	troot = root->b[EB_LEFT];
	if (unlikely(troot == NULL)) {
		/* Tree is empty, insert the leaf part below the left branch */
		root->b[EB_LEFT] = eb_dotag(&new->node.branches, EB_LEAF);
		new->node.leaf_p = eb_dotag(root, EB_LEFT);
		new->node.node_p = NULL; /* node part unused */
		return new;
	}

	/* The tree descent is fairly easy :
	 *  - first, check if we have reached a leaf node
	 *  - second, check if we have gone too far
	 *  - third, reiterate
	 * Everywhere, we use <new> for the node node we are inserting, <root>
	 * for the node we attach it to, and <old> for the node we are
	 * displacing below <new>. <troot> will always point to the future node
	 * (tagged with its type). <side> carries the side the node <new> is
	 * attached to below its parent, which is also where previous node
	 * was attached. <newval> carries the value being inserted.
	 */
	newval = new->val;

	while (1) {
		if (unlikely(eb_gettag(troot) == EB_LEAF)) {
			eb_troot_t *new_left, *new_rght;
			eb_troot_t *new_leaf, *old_leaf;

			old = container_of(eb_untag(troot, EB_LEAF),
					    struct eb32_node, node.branches);

			new_left = eb_dotag(&new->node.branches, EB_LEFT);
			new_rght = eb_dotag(&new->node.branches, EB_RGHT);
			new_leaf = eb_dotag(&new->node.branches, EB_LEAF);
			old_leaf = eb_dotag(&old->node.branches, EB_LEAF);

			new->node.node_p = old->node.leaf_p;

			/* The tree did contain this value exactly once. We
			 * insert the new node just above the previous one,
			 * with the new leaf on the right.
			 */
			if ((newval ^ old->val) == 0) {  // (newval == old->val)
				old->node.leaf_p = new_left;
				new->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = old_leaf;
				new->node.branches.b[EB_RGHT] = new_leaf;
				new->node.bit = -1;
				root->b[side] = eb_dotag(&new->node.branches, EB_NODE);
				return new;
			}

			/* The tree did not contain this value, so we insert
			 * <new> before the leaf <old>, and set ->bit to
			 * designate the lowest bit position in <new> which
			 * applies to ->branches.b[].
			 *
			 * We need to check on which of the root's leaves the
			 * node will be attached. For this we have to compare
			 * its value to old's. Strictly speaking, this works
			 * with 2 leaves, but it would require some bitmask
			 * checks if we had higher numbers of leaves.
			 */
			if (newval < old->val) {
				new->node.leaf_p = new_left;
				old->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = new_leaf;
				new->node.branches.b[EB_RGHT] = old_leaf;
			} else {
				old->node.leaf_p = new_left;
				new->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = old_leaf;
				new->node.branches.b[EB_RGHT] = new_leaf;
			}
			break;
		}

		/* OK we're walking down this link */
		old = container_of(eb_untag(troot, EB_NODE),
				    struct eb32_node, node.branches);

		/* Stop going down when we don't have common bits anymore. We
		 * also stop in front of a duplicates tree because it means we
		 * have to insert above.
		 */

		if ((old->node.bit < 0) || /* we're above a duplicate tree, stop here */
		    (((newval ^ old->val) >> old->node.bit) >= EB_NODE_BRANCHES)) {
			/* The tree did not contain the value, so we insert <new> before the node
			 * <old>, and set ->bit to designate the lowest bit position in <new>
			 * which applies to ->branches.b[].
			 */
			eb_troot_t *new_left, *new_rght;
			eb_troot_t *new_leaf, *old_node;

			if (newval == old->val) {
				/* FIXME!!!! insert_dup(&old->node, &new->node); */
				return new;
			}

			new_left = eb_dotag(&new->node.branches, EB_LEFT);
			new_rght = eb_dotag(&new->node.branches, EB_RGHT);
			new_leaf = eb_dotag(&new->node.branches, EB_LEAF);
			old_node = eb_dotag(&old->node.branches, EB_NODE);

			new->node.node_p = old->node.node_p;

			if (newval < old->val) {
				new->node.leaf_p = new_left;
				old->node.node_p = new_rght;
				new->node.branches.b[EB_LEFT] = new_leaf;
				new->node.branches.b[EB_RGHT] = old_node;
			} else {
				old->node.node_p = new_left;
				new->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = old_node;
				new->node.branches.b[EB_RGHT] = new_leaf;
			}
			break;
		}

		/* walk down */
		root = &old->node.branches;
		side = (newval >> old->node.bit) & EB_NODE_BRANCH_MASK;
		troot = root->b[side];
	}

	/* Ok, now we are inserting <new> between <root> and <old>. <old>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <side>. Update the root's leaf till we have it. Note that we can also
	 * find the side by checking the side of new->node.node_p.
	 */

	/* We need the common higher bits between newval and old->val.
	 * What differences are there between newval and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of newval and old->val are identical here (otherwise they
	 * would sit on different branches).
	 */
	// note that if EB_NODE_BITS > 1, we should check that it's still >= 0
	new->node.bit = flsnz(newval ^ old->val) - EB_NODE_BITS;
	root->b[side] = eb_dotag(&new->node.branches, EB_NODE);

	return new;
}


	/********** OK TILL THERE *********/




/* Inserts node <new> into subtree starting at link node <root>.
 * Only new->leaf.val needs be set with the value.
 * The node is returned.
 */

/************ FIXME: must be rewritten *************/
#if BITS_PER_LONG == 64
static inline struct eb64_node *
__eb64_insert(struct eb64_node *root, struct eb64_node *new) {
	struct eb64_node *next;
	unsigned int l;
	u64 x;

	x = new->val;
	l = x >> 63;

	next = (struct eb64_node *)root->node.branches.b[l];
	if (unlikely(next == NULL)) {
		root->node.branches.b[l] = (struct eb_node *)new;
		/* This can only happen on the root node. */
		/* We'll have to insert our new leaf node here. */
		new->node.leaf_p = &root->node;
		new->node.bit = 0; /* FIXME!!! link part unused */
		new->node.node_p = NULL; /* FIXME: node part unused */
		return new;
	}

	/*
	 * This loop is the critical path in large trees.
	 */
	while (1) {
		if (unlikely(next->node.leaf_p == &root->node)) {
			/* we're on a leaf node */
			if (next->val == x) {
				/* We are inserting a value we already have. */
				/* FIXME!!! add a duplicate here */
				//insert_dup(&next->node, &new->node);
				return new;
			}
			/* Set the leaf's parent to the new node */
			next->node.leaf_p = (struct eb_node *)new;
			break;
		}

		/* Stop going down when we don't have common bits anymore. */
		if (((x ^ next->val) >> next->node.bit) != 0) {
			/* Set the link's parent to the new node */
			next->node.node_p = (struct eb_node *)new;
			break;
		}

		/* walk down */
		root = next;
		l = (x >> (next->node.bit - 1)) & 1;
		next = (struct eb64_node *)next->node.branches.b[l];
	}

	/* Ok, now we are inserting <new> between <root> and <next>. <next>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <l>. Update the root's leaf till we have it.
	 */
	root->node.branch[l] = (struct eb_node *)new;

	/* We need the common higher bits between x and next->val.
	 * What differences are there between x and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of x and next->val are identical here (otherwise they
	 * would sit on different branches).
	 */

	new->node.node_p = &root->node;
	new->node.leaf_p = (struct eb_node *)new;
	new->node.bit = fls64(x ^ next->val);   /* lower identical bit */

	/* This optimization is a bit tricky. The goal is to put new->branch as well
	 * as the other leaf on the right branch of the new parent link, depending
	 * on which one is bigger.
	 */
	l = (x > next->val);
	new->node.branch[l ^ 1] = &next->node;
	new->node.branch[l] = (struct eb_node *)new;

	return new;
}

#else /* BITS_PER_LONG != 64 */

/************ FIXME: must be rewritten *************/
static inline struct eb64_node *
__eb64_insert(struct eb64_node *root, struct eb64_node *new) {
	struct eb64_node *next;
	unsigned int l;
	u64 x;
	u32 lo, hi;

	x = new->val;
	l = x >> 63;

	next = (struct eb64_node *)root->node.branches.b[l];
	if (unlikely(next == NULL)) {
		root->node.branches.b[l] = (struct eb_node *)new;
		/* This can only happen on the root node. */
		/* We'll have to insert our new leaf node here. */
		new->node.leaf_p = &root->node;
		new->node.bit = 0; /* FIXME!!! node part unused */
		new->node.node_p = NULL; /* FIXME: node part unused */
		return new;
	}

	/*
	 * This loop is the critical path in large trees.
	 */
	while (1) {
		lo = x ^ next->val;
		hi = (x ^ next->val) >> 32;

		if (unlikely(next->node.leaf_p == &root->node)) {
			/* we're on a leaf node */
			if (!(hi | lo)) {
				/* We are inserting a value we already have. */
				/* FIXME!!! add a duplicate here */
				//insert_dup(&next->node, &new->node);
				return new;
			}
			/* Set the leaf's parent to the new node */
			next->node.leaf_p = (struct eb_node *)new;
			break;
		}

		/* Stop going down when we don't have common bits anymore. */
		if (((next->node.bit < 32) &&
		     (hi || (lo >> next->node.bit))) ||
		    ((next->node.bit >= 32) &&
		     ((hi >> (next->node.bit - 32)) != 0))) {
			/* Set the link's parent to the new node */
			next->node.node_p = (struct eb_node *)new;
			break;
		}

		/* walk down */
		root = next;
		if (next->node.bit >= 33)
			l = ((u32)(x >> 32) >> (next->node.bit - 33)) & 1;
		else
			l = ((u32)x >> (next->node.bit - 1)) & 1;
		/* l&1 below is just a hint for gcc */
		next = (struct eb64_node *)next->node.branches.b[l&1];
	}

	/* Ok, now we are inserting <new> between <root> and <next>. <next>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <l>. Update the root's leaf till we have it.
	 */
	root->node.branches.b[l] = (struct eb_node *)new;

	/* We need the common higher bits between x and next->val.
	 * What differences are there between x and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of x and next->val are identical here (otherwise they
	 * would sit on different branches).
	 */

	new->node.node_p = &root->node;
	new->node.leaf_p = (struct eb_node *)new;

	/* Trick: <hi> is still valid, but using it right here forces the
	 * compiler to propagate the value across the function, which clobbers
	 * some registers. It's cheaper to recompute them now on architectures
	 * with low number of registers.
	 */
	hi = (x ^ next->val) >> 32;
	new->node.bit = hi ? 32 + flsnz(hi)
	                   : flsnz(x ^ next->val);   /* lower identical bit */

	/* This optimization is a bit tricky. The goal is to put new->branches.b as well
	 * as the other leaf on the right branch of the new parent link, depending
	 * on which one is bigger.
	 */
	l = (x > next->val);

	new->node.branches.b[l ^ 1] = &next->node;
	new->node.branches.b[l] = (struct eb_node *)new;

	return new;
}

#endif /* BITS_PER_LONG == 64 */


/********************************************************************/


struct eb32_node *eb32_lookup(struct eb32_node *root, unsigned long x);
struct eb32_node *eb32_insert(struct eb_root *root, struct eb32_node *new);
struct eb64_node *eb64_insert(struct eb64_node *root, struct eb64_node *new);
int eb_delete(struct eb_node *node);
struct eb_node *eb_first(struct eb_root *root);
struct eb_node *eb_last(struct eb_root *root);
struct eb_node *eb_prev(struct eb_node *node);
struct eb_node *eb_next(struct eb_node *node);


#define eb32_delete(node)						      \
	(eb_delete((struct eb_node *)(node)))

#define __eb32_delete(node)						      \
	(__eb_delete((struct eb_node *)(node)))

#define eb64_delete(node)						      \
	(eb_delete((struct eb_node *)(node)))

#define __eb64_delete(node)						      \
	(__eb_delete((struct eb_node *)(node)))

#define eb_entry(ptr, type, member) container_of(ptr, type, member)



/*********************************************************************/



/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 */
