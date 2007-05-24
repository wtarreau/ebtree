/*
 * ebtree.h : Elastic Binary Trees - macros and structures.
 * (C) 2002-2007 - Willy Tarreau - willy@ant-computing.com
 *
 * 2007/05/18: adapted the structure to support embedded nodes
 * 2007/05/13: adapted to mempools v2.
 *
 */



/*

  General idea:
  In a radix binary tree, we may have up to 2N-1 nodes for N values if all of
  them are leaves. If we provide one spare node with each value and we link
  all of them together, it is possible to build a self-contained radix tree.
  Spare nodes will be a self-contained linked list.

  tree root :
  - pointer to first node
  - pointer to first spare node

  When an entry is deleted, either it had at most one leaf, in which case it is
  reparented, or it had two leaves, in which case we have to replace it with a
  spare node from the list. If the entry provided a spare node, this one must
  be replaced by another one from the spare list.

  When an entry is added, if its value is used by a spare node, it MUST be
  replaced so that we never run out of spare nodes.

  Note that we MUST NEVER link data to a spare node. So it is important that
  a spare node is easily distinguished from a data node. Note that the spare
  node cannot have duplicates, so we might use this information for that
  purpose (eg: put NULLs in it).

  Also, if we want to be able to delete a node without referring to the root,
  we need to be able to scan the spare list to find another spare node in case
  ours was used by some other node. Having a link to the head of the spare list
  might be needed. We also need to be able to remove the spare node from the
  spare list. This requires that the spare nodes are dual-linked together.

  ----
  We can simplify the process by having data only on leaf nodes and the glue
  nodes on all intermediate nodes. Leaves will then be attached to the highest
  upper glue node (="parent"). Then the leaves will carry data and will need no
  L/R pointers. The parents will need no guests. The main problem with this
  approach is that some values will appear at several positions in the tree, in
  parents and leaves, requiring to reach the leaves to read the values. But it
  might be worth it to scan the tree in cheap iterations.

  Eg, to store 8, 10, 12, 13, 14 :

      ultree       this tree

        8              8
       / \            / \
      10 12          10 12
        /  \           /  \
       13  14         12  14
                     / \
                    12 13

    parent node : 5/6 words
      - up, left, right
      - length, prefix(32/64)

    leaf node : 5/6 words
      - up, dup.next, dup.prev
      - length(0), value(32/64) or pointer to leaf

    => total = 10/12 words per entry.

   Note that on real-world tests (with a scheduler), is was verified that the
   case with data on an intermediate node never happens. This is because the
   population is too large for such coincidences to happen. It would require
   for instance that a task has its expiration time at an exact second, with
   other tasks sharing that second. This is too rare to try to optimize for it.


   struct eb_half_node {
      struct list nodes; // n,p for duplicates or l,r for links
      struct eb_half_node *u;
      int bit;   // lower bit number. 0 means we're in a leaf node
      u64 val; // value or prefix
   };

   struct eb_node {
     struct eb_half_node leaf;
     struct eb_half_node link;
   };

   Maybe further investigation will lead to separate the data from the rest, to
   get something like this :

   struct eb_half_node {
      struct list nodes;
      struct eb_half_node *parent;
      int bit;
   };

   struct eb_node {
     struct eb_half_node leaf;
     struct eb_half_node link;
     u64 leaf_data;
     u64 link_data;
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
   above it, it has no link in used.

 */


#if defined(__i386__)
static inline int fls(int x)
{
        int r;

        __asm__("bsrl %1,%0\n\t"
                "jnz 1f\n\t"
                "movl $-1,%0\n"
                "1:" : "=r" (r) : "rm" (x));
        return r+1;
}
#else
// returns 1 to 32 for 1<<0 to 1<<31. Undefined for 0.
#define fls(___a) ({ \
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

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* 10 bytes, may be stored in a 256*10 = 2.5kB array to sort 256 values. */
struct eb8i_hnode {
    struct { u8 n, p; } ptr; /* navigation: either left/right or next/prev */
    u8 parent;               /* parent node. Points to self for root. */
    u8 bit;                  /* bit position, 0 for leaf */
    u8 val;                  /* prefix of value */
};

