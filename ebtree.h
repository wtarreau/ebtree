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
       struct eb_node *link_p;  // link node's parent
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

   Useful properties :
     - links are only provided above the leaf, never below. This implies that
       the nodes directly attached to the root do not use their link. It also
       enhances the probability that the link directly above a leaf are from
       the same node.

     - a link connected to its own leaf will have leaf_p = node = leaf[0|1].

     - leaf[0] can never be equal to leaf[1] except for the root which can have
       them both NULL.

     - link_p can never be equal to the same node.

     - two leaves can never point to the same location

     - duplicates do not use their link part, nor their leaf_p pointer.

     - links do not use their dup list.

     - those cannot be merged because a leaf at the head of a dup list needs
       both a link above it and a dup list.

     - a leaf-only node should have some easily distinguishable info in the
       link part, such as NULL or a pointer to the same node (which cannot
       happen in normal case). The NULL might be better to identify the root.

     - bit is necessarily > 0.

   Basic definitions (subject to change) :
     - for duplicate leaf nodes, leaf_p = NULL.
     - use bit == 0 to indicate a leaf node which is not used as a link
     - root->bit = INTBITS for the represented data type (eg: 32)
     - root->link_p = root->leaf_p = NULL
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

#ifndef LIST_INIT

#define LIST_INIT(l) ((l)->n = (l)->p = (l))
#define LIST_ADDQ(lh, el) ({ (el)->p = (lh)->p; (el)->p->n = (lh)->p = (el); (el)->n = (lh); (el); })
#define LIST_DEL(el) ({ typeof(el) __ret = (el); (el)->n->p = (el)->p; (el)->p->n = (el)->n; (__ret); })
#define LIST_ELEM(lh, pt, el) ((pt)(((void *)(lh)) - ((void *)&((pt)NULL)->el)))

struct list {
	struct list *n;	/* next */
	struct list *p;	/* prev */
};
#endif


#ifndef container_of
#define container_of(ptr, type, member) ({      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);  \
	(type *)( (char *)__mptr - offsetof(type,member) );})
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
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

#define LINK_SIDE_LEFT  0
#define LINK_SIDE_RIGHT 1

/* 28 bytes per node on 32-bit machines. */
struct eb_node {
	unsigned short  bit;     /* link's bit position. */
	unsigned short  side;    /* link's side below its parent */
	struct eb_node *leaf_p;  /* leaf node's parent */
	struct eb_node *link_p;  /* link node's parent */
	struct eb_node *leaf[2]; /* link's leaf nodes */
	struct list     dup;     /* leaf duplicates */
};

/* Those structs carry nodes and data. They must start with the eb_node so that
 * any eb*_node can be cast into an eb_node.
 */
struct eb32_node {
	struct eb_node node;
	u32 val;
};

struct eb64_node {
	struct eb_node node;
	u64 val;
};

/*
 * The root is a node initialized with :
 * - bit = 32, so that we split on bit 31 below it
 * - val = 0
 * - parent* = left = right = NULL
 * During its life, only left and right will change. Checking
 * that parent = NULL should be enough to retain from deleting it.
 *
 */


/********************************************************************/

#define EB32_ROOT							\
	(struct eb32_node) {						\
		.node = { .bit = 32, 					\
			  .link_p = NULL, .leaf_p = NULL,		\
			  .leaf = { [0] = NULL, [1] = NULL },		\
			  .dup = { .n = NULL, .p = NULL }},		\
		.val = 0,						\
	}

#define EB32_TREE_HEAD(name) 						\
	struct eb32_node name = EB32_ROOT

#define EB64_ROOT							\
	(struct eb64_node) {						\
		.node = { .bit = 64, 					\
			  .link_p = NULL, .leaf_p = NULL,		\
			  .leaf = { [0] = NULL, [1] = NULL },		\
			  .dup = { .n = NULL, .p = NULL }},		\
		.val = 0,						\
	}

#define EB64_TREE_HEAD(name) 						\
	struct eb64_node name = EB64_ROOT


