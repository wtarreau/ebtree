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

/* The links used in node_p / leaf_p are tagged with EB_SIDE_* to indicate the
 * side of the parent's branch they're attached to. The stored links are of
 * type ebx_link_t, which depends on the storage model. These links are built
 * using ebx_setlink() from the link's memory position and a tagged root
 * pointer. For absolute addressing, the tagged root pointer is simply used
 * as-is. For relative addressing, the distance between the link's storage
 * address and its target is stored instead. The tagged root pointer, in turn,
 * is assembled by __ebx_dotag() from a valid absolute pointer and a side.
 * Parent pointers point to the upper node's "branches" structure. This is
 * convenient for the root as the tree's root is only made of this structure.
 *
 * So the model looks like this :
 *
 *   Node1                             Node2
 *     link -------------------------->  branches
 *      |                                   |
 *      |                                   V
 *      |          tagged_ptr = __ebx_dotag(&node2->branches, side)
 *      |            |
 *       \          /
 *         \      /
 *      __ebx_setlink(&link, tagged_ptr)
 *            |
 *            V
 *      sets link's value --->  tagged_ptr = __ebx_getroot(&link)
 *                                         |
 *                                         V
 *                           side = __ebx_get_parent_side(tagged_ptr)
 *                       branches = __ebx_untag(tagged_ptr, side)
 *
 * The tagged pointer doesn't really need to be represented as a pointer,
 * though tests have shown that it's hard for compilers to propagate some
 * operation simplifications over casts between scalars and pointers. For
 * this reason it's much more efficient to store them as regular pointers
 * to branches which explains why they're declared of type ebx_link_t *.
 *
 * The same principle applies to branches where the branch pointer points to
 * the children's "branches" part, and where the tagged branch pointer carries
 * the branch type (node or leaf).
 *
 * Functions __ebx_setlink() and __ebx_getroot() respectively set a link from
 * a tagged pointer or retrieve a tagged pointer from a link.
 *
 * Function __ebx_dotag() adds a known tag value to an absolute pointer to build
 * a tagged pointer.
 *
 * Function __ebx_untag() removes a known tag value from a tagged pointer to
 * return the absolute pointer. The tag value may be retrieved from the absolute
 * pointer using __ebx_get_parent_side(tagged_ptr) when reading a parent pointer
 * (node_p or leaf_p), and using __ebx_get_branch_type(tagged_ptr) when reading
 * a branch pointer.
 *
 */

/* The link type is either a signed int or a void *, the type is declared by
 * the caller.
 */
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

#ifdef EB_TREE_RELATIVE
/* relative pointer mode, we must never have branches[] at position zero */
struct ebx_node {
	ebx_link_t       node_p;  /* link node's parent */
	ebx_link_t       leaf_p;  /* leaf node's parent */
	struct ebx_root branches; /* branches, must be at the beginning */
	short int       bit;     /* link's bit position. */
	short unsigned int pfx; /* data prefix length, always related to leaf */
} __attribute__((packed));
#else
/* absolute pointer mode : optimal with branches first */
struct ebx_node {
	struct ebx_root branches; /* branches, must be at the beginning */
	ebx_link_t       node_p;  /* link node's parent */
	ebx_link_t       leaf_p;  /* leaf node's parent */
	short int       bit;     /* link's bit position. */
	short unsigned int pfx; /* data prefix length, always related to leaf */
} __attribute__((packed));
#endif


/* These functions are declared in ebxtree.c */
void __ebx_delete(struct ebx_node *node);
REGPRM1 struct ebx_node *__ebx_insert_dup(struct ebx_node *sub, struct ebx_node *new);

/***************************************\
 * Private functions. Not for end-user *
\***************************************/

/* Converts a root pointer to its equivalent ebx_troot_t pointer,
 * ready to be stored in ->branch[], leaf_p or node_p. NULL is not
 * conserved. To be used with EB_TYPE_LEAF, EB_TYPE_NODE, EB_SIDE_LEFT or EB_SIDE_RGHT in <tag>.
 */