struct eb8i_node {
    struct eb8i_hnode leaf;
    struct eb8i_hnode link;
};

/* 20 bytes, may be stored in a 65536*20 = 1.2MB array to sort 65536 values. */
struct eb16i_hnode {
    struct { u16 n, p; } ptr; /* navigation: either left/right or next/prev */
    u16 parent;               /* parent node. Points to self for root. */
    u16 bit;                  /* bit position, 0 for leaf */
    u16 val;                  /* prefix of value */
};

struct eb16i_node {
    struct eb16i_hnode leaf;
    struct eb16i_hnode link;
};


/* 40 bytes per node on 32-bit machines.
 * For link nodes, the 'ptr' pointers are used this way :
 * - prev = left (bit unset)
 * - next = right (bit set)
 *
 * Unused linked nodes must have their 'bit' set to zero, which makes it easy
 * to check against when deleting nodes.
 */
struct eb32_hnode {
    union {
	struct list dup;                    // duplicates (for leaves)
	struct eb32_hnode *leaves[2];       // leaves (for links)
    } ptr;
    struct eb32_hnode *parent; // parent
    u32 val;                   // prefix or value
    u32 bit;                   // lower bit number, 0 means leaf node
};

/* Note: the ordering is not very important, but tests show that having leaf
 * first slightly improves insertion and walk time (about 2%).
 */
struct eb32_node {
    struct eb32_hnode leaf;
    struct eb32_hnode link;
};

/*
 * The root is a half-node initialized with :
 * - bit = 32, so that we split on bit 31 below it
 * - val = 0
 * - parent = left = right = NULL
 * During its life, only left and right will change. Checking
 * that parent = NULL should be enough to retain from deleting it.
 * 
 */


/********************************************************************/




#define EB32_TREE_HEAD(name) 					\
	struct eb32_hnode name = { 				\
	    .bit = 32, .val = 0, .parent = NULL, 		\
	        .ptr  = { .leaves = { [0] = NULL, [1] = NULL }}	\
	}


/* Walks down left link node <node>, which must not be NULL. It stops at the
 * leaf, and returns the node carrying it. Note that <node> can itself be a
 * leaf, which will immediately be returned.
 */
#define eb_walk_down_left(node, type)			\
({							\
    typeof (node) __n = (node);				\
    while (__n->bit)					\
	__n = __n->ptr.leaves[0];			\
    LIST_ELEM(__n, type*, leaf);			\
})


/* Walks down right link node <node>, which must not be NULL. It stops at the
 * leaf, and returns the node carrying it. Note that <node> can itself be a
 * leaf, which will immediately be returned.
 */
#define eb_walk_down_right(node, type)			\
({							\
    typeof (node) __n = (node);				\
    while (__n->bit)					\
	__n = __n->ptr.leaves[1];			\
    LIST_ELEM(__n, type*, leaf);			\
})


/* Walks up left starting from node <node>, which must have a valid parent (ie
 * not just a duplicate). It stops when it gets a pointer to the next left
 * node, or NULL, meaning it reached the root of the tree. The pointer to the
 * next link node is returned.
 */
#define eb_walk_up_left_with_parent(node, par)		\
({							\
    __label__ __out;					\
    typeof (node) __p = (par);				\
    typeof (node) __b = (node);				\
    while (__p->ptr.leaves[0] == __b) {			\
	__b = __p; __p = __p->parent;			\
	if (unlikely(!__p)) { __b = NULL; goto __out; }	\
    }							\
    __b = __p->ptr.leaves[0];				\
__out:							\
    __b;						\
})


/* Walks up right starting from node <node>, which must have a valid parent (ie
 * not just a duplicate). It stops when it gets a pointer to the next right
 * node, or NULL, meaning it reached the root of the tree. The pointer to the
 * next link node is returned.
 */
#define eb_walk_up_right_with_parent(node, par)		\
({							\
    __label__ __out;					\
    typeof (node) __p = (par);				\
    typeof (node) __b = (node);				\
    while (__p->ptr.leaves[1] == __b) {			\
	__b = __p; __p = __p->parent;			\
	if (unlikely(!__p)) { __b = NULL; goto __out; }	\
    }							\
    __b = __p->ptr.leaves[1];				\
__out:							\
    __b;						\
})