/* Walks down link node <root> starting with <start> leaf, and always walking
 * on side <side>. It either returns the first leaf on that side, or NULL if
 * no leaf is left. Note that <root> may either be NULL or a link node, but
 * it must not be a leaf-only node. <start> may either be NULL, a link node,
 * or a heading leaf node (not a dup). The leaf (or NULL) is returned.
 */
static inline struct eb_node *
eb_walk_down(struct eb_node *root, unsigned int side, struct eb_node *start)
{
	if (!start)
		return start;	/* only possible at root */
	while (start->leaf_p != root) {
		root = start;
		start = start->leaf[side];
	};
	return start;
}

/* Walks up starting from node <node> with parent <par>, which must be a valid
 * parent (ie: link_p or leaf_p, and <node> must not be a duplicate). It
 * follows side <side> for as long as possible, and stops when it reaches a
 * node which sees it on the other side, or before attempting to go beyond the
 * root. The pointer to the closest common ancestor is returned, which might be
 * NULL if none is found.
 */
static inline struct eb_node *
eb_walk_up(struct eb_node *node, int side, struct eb_node *par)
{
	while (par && par->leaf[side] == node) {
		node = par;
		par = par->link_p;
	}
	return par;
}

/* Walks up starting from valid leaf node <node> which must not be a duplicate,
 * It follows side <side> for as long as possible, and stops when it reaches a
 * node which sees it on the other side, or before attempting to go beyond the
 * root. The pointer to the closest common ancestor is returned, which might be
 * NULL if none is found.
 */
static inline struct eb_node *
eb_walk_up_from_leaf(struct eb_node *node, int side)
{
	struct eb_node *par;

	par = node->leaf_p;
	if (likely(par->leaf[side] == node)) {
		do {
			node = par;
			par = par->link_p;
			/* This test looks tricky, but it helps the compiler
			 * produce good code by only emitting comparisons to
			 * zero. As the function is inlined, the test is
			 * optimized away by the compiler. -WT */
		} while (unlikely(((!side && !node->side) || (side && node->side)) && par));
	}
	return par;
}

#define eb_walk_down_left(node, start)			\
	eb_walk_down((struct eb_node *)node, 0, (struct eb_node *)start)

#define eb_walk_down_right(node, start)			\
	eb_walk_down((struct eb_node *)node, 1, (struct eb_node *)start)

#define eb_walk_up_left_with_parent(node, par)			\
	eb_walk_up(node, 0, par)

#define eb_walk_up_right_with_parent(node, par)			\
	eb_walk_up(node, 1, par)

/* Walks up left starting from leaf node <node> */
#define eb_walk_up_left(node)					\
	eb_walk_up_left_with_parent((node), (node)->leaf_p)

/* Walks up right starting from leaf node <node> */
#define eb_walk_up_right(node)					\
	eb_walk_up_right_with_parent((node), (node)->leaf_p)


/* Returns the pointer to the other node sharing the same parent. This
 * method is tricky but avoids a test and is faster.
 */
#define eb_sibling_with_parent(node, par)			\
	((struct eb_node *)					\
	 (((unsigned long)(par)->leaf[0]) ^			\
	  ((unsigned long)(par)->leaf[1]) ^			\
	  ((unsigned long)(node))))

#define eb_sibling_with_parent_test(node, par)			\
	(((par)->leaf[1] == (node)) ? (par)->leaf[0] : (par)->leaf[1])

/* returns first leaf in the tree starting at <root>, or NULL if none */
#define eb_first(root)							\
	((typeof(root))__eb_first_node((struct eb_node *)(root)))

/* returns last leaf in the tree starting at <root>, or NULL if none */
#define eb_last(root)							\
	((typeof(root))__eb_last_node((struct eb_node *)(root)))

/* returns next leaf node after an existing leaf node, or NULL if none. */
#define eb_next(node)							\
	((typeof(node))__eb_next_node((struct eb_node *)(node)))

/* returns previous leaf node before an existing leaf node, or NULL if none. */
#define eb_prev(node)							\
	((typeof(node))__eb_prev_node((struct eb_node *)(node)))