static inline ebx_troot_t *__ebx_dotag(const struct ebx_root *root, const int tag)
{
	return (ebx_troot_t *)((char *)root + tag);
}

/* Converts an ebx_troot_t pointer pointer to its equivalent ebx_root pointer,
 * for use with pointers from ->branch[], leaf_p or node_p. NULL is conserved
 * as long as the tree is not corrupted. To be used with EB_TYPE_LEAF, EB_TYPE_NODE,
 * EB_SIDE_LEFT or EB_SIDE_RGHT in <tag>.
 */
static inline struct ebx_root *__ebx_untag(const ebx_troot_t *troot, const int tag)
{
	return (struct ebx_root *)((char *)troot - tag);
}

/* returns the tag associated with an ebx_troot_t pointer */
static inline int __ebx_gettag(ebx_troot_t *troot)
{
	return (unsigned long)troot & 1;
}

/* Returns a pointer to the ebx_node holding <root>, where <root> is stored at <base> */
static inline struct ebx_node *__ebx_root_to_node(struct ebx_root *root)
{
	return container_of(root, struct ebx_node, branches);
}

#ifdef EB_TREE_RELATIVE
/* Assigns a pointer to a link.
 * NOTE: We store the pointer as a relative offset of 2 so that it can never
 * match a valid pointer. The default implementation does not consider NULL
 * pointers because they almost never appear in the trees.
 */
static inline void __ebx_setlink(ebx_link_t *dest, const ebx_troot_t *troot)
{
	*dest = (void *)troot - (void *)dest;
}

/* Returns the pointer from a link */
static inline ebx_troot_t *__ebx_getroot(const ebx_link_t *src)
{
	return *src + (void *)src;
}

/* A relative offset is NULL if it's either 0 or 1 (tagged 0). The cast to
 * unsigned long long does not affect smaller types (verified).
 */
static inline int __ebx_link_is_null(ebx_link_t link)
{
	return link == 0;
}

/* A pointer designates the ROOT if its right branch is NULL. */
static inline int __ebx_is_root(struct ebx_root *root)
{
	return root->b[EB_SIDE_RGHT] <= 1;
}
#endif

/* Walks down starting at root pointer <start>, and always walking on side
 * <side>. It either returns the node hosting the first leaf on that side,
 * or NULL if no leaf is found. <start> may either be NULL or a branch pointer.
 * The pointer to the leaf (or NULL) is returned.
 */
static inline struct ebx_node *__ebx_walk_down(ebx_troot_t *start, unsigned int side)
{
	/* A NULL pointer on an empty tree root will be returned as-is */
	while (__ebx_gettag(start) == EB_TYPE_NODE)
		start = __ebx_getroot(&(__ebx_untag(start, EB_TYPE_NODE))->b[side]);
	/* NULL is left untouched (root==ebx_node, EB_TYPE_LEAF==0) */
	return __ebx_root_to_node(__ebx_untag(start, EB_TYPE_LEAF));
}


/**************************************\
 * Public functions, for the end-user *
\**************************************/

/* Return non-zero if the tree is empty, otherwise zero */
static inline int ebx_is_empty(struct ebx_root *root)
{
	return !root->b[EB_SIDE_LEFT];
}

/* Return non-zero if the node is a duplicate, otherwise zero */
static inline int __ebx_is_dup(struct ebx_node *node)
{
	return node->bit < 0;
}

/* Return the first leaf in the tree starting at <root>, or NULL if none */
static inline struct ebx_node *ebx_first(struct ebx_root *root)
{
	if (__ebx_link_is_null(root->b[EB_SIDE_LEFT]))
		return NULL;
	return __ebx_walk_down(__ebx_getroot(&root->b[0]), EB_SIDE_LEFT);
}