#define eb_walk_up_left(node)					\
	eb_walk_up_left_with_parent((node), (node)->parent)


#define eb_walk_up_right(node)					\
	eb_walk_up_right_with_parent((node), (node)->parent)


/* returns the pointer to the other node sharing the same parent */
#define eb_sibling_with_parent(node, par)			\
	(typeof (node))						\
	(((unsigned long)(par)->ptr.leaves[0]) ^		\
	 ((unsigned long)(par)->ptr.leaves[1]) ^		\
	 ((unsigned long)(node)))


/* returns the pointer to the other node sharing the same parent */
#define eb_sibling(node)					\
	eb_sibling_with_parent((node), (node)->parent)


/* returns first leaf in the tree starting at <root>, or NULL if none */
#define eb_first(root, type)						\
    ((root)->ptr.leaves[0]) ?						\
	eb_walk_down_left((root)->ptr.leaves[0], type) :		\
	(((root)->ptr.leaves[1]) ?					\
	 eb_walk_down_left((root)->ptr.leaves[1], type) :		\
	 NULL)


/* returns last leaf in the tree starting at <root>, or NULL if none */
#define eb_last(root, type)						\
    ((root)->ptr.leaves[1]) ?						\
	eb_walk_down_right((root)->ptr.leaves[1], type) :		\
	(((root)->ptr.leaves[0]) ?					\
	 eb_walk_down_right((root)->ptr.leaves[0], type) :		\
	 NULL)


/* returns previous leaf node before an existing leaf node, or NULL if none. */
#define eb_prev(node)							\
({									\
    __label__ __out;							\
    typeof (node) __n = (node);						\
    typeof ((node)->link) *__prev;					\
    if (__n->leaf.ptr.dup.p != &__n->leaf.ptr.dup) {			\
	__prev = LIST_ELEM(__n->leaf.ptr.dup.p,				\
			   typeof (__n->link) *,			\
			   ptr.dup);					\
	__n = LIST_ELEM(__prev, typeof (*node) *, leaf);		\
	if (!__prev->parent)						\
	    goto __out;							\
	/* we returned to the list's head, let's walk up now */		\
    }									\
    __prev = eb_walk_up_left(&__n->leaf);				\
    __n = likely(__prev) ? eb_walk_down_right(__prev, typeof (*node))	\
                         : NULL;					\
__out:									\
    __n;								\
})


/* returns next leaf node after an existing leaf node, or NULL if none. */
#define eb_next(node)							\
({									\
    __label__ __out;							\
    typeof (node) __n = (node);						\
    typeof ((node)->link) *__next;					\
    if (__n->leaf.ptr.dup.n != &__n->leaf.ptr.dup) {			\
	__next = LIST_ELEM(__n->leaf.ptr.dup.n,				\
			   typeof (__n->link) *,			\
			   ptr.dup);					\
	__n = LIST_ELEM(__next, typeof (*node) *, leaf);		\
	if (!__next->parent)						\
	    goto __out;							\
	/* we returned to the list's head, let's walk up now */		\
    }									\
    __next = eb_walk_up_right(&__n->leaf);				\
    __n = likely(__next) ? eb_walk_down_left(__next, typeof (*node))	\
                         : NULL;					\
__out:									\
    __n;								\
})


/********************************************************************/




static inline struct eb32_node *
eb32_walk_down_left(struct eb32_hnode *node)
{
    return eb_walk_down_left(node, struct eb32_node);
}

static inline struct eb32_node *
eb32_walk_down_right(struct eb32_hnode *node)
{
    return eb_walk_down_right(node, struct eb32_node);
}

static inline struct eb32_node *
eb32_first(struct eb32_hnode *root)
{
    return eb_first(root, struct eb32_node);
}

static inline struct eb32_node *
eb32_last(struct eb32_hnode *root)
{
    return eb_last(root, struct eb32_node);
}

static inline struct eb32_node *
eb32_next(struct eb32_node *node)
{
    return eb_next(node);
}