/* Returns first leaf in the tree starting at <root>, or NULL if none */
static inline struct eb_node *
__eb_first_node(struct eb_node *root)
{
	int side;

	side = ((struct eb_node *)(root))->leaf[0] == NULL;
	return eb_walk_down_left((struct eb_node *)(root),
		((struct eb_node *)(root))->leaf[side]);
}

/* returns last leaf in the tree starting at <root>, or NULL if none */
static inline struct eb_node *
__eb_last_node(struct eb_node *root)
{
	int side;
	struct eb_node *node;

	side = ((struct eb_node *)(root))->leaf[1] != NULL;
	node = eb_walk_down_right((struct eb_node *)(root),
		((struct eb_node *)(root))->leaf[side]);
	/* let's return a possible last duplicate first */
	return LIST_ELEM(node->dup.p, struct eb_node *, dup);
}

/* returns previous leaf node before an existing leaf node, or NULL if none. */
static inline struct eb_node *
__eb_prev_node(struct eb_node *node)
{
	if (unlikely(!node->leaf_p)) {
		/* let's return duplicates before going further */
		return LIST_ELEM(node->dup.p, struct eb_node *, dup);
	}

	node = eb_walk_up_from_leaf(node, LINK_SIDE_LEFT);
	//node = eb_walk_up_left_with_parent(node, node->leaf_p);
	if (unlikely(!node))
		return node;

	node = eb_walk_down_right(node, node->leaf[0]);
	if (unlikely(!node))
	    return node;

	if (unlikely(node->dup.n != &node->dup)) {
		/* let's return last duplicate first */
		node = LIST_ELEM(node->dup.p, struct eb_node *, dup);
	}
	return node;
}

/* returns next leaf node after an existing leaf node, or NULL if none. */
static inline struct eb_node *
__eb_next_node(struct eb_node *node)
{
	if (unlikely(node->dup.n != &node->dup)) {
		/* let's return duplicates before going further */
		node = LIST_ELEM(node->dup.n, struct eb_node *, dup);
		if (!node->leaf_p)
			return node;
		/* we returned to the list's head, let's walk up now */
	}
	node = eb_walk_up_from_leaf(node, LINK_SIDE_RIGHT);
	//node = eb_walk_up_right_with_parent(node, node->leaf_p);
	return node ? eb_walk_down_left(node, node->leaf[1]) : node;
}

/* Removes a leaf node from the tree, and returns zero after deleting the
 * last node. Otherwise, non-zero is returned.
 */