/* Return the last leaf in the tree starting at <root>, or NULL if none */
static inline struct ebx_node *ebx_last(struct ebx_root *root)
{
	if (__ebx_link_is_null(root->b[EB_SIDE_LEFT]))
		return NULL;
	return __ebx_walk_down(__ebx_getroot(&root->b[0]), EB_SIDE_RGHT);
}

/* Return previous leaf node before an existing leaf node, or NULL if none. */
static inline struct ebx_node *ebx_prev(struct ebx_node *node)
{
	ebx_troot_t *t = __ebx_getroot(&node->leaf_p);

	while (__ebx_gettag(t) == EB_SIDE_LEFT) {
		/* Walking up from left branch. We must ensure that we never
		 * walk beyond root.
		 */
		if (unlikely(__ebx_is_root(__ebx_untag(t, EB_SIDE_LEFT))))
			return NULL;
		t = __ebx_getroot(&(__ebx_root_to_node(__ebx_untag(t, EB_SIDE_LEFT)))->node_p);
	}
	/* Note that <t> cannot be NULL at this stage */
	t = __ebx_getroot(&(__ebx_untag(t, EB_SIDE_RGHT))->b[EB_SIDE_LEFT]);
	return __ebx_walk_down(t, EB_SIDE_RGHT);
}

/* Return next leaf node after an existing leaf node, or NULL if none. */
static inline struct ebx_node *ebx_next(struct ebx_node *node)
{
	ebx_troot_t *t = __ebx_getroot(&node->leaf_p);

	while (__ebx_gettag(t) != EB_SIDE_LEFT)
		/* Walking up from right branch, so we cannot be below root */
		t = __ebx_getroot(&(__ebx_root_to_node(__ebx_untag(t, EB_SIDE_RGHT)))->node_p);

	/* Note that <t> cannot be NULL at this stage */
	if (__ebx_is_root(__ebx_untag(t, EB_SIDE_LEFT)))
		return NULL;

	t = __ebx_getroot(&(__ebx_untag(t, EB_SIDE_LEFT))->b[EB_SIDE_RGHT]);
	return __ebx_walk_down(t, EB_SIDE_LEFT);
}

/* Return previous leaf node within a duplicate sub-tree, or NULL if none. */
static inline struct ebx_node *ebx_prev_dup(struct ebx_node *node)
{
	ebx_troot_t *t = __ebx_getroot(&node->leaf_p);

	while (__ebx_gettag(t) == EB_SIDE_LEFT) {
		/* Walking up from left branch. We must ensure that we never
		 * walk beyond root.
		 */
		if (unlikely(__ebx_is_root(__ebx_untag(t, EB_SIDE_LEFT))))
			return NULL;
		/* if the current node leaves a dup tree, quit */
		if ((__ebx_root_to_node(__ebx_untag(t, EB_SIDE_LEFT)))->bit >= 0)
			return NULL;
		t = __ebx_getroot(&(__ebx_root_to_node(__ebx_untag(t, EB_SIDE_LEFT)))->node_p);
	}
	/* Note that <t> cannot be NULL at this stage */
	if ((__ebx_root_to_node(__ebx_untag(t, EB_SIDE_RGHT)))->bit >= 0)
		return NULL;
	t = __ebx_getroot(&(__ebx_untag(t, EB_SIDE_RGHT))->b[EB_SIDE_LEFT]);
	return __ebx_walk_down(t, EB_SIDE_RGHT);
}