static inline struct eb32_node *
eb32_prev(struct eb32_node *node)
{
    return eb_prev(node);
}



/********************************************************************/




#ifdef STATS
extern unsigned long total_jumps;
#define COUNT_STATS  total_jumps++;
#else
#define COUNT_STATS
#endif


/* Inserts node <new> into subtree starting at link node <root>.
 * Only new->leaf.val needs be set with the value.
 * The node is returned.
 */

static inline struct eb32_node *
__eb32_insert(struct eb32_hnode *root, struct eb32_node *new) {
    struct eb32_hnode *next;
    struct eb32_hnode **branch;
    int l;
    u32 x;

    x = new->leaf.val;

    next = root;
    do {
	COUNT_STATS;

	/* walk down */
	root = next;
	if ((x >> (next->bit - 1)) & 1) {
	    branch = &next->ptr.leaves[1];
	    next = *branch;
	} else {
	    branch = &next->ptr.leaves[0];
	    next = *branch;
	}

	/* this can only happen on root node */
	if (unlikely(next == NULL)) {
	    /* we'll have to insert our new leaf node here */
	    *branch = &new->leaf;
	    new->leaf.parent = root;
	    LIST_INIT(&new->leaf.ptr.dup);

	    new->link.bit = 0; /* link node is not used */
	    new->leaf.bit = 0; /* leaf node */
	    return new;
	}

	/* this happens on leaf nodes */
	if (unlikely(!next->bit)) {
	    if (unlikely(x == next->val)) {
		/* We have found the exact value, we join the duplicates */
		LIST_ADDQ(&next->ptr.dup, &new->leaf.ptr.dup);
		new->leaf.parent = NULL; /* we're a duplicate, no parent */

		new->link.bit = 0; /* link node is not used */
		new->leaf.bit = 0; /* leaf node */
		return new;
	    }
	    break;
	}

	/* Stop going down when we don't have common bits anymore. */
    } while (((x ^ next->val) >> next->bit) == 0);

    /* Ok, now we know that we must insert between <root> and <next>. Also, we
     * still have the branch pointer from <root> in <branch>.
     */

    /* We need the common higher bits between x and next->low.
     * What differences are there between x and the node here ?
     * NOTE that bit(new) is always < bit(parent) because highest
     * bit of x and next->val are identical here (else they would
     * be on a different branch).
     */

    new->link.parent = root;
    new->link.bit = fls(x ^ next->val);   /* lower identical bit */
    new->link.val = x;
    /* If we need to be able to later scan by integer ranges, it will be useful
     * to flush the bits not covered by this node's position :
     * new->link.val = x & ~((1 << new->link.bit) - 1);
     */

    /* This optimization is a bit tricky. The goal is to put new->leaf as well
     * as the other leaf on the right branch of the new parent link, depending
     * on which one is bigger.
     */
    l = (x > next->val);
    new->link.ptr.leaves[l ^ 1] = next;
    new->link.ptr.leaves[l] = &new->leaf;

    /* this could be done anywhere */
    *branch = &new->link;
    next->parent = &new->link;

    /* now we build the leaf node and chain it directly below the link node */
    new->leaf.parent = &new->link;

    LIST_INIT(&new->leaf.ptr.dup);
    new->leaf.bit = 0; /* leaf node */
    return new;
}








/*********************************************************************/


/* Removes a node from the tree, and returns a pointer to the next remaining
 * node sharing the same duplicates, parent or grand-parent. It may be a link
 * node, or a leaf node when the node was in a duplicate list.
 * This makes it easier to massively remove entries without searching next
 * entry from scratch. Note that when deleting last node, the return will be
 * NULL.
 */