static inline int
__eb_delete(struct eb_node *node)
{
	__label__ replace_link;
	unsigned int l;
	struct eb_node *newlink, *parent, *gparent;

	parent = node->leaf_p;

	/* Duplicates are simply unlinked, because we know they are not linked
	 * to anything. Also, we know the tree is not empty afterwards.
	 */
	if (!parent) {
		LIST_DEL(&node->dup);
		return 1;
	}

	/* List heads are copied then removed. The parent pointing to them is
	 * updated to point to the first duplicate. Since the heads may have
	 * their link part used somewhere else, and we know that the duplicate
	 * cannot have his in use, we can switch over to using his.
	 */
	if (node->dup.n != &node->dup) {
		newlink = LIST_ELEM(node->dup.n, struct eb_node *, dup);
		LIST_DEL(&node->dup);
		newlink->leaf_p = parent;

		l = (parent->leaf[1] == node);
		parent->leaf[l] = newlink;

		/* keep newlink intact as we can use it for the replacement */
		goto replace_link;
	}

	/* Here, we know that the node is not part of any duplicate list.
	 * We likely have to release the parent link, unless it's the root,
	 * in which case we only set our branch to NULL.
	 */
	gparent = parent->link_p;
	if (unlikely(gparent == NULL)) {
		l = (parent->leaf[1] == node);
		parent->leaf[l] = NULL;
		return !!parent->leaf[l ^ 1];
	}

	/* To release the parent, we have to identify our sibling, and reparent
	 * it directly to/from the grand parent. Note that the sibling can
	 * either be a link or a leaf.
	 */
	newlink = eb_sibling_with_parent(node, parent);
	if (newlink->leaf_p == parent)
		newlink->leaf_p = gparent;
	else
		newlink->link_p = gparent;

	l = (gparent->leaf[1] == parent);
	gparent->leaf[l] = newlink;

	/* Mark the parent unused. Note that we do not check if the parent is
	 * our own link, but that's not a problem because if it is, it will be
	 * marked unused at the same time, which we'll use below to know we can
	 * safely remove it.
	 */
	parent->bit = 0;

	/* The parent link has been detached, and is unused. So we can use it
	 * if we need to replace the node's link somewhere else.
	 */
	newlink = parent;

 replace_link:
	/* check whether our link part is in use */
	if (!node->bit)
		return 1; /* tree is not empty yet */

	/* From now on, node and newlink are necessarily different, and the
	 * node's link part is in use. By definition, <newlink> is at least
	 * below <link>, so keeping its value for the bit string is OK.
	 */

	newlink->link_p = node->link_p;
	newlink->leaf[0] = node->leaf[0];
	newlink->leaf[1] = node->leaf[1];
	newlink->bit = node->bit;

	/* We must now update the new link's parent */
	gparent = node->link_p;
	if (gparent->leaf[0] == node)
		gparent->leaf[0] = newlink;
	else
		gparent->leaf[1] = newlink;

	/* ... and the link's leaves */
	for (l = 0; l <= 1; l++) {
		if (newlink->leaf[l]->leaf_p == node)
			newlink->leaf[l]->leaf_p = newlink;
		else
			newlink->leaf[l]->link_p = newlink;
	}

	/* Now the node has been completely unlinked */
	return 1; /* tree is not empty yet */
}

/*
 * This generic insert macro may be used as a building block for other integer
 * types. Do not put any comments in this one to avoid any problem. Please
 * refer to __eb32_insert() for comments.
 */
#define __eb_insert(root, new) do {                                           \
	__label__ __out_insert;                                               \
	typeof(root) __ro = root;                                             \
	typeof(new)  __n  = new;                                              \
	typeof(new)  __nxt;                                                   \
	unsigned int __lf = (__n->val >> (__ro->node.bit - 1)) & 1;           \
	__nxt = (typeof(__nxt))__ro->node.leaf[__lf];                         \
	if (unlikely(__nxt == NULL)) {                                        \
		__ro->node.leaf[__lf] = (struct eb_node *)__n;                \
		__n->node.leaf_p = (struct eb_node *)__ro;                    \
		LIST_INIT(&__n->node.dup);                                    \
		__n->node.bit = 0;                                            \
		goto __out_insert;                                            \
	}                                                                     \
	while (1) {                                                           \
		if (unlikely(__nxt->node.leaf_p == (struct eb_node *)__ro)) { \
			if (__nxt->val == __n->val) {                         \
				LIST_ADDQ(&__nxt->node.dup, &__n->node.dup);  \
				__n->node.leaf_p = NULL;                      \
				__n->node.bit = 0;                            \
				goto __out_insert;                            \
			}                                                     \
			__nxt->node.leaf_p = (struct eb_node *)__n;           \
			break;                                                \
		}                                                             \
		if (((__n->val ^ __nxt->val) >> __nxt->node.bit) != 0) {      \
			__nxt->node.link_p = (struct eb_node *)__n;           \
			break;                                                \
		}                                                             \
		__ro = __nxt;                                                 \
		__lf = (__n->val >> (__nxt->node.bit - 1)) & 1;               \
		__nxt = (typeof(__nxt))__nxt->node.leaf[__lf];                \
	}                                                                     \
	__ro->node.leaf[__lf] = (struct eb_node *)__n;                        \
	__n->node.link_p = (struct eb_node *)__ro;                            \
	__n->node.leaf_p = (struct eb_node *)__n;                             \
	__n->node.bit = fls_auto(__n->val ^ __nxt->val);                      \
	__lf = (__n->val > __nxt->val);                                       \
	__n->node.leaf[__lf ^ 1] = (struct eb_node *)__nxt;                   \
	__n->node.leaf[__lf] = (struct eb_node *)__n;                         \
	LIST_INIT(&__n->node.dup);                                            \
 __out_insert:                                                                \
	;                                                                     \
} while (0)