/* Return next leaf node within a duplicate sub-tree, or NULL if none. */
static inline struct ebx_node *ebx_next_dup(struct ebx_node *node)
{
	ebx_troot_t *t = __ebx_getroot(&node->leaf_p);

	while (__ebx_gettag(t) != EB_SIDE_LEFT) {
		/* Walking up from right branch, so we cannot be below root */
		/* if the current node leaves a dup tree, quit */
		if ((__ebx_root_to_node(__ebx_untag(t, EB_SIDE_RGHT)))->bit >= 0)
			return NULL;
		t = __ebx_getroot(&(__ebx_root_to_node(__ebx_untag(t, EB_SIDE_RGHT)))->node_p);
	}

	/* Note that <t> cannot be NULL at this stage */
	if ((__ebx_root_to_node(__ebx_untag(t, EB_SIDE_LEFT)))->bit >= 0)
		return NULL;

	if (unlikely(__ebx_is_root(__ebx_untag(t, EB_SIDE_LEFT))))
		return NULL;

	t = __ebx_getroot(&(__ebx_untag(t, EB_SIDE_LEFT))->b[EB_SIDE_RGHT]);
	return __ebx_walk_down(t, EB_SIDE_LEFT);
}

/* Return previous leaf node before an existing leaf node, skipping duplicates,
 * or NULL if none. */
static inline struct ebx_node *ebx_prev_unique(struct ebx_node *node)
{
	ebx_troot_t *t = __ebx_getroot(&node->leaf_p);

	while (1) {
		if (__ebx_gettag(t) != EB_SIDE_LEFT) {
			node = __ebx_root_to_node(__ebx_untag(t, EB_SIDE_RGHT));
			/* if we're right and not in duplicates, stop here */
			if (node->bit >= 0)
				break;
			t = __ebx_getroot(&node->node_p);
		}
		else {
			/* Walking up from left branch. We must ensure that we never
			 * walk beyond root.
			 */
			if (unlikely(__ebx_is_root(__ebx_untag(t, EB_SIDE_LEFT))))
				return NULL;
			t = __ebx_getroot(&(__ebx_root_to_node(__ebx_untag(t, EB_SIDE_LEFT)))->node_p);
		}
	}
	/* Note that <t> cannot be NULL at this stage */
	t = __ebx_getroot(&(__ebx_untag(t, EB_SIDE_RGHT))->b[EB_SIDE_LEFT]);
	return __ebx_walk_down(t, EB_SIDE_RGHT);
}

/* Return next leaf node after an existing leaf node, skipping duplicates, or
 * NULL if none.
 */
static inline struct ebx_node *ebx_next_unique(struct ebx_node *node)
{
	ebx_troot_t *t = __ebx_getroot(&node->leaf_p);

	while (1) {
		if (__ebx_gettag(t) == EB_SIDE_LEFT) {
			if (unlikely(__ebx_is_root(__ebx_untag(t, EB_SIDE_LEFT))))
				return NULL;	/* we reached root */
			node = __ebx_root_to_node(__ebx_untag(t, EB_SIDE_LEFT));
			/* if we're left and not in duplicates, stop here */
			if (node->bit >= 0)
				break;
			t = __ebx_getroot(&node->node_p);
		}
		else {
			/* Walking up from right branch, so we cannot be below root */
			t = __ebx_getroot(&(__ebx_root_to_node(__ebx_untag(t, EB_SIDE_RGHT)))->node_p);
		}
	}

	/* Note that <t> cannot be NULL at this stage */
	if (unlikely(__ebx_is_root(__ebx_untag(t, EB_SIDE_LEFT))))
		return NULL;

	t = __ebx_getroot(&(__ebx_untag(t, EB_SIDE_LEFT))->b[EB_SIDE_RGHT]);
	return __ebx_walk_down(t, EB_SIDE_LEFT);
}


/* This function is used to build a tree of duplicates by adding a new node to
 * a subtree of at least 2 entries. This is the version for end users, as the
 * internal functions use __ebx_insert_dup() instead. <root> is only used with
 * the re-entrant variants.
 */
static inline struct ebx_node *ebx_insert_dup(struct ebx_root *root,
                                              struct ebx_node *sub, struct ebx_node *new)
{
	return __ebx_insert_dup(sub, new);
}


/* Removes a leaf node from the tree if it was still in it. Marks the node
 * as unlinked. <root> is only used with the re-entrant variants.
 */
static forceinline void ebx_delete(struct ebx_root *root, struct ebx_node *node)
{
	__ebx_delete(node);
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 */