static inline struct eb32_hnode *
__eb32_delete(struct eb32_node *node)
{
    __label__ replace_link;
    struct eb32_hnode *leaf, *link, *newlink, *parent, *gparent, *sibling;

    leaf = &node->leaf;
    link = &node->link;
    parent = leaf->parent;

    /* duplicates are simply removed */
    if (!parent) {
	sibling = LIST_ELEM(leaf->ptr.dup.n, struct eb32_hnode *, ptr.dup);
	LIST_DEL(&leaf->ptr.dup);
	return sibling;
    }

    /* List heads are copied then removed. The parent pointing to them is
     * updated to point to the first duplicate. Since the heads may have their
     * link part used somewhere else, and we know that the duplicate cannot
     * have his in use, we can switch over to using his.
     */
    if (leaf->ptr.dup.n != &leaf->ptr.dup) {
	sibling = LIST_ELEM(leaf->ptr.dup.n, struct eb32_hnode *, ptr.dup);
	LIST_DEL(&leaf->ptr.dup);
	sibling->parent = parent;

	if (parent->ptr.leaves[0] == leaf)
	    parent->ptr.leaves[0] = sibling;
	else
	    parent->ptr.leaves[1] = sibling;
	
	newlink = &(LIST_ELEM(sibling, struct eb32_node*, leaf))->link;
	/* keep sibling intact as we'll use it as the return value */
	goto replace_link;
    }

    /* Here, we know that the node is not part of any duplicate list.
     * We likely have to release the parent link, unless it's the root,
     * in which case we only set our branch to NULL.
     */
    gparent = parent->parent;
    if (unlikely(gparent == NULL)) {
	if (parent->ptr.leaves[0] == leaf) {
	    parent->ptr.leaves[0] = NULL;
	    return parent->ptr.leaves[1];
	}
	else {
	    parent->ptr.leaves[1] = NULL;
	    return parent->ptr.leaves[0];
	}
    }

    /* To release the parent, we have to identify our sibling, and link it
     * directly to/from the grand parent. The sibling will also be returned.
     */
    sibling = eb_sibling_with_parent(leaf, parent);
    sibling->parent = gparent;

    if (gparent->ptr.leaves[0] == parent)
	gparent->ptr.leaves[0] = sibling;
    else
	gparent->ptr.leaves[1] = sibling;

    /* Mark parent as unused. Note that we do not check if the parent is
     * our own link, but that's not a problem because if it is, it will
     * be marked unused at the same time, which we'll use to know we can
     * safely remove it.
     */
    parent->bit = 0;

    /* The parent link has been detached, and is unused. So we can use it
     * if we need to replace the node's link.
     */

    newlink = parent;

 replace_link:
    /* check whether the link part is in use */
    if (!link->bit)
	return sibling;

    /* Note that now, link and newlink are necessary different */

    /* Important: do not change newlink->val, as it is still equal to the value
     * of the node which carries it. By definition, <newlink> is at least below
     * <link>, so keeping its value for the bit string is OK.
     */
    //*newlink = *link;
    newlink->ptr = link->ptr;
    newlink->parent = link->parent;
    newlink->bit = link->bit;


    /* We must now update the link's parent and the link's leaves */
    gparent = link->parent;
    if (gparent->ptr.leaves[0] == link)
	gparent->ptr.leaves[0] = newlink;
    else
	gparent->ptr.leaves[1] = newlink;

    newlink->ptr.leaves[0]->parent = newlink;
    newlink->ptr.leaves[1]->parent = newlink;

    /* Now the node has been completely unlinked */
    return sibling;
}


/*
 * Finds the first occurence of a value in the tree <root>. If none can be
 * found, NULL is returned.
 */
static inline struct eb32_node *
__eb32_lookup(struct eb32_hnode *root, unsigned long x)
{
    while (1) {
	// Don't ask why this slows down like hell ! Gcc changes all
	// the loop !
	//root = root->ptr.leaves[((x >> (root->bit - 1)) & 1)];

	if ((x >> (root->bit - 1)) & 1)
	    root = root->ptr.leaves[1];
	else
	    root = root->ptr.leaves[0];

	/* may only happen in tree root */
	if (unlikely(!root))
	   return NULL;

	if (!root->bit) {
	    /* reached a leaf */
	    if (x == root->val)
		return LIST_ELEM(root, struct eb32_node*, leaf);
	    else
		return NULL;
	}

	if (unlikely((x ^ root->val) >> root->bit))
	    /* no common bits anymore */
	    return NULL;
    }
}