/*
 * This generic lookup macro may be used as a building block for other integer
 * types. Do not put any comments in this one to avoid any problem. Please
 * refer to __eb32_lookup() for comments.
 */
#define __eb_lookup(root, x) ({                                               \
	__label__ __out_lookup;                                               \
	typeof(root)    __ro = root;                                          \
	typeof(x)       __x = x;                                              \
	struct eb_node *__par = (struct eb_node *)__ro;                       \
	__ro = (typeof(root))__par->leaf[(__x >> (__par->bit - 1)) & 1];      \
	if (unlikely(!__ro))                                                  \
		goto __out_lookup;                                            \
	while (1) {                                                           \
		if (unlikely(__ro->node.leaf_p == __par)) {                   \
			if (__ro->val != __x)                                 \
				__ro = NULL;                                  \
			break;                                                \
		}                                                             \
		if (unlikely((__x ^ __ro->val) == 0))                         \
			break;                                                \
		if (unlikely((__x ^ __ro->val) >> __ro->node.bit)) {          \
			__ro = NULL;                                          \
			break;                                                \
		}                                                             \
		__par = (struct eb_node *)__ro;                               \
		if ((__x >> (__par->bit - 1)) & 1)                            \
			__ro = (typeof(root))__par->leaf[1];                  \
		else                                                          \
			__ro = (typeof(root))__par->leaf[0];                  \
	}                                                                     \
 __out_lookup:                                                                \
	__ro;                                                                 \
})


/********************************************************************/
/*         The following functions are data type-specific           */
/********************************************************************/


/*
 * Finds the first occurence of a value in the tree <root>. If none can be
 * found, NULL is returned.
 */
static inline struct eb32_node *
__eb32_lookup(struct eb32_node *root, unsigned long x)
{
	struct eb_node *parent = (struct eb_node *)root;

	root = (struct eb32_node *)parent->leaf[x >> 31];
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
		// root = (struct eb32_node *)parent->leaf[((x >> (parent->bit - 1)) & 1)];

		if ((x >> (parent->bit - 1)) & 1)
			root = (struct eb32_node *)parent->leaf[1];
		else
			root = (struct eb32_node *)parent->leaf[0];
	}
}


/* Inserts node <new> into subtree starting at link node <root>.
 * Only new->leaf.val needs be set with the value.
 * The node is returned.
 */

static inline struct eb32_node *
__eb32_insert(struct eb32_node *root, struct eb32_node *new) {
	struct eb32_node *next;
	unsigned int l, s;
	u32 x;

	x = new->val;
	l = x >> 31;

	next = (struct eb32_node *)root->node.leaf[l];
	if (unlikely(next == NULL)) {
		new->node.side = l;
		root->node.leaf[l] = (struct eb_node *)new;
		/* This can only happen on the root node. */
		/* We'll have to insert our new leaf node here. */
		new->node.leaf_p = (struct eb_node *)root;
		LIST_INIT(&new->node.dup);
		new->node.bit = 0; /* link part unused */
		return new;
	}

	/*
	 * This loop is the critical path in large trees.
	 */
	while (1) {
		if (unlikely(next->node.leaf_p == (struct eb_node *)root)) {
			/* we're on a leaf node */
			if (next->val == x) {
				/* We are inserting a value we already have.
				 * We just have to join the duplicates list.
				 */
				LIST_ADDQ(&next->node.dup, &new->node.dup);
				new->node.leaf_p = NULL; /* we're a duplicate, no parent */
				new->node.bit = 0; /* link part unused */
				new->node.side = l;
				return new;
			}
			/* Set the leaf's parent to the new node */
			next->node.leaf_p = (struct eb_node *)new;
			s = (x < next->val);
			break;
		}

		/* Stop going down when we don't have common bits anymore. */
		if (((x ^ next->val) >> next->node.bit) != 0) {
			/* Set the link's parent to the new node */
			next->node.link_p = (struct eb_node *)new;
			s = (x < next->val);
			next->node.side = s;
			break;
		}

		/* walk down */
		root = next;
		l = (x >> (next->node.bit - 1)) & 1;
		next = (struct eb32_node *)next->node.leaf[l];
	}

	/* Ok, now we are inserting <new> between <root> and <next>. <next>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <l>. Update the root's leaf till we have it.
	 */
	new->node.side = l;
	root->node.leaf[l] = (struct eb_node *)new;

	/* We need the common higher bits between x and next->val.
	 * What differences are there between x and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of x and next->val are identical here (otherwise they
	 * would sit on different branches).
	 */

	new->node.link_p = (struct eb_node *)root;
	new->node.leaf_p = (struct eb_node *)new;
	new->node.bit = flsnz(x ^ next->val);   /* lower identical bit */

	/* This optimization is a bit tricky. The goal is to put new->leaf as well
	 * as the other leaf on the right branch of the new parent link, depending
	 * on which one is bigger.
	 */
	l = s ^ 1;
	new->node.leaf[s] = (struct eb_node *)next;
	new->node.leaf[l] = (struct eb_node *)new;

	/* now we build the leaf part and chain it directly below the link node */
	LIST_INIT(&new->node.dup);

	return new;
}


/* Inserts node <new> into subtree starting at link node <root>.
 * Only new->leaf.val needs be set with the value.
 * The node is returned.
 */

#if BITS_PER_LONG == 64
static inline struct eb64_node *
__eb64_insert(struct eb64_node *root, struct eb64_node *new) {
	struct eb64_node *next;
	unsigned int l;
	u64 x;

	x = new->val;
	l = x >> 63;

	next = (struct eb64_node *)root->node.leaf[l];
	if (unlikely(next == NULL)) {
		root->node.leaf[l] = (struct eb_node *)new;
		/* This can only happen on the root node. */
		/* We'll have to insert our new leaf node here. */
		new->node.leaf_p = (struct eb_node *)root;
		LIST_INIT(&new->node.dup);
		new->node.bit = 0; /* link part unused */
		return new;
	}

	/*
	 * This loop is the critical path in large trees.
	 */
	while (1) {
		if (unlikely(next->node.leaf_p == (struct eb_node *)root)) {
			/* we're on a leaf node */
			if (next->val == x) {
				/* We are inserting a value we already have.
				 * We just have to join the duplicates list.
				 */
				LIST_ADDQ(&next->node.dup, &new->node.dup);
				new->node.leaf_p = NULL; /* we're a duplicate, no parent */
				new->node.bit = 0; /* link part unused */
				return new;
			}
			/* Set the leaf's parent to the new node */
			next->node.leaf_p = (struct eb_node *)new;
			break;
		}

		/* Stop going down when we don't have common bits anymore. */
		if (((x ^ next->val) >> next->node.bit) != 0) {
			/* Set the link's parent to the new node */
			next->node.link_p = (struct eb_node *)new;
			break;
		}

		/* walk down */
		root = next;
		l = (x >> (next->node.bit - 1)) & 1;
		next = (struct eb64_node *)next->node.leaf[l];
	}

	/* Ok, now we are inserting <new> between <root> and <next>. <next>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <l>. Update the root's leaf till we have it.
	 */
	root->node.leaf[l] = (struct eb_node *)new;

	/* We need the common higher bits between x and next->val.
	 * What differences are there between x and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of x and next->val are identical here (otherwise they
	 * would sit on different branches).
	 */

	new->node.link_p = (struct eb_node *)root;
	new->node.leaf_p = (struct eb_node *)new;
	new->node.bit = fls64(x ^ next->val);   /* lower identical bit */

	/* This optimization is a bit tricky. The goal is to put new->leaf as well
	 * as the other leaf on the right branch of the new parent link, depending
	 * on which one is bigger.
	 */
	l = (x > next->val);
	new->node.leaf[l ^ 1] = (struct eb_node *)next;
	new->node.leaf[l] = (struct eb_node *)new;

	/* now we build the leaf part and chain it directly below the link node */
	LIST_INIT(&new->node.dup);

	return new;
}

#else /* BITS_PER_LONG != 64 */

static inline struct eb64_node *
__eb64_insert(struct eb64_node *root, struct eb64_node *new) {
	struct eb64_node *next;
	unsigned int l;
	u64 x;
	u32 lo, hi;

	x = new->val;
	l = x >> 63;

	next = (struct eb64_node *)root->node.leaf[l];
	if (unlikely(next == NULL)) {
		root->node.leaf[l] = (struct eb_node *)new;
		/* This can only happen on the root node. */
		/* We'll have to insert our new leaf node here. */
		new->node.leaf_p = (struct eb_node *)root;
		LIST_INIT(&new->node.dup);
		new->node.bit = 0; /* link part unused */
		return new;
	}

	/*
	 * This loop is the critical path in large trees.
	 */
	while (1) {
		lo = x ^ next->val;
		hi = (x ^ next->val) >> 32;

		if (unlikely(next->node.leaf_p == (struct eb_node *)root)) {
			/* we're on a leaf node */
			if (!(hi | lo)) {
				/* We are inserting a value we already have.
				 * We just have to join the duplicates list.
				 */
				LIST_ADDQ(&next->node.dup, &new->node.dup);
				new->node.leaf_p = NULL; /* we're a duplicate, no parent */
				new->node.bit = 0; /* link part unused */
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
			next->node.link_p = (struct eb_node *)new;
			break;
		}

		/* walk down */
		root = next;
		if (next->node.bit >= 33)
			l = ((u32)(x >> 32) >> (next->node.bit - 33)) & 1;
		else
			l = ((u32)x >> (next->node.bit - 1)) & 1;
		/* l&1 below is just a hint for gcc */
		next = (struct eb64_node *)next->node.leaf[l&1];
	}

	/* Ok, now we are inserting <new> between <root> and <next>. <next>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <l>. Update the root's leaf till we have it.
	 */
	root->node.leaf[l] = (struct eb_node *)new;

	/* We need the common higher bits between x and next->val.
	 * What differences are there between x and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of x and next->val are identical here (otherwise they
	 * would sit on different branches).
	 */

	new->node.link_p = (struct eb_node *)root;
	new->node.leaf_p = (struct eb_node *)new;

	/* Trick: <hi> is still valid, but using it right here forces the
	 * compiler to propagate the value across the function, which clobbers
	 * some registers. It's cheaper to recompute them now on architectures
	 * with low number of registers.
	 */
	hi = (x ^ next->val) >> 32;
	new->node.bit = hi ? 32 + flsnz(hi)
	                   : flsnz(x ^ next->val);   /* lower identical bit */

	/* This optimization is a bit tricky. The goal is to put new->leaf as well
	 * as the other leaf on the right branch of the new parent link, depending
	 * on which one is bigger.
	 */
	l = (x > next->val);

	new->node.leaf[l ^ 1] = (struct eb_node *)next;
	new->node.leaf[l] = (struct eb_node *)new;

	/* now we build the leaf part and chain it directly below the link node */
	LIST_INIT(&new->node.dup);

	return new;
}

#endif /* BITS_PER_LONG == 64 */


/********************************************************************/


struct eb32_node *eb32_lookup(struct eb32_node *root, unsigned long x);
struct eb32_node *eb32_insert(struct eb32_node *root, struct eb32_node *new);
struct eb64_node *eb64_insert(struct eb64_node *root, struct eb64_node *new);
int eb_delete(struct eb_node *node);
struct eb_node *eb_first_node(struct eb_node *root);
struct eb_node *eb_last_node(struct eb_node *root);
struct eb_node *eb_prev_node(struct eb_node *node);
struct eb_node *eb_next_node(struct eb_node *node);


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
