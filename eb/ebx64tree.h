/*
 * Elastic Binary Trees - macros and structures for operations on 64bit nodes.
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

/* This structure carries a node, a leaf, and a key. It must start with the
 * ebx_node so that it can be cast into an ebx_node. We could also have put some
 * sort of transparent union here to reduce the indirection level, but the fact
 * is, the end user is not meant to manipulate internals, so this is pointless.
 */
struct ebx64_node {
	struct ebx_node node; /* the tree node, must be at the beginning */
	u64 key;
};

/*
 * Exported functions and macros.
 * Many of them are always inlined because they are extremely small, and
 * are generally called at most once or twice in a program.
 */

/* Return leftmost node in the tree, or NULL if none */
static inline struct ebx64_node *ebx64_first(struct ebx_root *root)
{
	return eb_entry(ebx_first(root), struct ebx64_node, node);
}

/* Return rightmost node in the tree, or NULL if none */
static inline struct ebx64_node *ebx64_last(struct ebx_root *root)
{
	return eb_entry(ebx_last(root), struct ebx64_node, node);
}

/* Return next node in the tree, or NULL if none */
static inline struct ebx64_node *ebx64_next(struct ebx64_node *eb64)
{
	return eb_entry(ebx_next(&eb64->node), struct ebx64_node, node);
}

/* Return previous node in the tree, or NULL if none */
static inline struct ebx64_node *ebx64_prev(struct ebx64_node *eb64)
{
	return eb_entry(ebx_prev(&eb64->node), struct ebx64_node, node);
}

/* Return next leaf node within a duplicate sub-tree, or NULL if none. */
static inline struct ebx64_node *ebx64_next_dup(struct ebx64_node *eb64)
{
	return eb_entry(ebx_next_dup(&eb64->node), struct ebx64_node, node);
}

/* Return previous leaf node within a duplicate sub-tree, or NULL if none. */
static inline struct ebx64_node *ebx64_prev_dup(struct ebx64_node *eb64)
{
	return eb_entry(ebx_prev_dup(&eb64->node), struct ebx64_node, node);
}

/* Return next node in the tree, skipping duplicates, or NULL if none */
static inline struct ebx64_node *ebx64_next_unique(struct ebx64_node *eb64)
{
	return eb_entry(ebx_next_unique(&eb64->node), struct ebx64_node, node);
}

/* Return previous node in the tree, skipping duplicates, or NULL if none */
static inline struct ebx64_node *ebx64_prev_unique(struct ebx64_node *eb64)
{
	return eb_entry(ebx_prev_unique(&eb64->node), struct ebx64_node, node);
}

/* Delete node from the tree if it was linked in. Mark the node unused. The
 * tree's root is only used with the re-entrant variants.
 */
static inline void ebx64_delete(struct ebx_root *root, struct ebx64_node *eb64)
{
	ebx_delete(root, &eb64->node);
}

/*
 * The following functions are not inlined by default. They are declared
 * in eb64tree.c, which simply relies on their inline version.
 */
REGPRM2 struct ebx64_node *ebx64_lookup(struct ebx_root *root, u64 x);
REGPRM2 struct ebx64_node *ebx64i_lookup(struct ebx_root *root, s64 x);
REGPRM2 struct ebx64_node *ebx64_lookup_le(struct ebx_root *root, u64 x);
REGPRM2 struct ebx64_node *ebx64_lookup_ge(struct ebx_root *root, u64 x);
REGPRM2 struct ebx64_node *ebx64_insert(struct ebx_root *root, struct ebx64_node *new);
REGPRM2 struct ebx64_node *ebx64i_insert(struct ebx_root *root, struct ebx64_node *new);

/*
 * The following functions are less likely to be used directly, because their
 * code is larger. The non-inlined version is preferred.
 */

/* Delete node from the tree if it was linked in. Mark the node unused. This
 * version is not re-entrant.
 */
static forceinline void __ebx64_delete(struct ebx64_node *eb64)
{
	__ebx_delete(&eb64->node);
}

/*
 * Find the first occurence of a key in the tree <root>. If none can be
 * found, return NULL.
 */
static forceinline struct ebx64_node *__ebx64_lookup(struct ebx_root *root, u64 x)
{
	struct ebx64_node *node;
	ebx_troot_t *troot;
	u64 y;

	if (unlikely(__ebx_link_is_null(root->b[EB_SIDE_LEFT])))
		return NULL;

	troot = __ebx_getroot(&root->b[EB_SIDE_LEFT]);

	while (1) {
		if ((__ebx_get_branch_type(troot) == EB_TYPE_LEAF)) {
			node = container_of(__ebx_untag(troot, EB_TYPE_LEAF),
					    struct ebx64_node, node.branches);
			if (node->key == x)
				return node;
			else
				return NULL;
		}
		node = container_of(__ebx_untag(troot, EB_TYPE_NODE),
				    struct ebx64_node, node.branches);

		y = node->key ^ x;
		if (!y) {
			/* Either we found the node which holds the key, or
			 * we have a dup tree. In the later case, we have to
			 * walk it down left to get the first entry.
			 */
			if (__ebx_is_dup(&node->node)) {
				troot = __ebx_getroot(&node->node.branches.b[EB_SIDE_LEFT]);
				while (__ebx_get_branch_type(troot) != EB_TYPE_LEAF)
					troot = __ebx_getroot(&(__ebx_untag(troot, EB_TYPE_NODE))->b[EB_SIDE_LEFT]);
				node = container_of(__ebx_untag(troot, EB_TYPE_LEAF),
						    struct ebx64_node, node.branches);
			}
			return node;
		}

		if ((y >> node->node.bit) >= EB_NODE_BRANCHES)
			return NULL; /* no more common bits */

		troot = __ebx_getroot(&node->node.branches.b[(x >> node->node.bit) & EB_NODE_BRANCH_MASK]);
	}
}

/*
 * Find the first occurence of a signed key in the tree <root>. If none can
 * be found, return NULL.
 */
static forceinline struct ebx64_node *__ebx64i_lookup(struct ebx_root *root, s64 x)
{
	struct ebx64_node *node;
	ebx_troot_t *troot;
	u64 key = x ^ (1ULL << 63);
	u64 y;

	if (unlikely(__ebx_link_is_null(root->b[EB_SIDE_LEFT])))
		return NULL;

	troot = __ebx_getroot(&root->b[EB_SIDE_LEFT]);

	while (1) {
		if ((__ebx_get_branch_type(troot) == EB_TYPE_LEAF)) {
			node = container_of(__ebx_untag(troot, EB_TYPE_LEAF),
					    struct ebx64_node, node.branches);
			if (node->key == (u64)x)
				return node;
			else
				return NULL;
		}
		node = container_of(__ebx_untag(troot, EB_TYPE_NODE),
				    struct ebx64_node, node.branches);

		y = node->key ^ x;
		if (!y) {
			/* Either we found the node which holds the key, or
			 * we have a dup tree. In the later case, we have to
			 * walk it down left to get the first entry.
			 */
			if (__ebx_is_dup(&node->node)) {
				troot = __ebx_getroot(&node->node.branches.b[EB_SIDE_LEFT]);
				while (__ebx_get_branch_type(troot) != EB_TYPE_LEAF)
					troot = __ebx_getroot(&(__ebx_untag(troot, EB_TYPE_NODE))->b[EB_SIDE_LEFT]);
				node = container_of(__ebx_untag(troot, EB_TYPE_LEAF),
						    struct ebx64_node, node.branches);
			}
			return node;
		}

		if ((y >> node->node.bit) >= EB_NODE_BRANCHES)
			return NULL; /* no more common bits */

		troot = __ebx_getroot(&node->node.branches.b[(key >> node->node.bit) & EB_NODE_BRANCH_MASK]);
	}
}

/* Insert ebx64_node <new> into subtree starting at node root <root>.
 * Only new->key needs be set with the key. The ebx64_node is returned.
 * If root->b[EB_SIDE_RGHT]==1, the tree may only contain unique keys.
 */
static forceinline struct ebx64_node *
__ebx64_insert(struct ebx_root *root, struct ebx64_node *new) {
	struct ebx64_node *old;
	unsigned int side;
	ebx_troot_t *troot;
	u64 newkey; /* caching the key saves approximately one cycle */
	ebx_ulink_t root_flags;
	int old_node_bit;

	side = EB_SIDE_LEFT;
	if (unlikely(__ebx_link_is_null(root->b[EB_SIDE_LEFT]))) {
		/* Tree is empty, insert the leaf part below the left branch */
		__ebx_setlink(&root->b[EB_SIDE_LEFT], __ebx_dotag(&new->node.branches, EB_TYPE_LEAF));
		__ebx_setlink(&new->node.leaf_p, __ebx_dotag(root, EB_SIDE_LEFT));
		new->node.node_p = 0; /* node part unused */
		return new;
	}
	troot = __ebx_getroot(&root->b[EB_SIDE_LEFT]);
	root_flags = __ebx_get_root_flags(root);

	/* The tree descent is fairly easy :
	 *  - first, check if we have reached a leaf node
	 *  - second, check if we have gone too far
	 *  - third, reiterate
	 * Everywhere, we use <new> for the node node we are inserting, <root>
	 * for the node we attach it to, and <old> for the node we are
	 * displacing below <new>. <troot> will always point to the future node
	 * (tagged with its type). <side> carries the side the node <new> is
	 * attached to below its parent, which is also where previous node
	 * was attached. <newkey> carries the key being inserted.
	 */
	newkey = new->key;

	while (1) {
		if (unlikely(__ebx_get_branch_type(troot) == EB_TYPE_LEAF)) {
			ebx_troot_t *new_left, *new_rght;
			ebx_troot_t *new_leaf, *old_leaf;

			old = container_of(__ebx_untag(troot, EB_TYPE_LEAF),
					    struct ebx64_node, node.branches);

			new_left = __ebx_dotag(&new->node.branches, EB_SIDE_LEFT);
			new_rght = __ebx_dotag(&new->node.branches, EB_SIDE_RGHT);
			new_leaf = __ebx_dotag(&new->node.branches, EB_TYPE_LEAF);
			old_leaf = __ebx_dotag(&old->node.branches, EB_TYPE_LEAF);

			__ebx_setlink(&new->node.node_p, __ebx_getroot(&old->node.leaf_p));

			/* Right here, we have 3 possibilities :
			   - the tree does not contain the key, and we have
			     new->key < old->key. We insert new above old, on
			     the left ;

			   - the tree does not contain the key, and we have
			     new->key > old->key. We insert new above old, on
			     the right ;

			   - the tree does contain the key, which implies it
			     is alone. We add the new key next to it as a
			     first duplicate.

			   The last two cases can easily be partially merged.
			*/
			 
			if (new->key < old->key) {
				__ebx_setlink(&new->node.leaf_p, new_left);
				__ebx_setlink(&old->node.leaf_p, new_rght);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_LEFT], new_leaf);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_RGHT], old_leaf);
			} else {
				/* we may refuse to duplicate this key if the tree is
				 * tagged as containing only unique keys.
				 */
				if ((new->key == old->key) && (root_flags & EB_UNIQUE))
					return old;

				/* new->key >= old->key, new goes the right */
				__ebx_setlink(&old->node.leaf_p, new_left);
				__ebx_setlink(&new->node.leaf_p, new_rght);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_LEFT], old_leaf);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_RGHT], new_leaf);

				if (new->key == old->key) {
					new->node.bit = -1;
					__ebx_setlink(&root->b[side], __ebx_dotag(&new->node.branches, EB_TYPE_NODE));
					return new;
				}
			}
			break;
		}

		/* OK we're walking down this link */
		old = container_of(__ebx_untag(troot, EB_TYPE_NODE),
				    struct ebx64_node, node.branches);
		old_node_bit = old->node.bit;

		/* Stop going down when we don't have common bits anymore. We
		 * also stop in front of a duplicates tree because it means we
		 * have to insert above.
		 */

		if ((old_node_bit < 0) || /* we're above a duplicate tree, stop here */
		    (((new->key ^ old->key) >> old_node_bit) >= EB_NODE_BRANCHES)) {
			/* The tree did not contain the key, so we insert <new> before the node
			 * <old>, and set ->bit to designate the lowest bit position in <new>
			 * which applies to ->branches.b[].
			 */
			ebx_troot_t *new_left, *new_rght;
			ebx_troot_t *new_leaf, *old_node;

			new_left = __ebx_dotag(&new->node.branches, EB_SIDE_LEFT);
			new_rght = __ebx_dotag(&new->node.branches, EB_SIDE_RGHT);
			new_leaf = __ebx_dotag(&new->node.branches, EB_TYPE_LEAF);
			old_node = __ebx_dotag(&old->node.branches, EB_TYPE_NODE);

			__ebx_setlink(&new->node.node_p, __ebx_getroot(&old->node.node_p));

			if (new->key < old->key) {
				__ebx_setlink(&new->node.leaf_p, new_left);
				__ebx_setlink(&old->node.node_p, new_rght);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_LEFT], new_leaf);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_RGHT], old_node);
			}
			else if (new->key > old->key) {
				__ebx_setlink(&old->node.node_p, new_left);
				__ebx_setlink(&new->node.leaf_p, new_rght);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_LEFT], old_node);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_RGHT], new_leaf);
			}
			else {
				struct ebx_node *ret;
				ret = __ebx_insert_dup(&old->node, &new->node);
				return container_of(ret, struct ebx64_node, node);
			}
			break;
		}

		/* walk down */
		root = &old->node.branches;
#if BITS_PER_LONG >= 64
		side = (newkey >> old_node_bit) & EB_NODE_BRANCH_MASK;
#else
		side = newkey;
		side >>= old_node_bit;
		if (old_node_bit >= 32) {
			side = newkey >> 32;
			side >>= old_node_bit & 0x1F;
		}
		side &= EB_NODE_BRANCH_MASK;
#endif
		troot = __ebx_getroot(&root->b[side]);
	}

	/* Ok, now we are inserting <new> between <root> and <old>. <old>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <side>. Update the root's leaf till we have it. Note that we can also
	 * find the side by checking the side of new->node.node_p.
	 */

	/* We need the common higher bits between new->key and old->key.
	 * What differences are there between new->key and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of new->key and old->key are identical here (otherwise they
	 * would sit on different branches).
	 */
	/* note that if EB_NODE_BITS > 1, we should check that it's still >= 0 */
	new->node.bit = flsnz(new->key ^ old->key) - EB_NODE_BITS;
	__ebx_setlink(&root->b[side], __ebx_dotag(&new->node.branches, EB_TYPE_NODE));

	return new;
}

/* Insert ebx64_node <new> into subtree starting at node root <root>, using
 * signed keys. Only new->key needs be set with the key. The ebx64_node
 * is returned. If root->b[EB_SIDE_RGHT]==1, the tree may only contain unique keys.
 */
static forceinline struct ebx64_node *
__ebx64i_insert(struct ebx_root *root, struct ebx64_node *new) {
	struct ebx64_node *old;
	unsigned int side;
	ebx_troot_t *troot;
	u64 newkey; /* caching the key saves approximately one cycle */
	ebx_ulink_t root_flags;
	int old_node_bit;

	side = EB_SIDE_LEFT;
	if (unlikely(__ebx_link_is_null(root->b[EB_SIDE_LEFT]))) {
		/* Tree is empty, insert the leaf part below the left branch */
		__ebx_setlink(&root->b[EB_SIDE_LEFT], __ebx_dotag(&new->node.branches, EB_TYPE_LEAF));
		__ebx_setlink(&new->node.leaf_p, __ebx_dotag(root, EB_SIDE_LEFT));
		new->node.node_p = 0; /* node part unused */
		return new;
	}
	troot = __ebx_getroot(&root->b[EB_SIDE_LEFT]);
	root_flags = __ebx_get_root_flags(root);

	/* The tree descent is fairly easy :
	 *  - first, check if we have reached a leaf node
	 *  - second, check if we have gone too far
	 *  - third, reiterate
	 * Everywhere, we use <new> for the node node we are inserting, <root>
	 * for the node we attach it to, and <old> for the node we are
	 * displacing below <new>. <troot> will always point to the future node
	 * (tagged with its type). <side> carries the side the node <new> is
	 * attached to below its parent, which is also where previous node
	 * was attached. <newkey> carries a high bit shift of the key being
	 * inserted in order to have negative keys stored before positive
	 * ones.
	 */
	newkey = new->key ^ (1ULL << 63);

	while (1) {
		if (unlikely(__ebx_get_branch_type(troot) == EB_TYPE_LEAF)) {
			ebx_troot_t *new_left, *new_rght;
			ebx_troot_t *new_leaf, *old_leaf;

			old = container_of(__ebx_untag(troot, EB_TYPE_LEAF),
					    struct ebx64_node, node.branches);

			new_left = __ebx_dotag(&new->node.branches, EB_SIDE_LEFT);
			new_rght = __ebx_dotag(&new->node.branches, EB_SIDE_RGHT);
			new_leaf = __ebx_dotag(&new->node.branches, EB_TYPE_LEAF);
			old_leaf = __ebx_dotag(&old->node.branches, EB_TYPE_LEAF);

			__ebx_setlink(&new->node.node_p, __ebx_getroot(&old->node.leaf_p));

			/* Right here, we have 3 possibilities :
			   - the tree does not contain the key, and we have
			     new->key < old->key. We insert new above old, on
			     the left ;

			   - the tree does not contain the key, and we have
			     new->key > old->key. We insert new above old, on
			     the right ;

			   - the tree does contain the key, which implies it
			     is alone. We add the new key next to it as a
			     first duplicate.

			   The last two cases can easily be partially merged.
			*/
			 
			if ((s64)new->key < (s64)old->key) {
				__ebx_setlink(&new->node.leaf_p, new_left);
				__ebx_setlink(&old->node.leaf_p, new_rght);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_LEFT], new_leaf);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_RGHT], old_leaf);
			} else {
				/* we may refuse to duplicate this key if the tree is
				 * tagged as containing only unique keys.
				 */
				if ((new->key == old->key) && (root_flags & EB_UNIQUE))
					return old;

				/* new->key >= old->key, new goes the right */
				__ebx_setlink(&old->node.leaf_p, new_left);
				__ebx_setlink(&new->node.leaf_p, new_rght);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_LEFT], old_leaf);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_RGHT], new_leaf);

				if (new->key == old->key) {
					new->node.bit = -1;
					__ebx_setlink(&root->b[side], __ebx_dotag(&new->node.branches, EB_TYPE_NODE));
					return new;
				}
			}
			break;
		}

		/* OK we're walking down this link */
		old = container_of(__ebx_untag(troot, EB_TYPE_NODE),
				    struct ebx64_node, node.branches);
		old_node_bit = old->node.bit;

		/* Stop going down when we don't have common bits anymore. We
		 * also stop in front of a duplicates tree because it means we
		 * have to insert above.
		 */

		if ((old_node_bit < 0) || /* we're above a duplicate tree, stop here */
		    (((new->key ^ old->key) >> old_node_bit) >= EB_NODE_BRANCHES)) {
			/* The tree did not contain the key, so we insert <new> before the node
			 * <old>, and set ->bit to designate the lowest bit position in <new>
			 * which applies to ->branches.b[].
			 */
			ebx_troot_t *new_left, *new_rght;
			ebx_troot_t *new_leaf, *old_node;

			new_left = __ebx_dotag(&new->node.branches, EB_SIDE_LEFT);
			new_rght = __ebx_dotag(&new->node.branches, EB_SIDE_RGHT);
			new_leaf = __ebx_dotag(&new->node.branches, EB_TYPE_LEAF);
			old_node = __ebx_dotag(&old->node.branches, EB_TYPE_NODE);

			__ebx_setlink(&new->node.node_p, __ebx_getroot(&old->node.node_p));

			if ((s64)new->key < (s64)old->key) {
				__ebx_setlink(&new->node.leaf_p, new_left);
				__ebx_setlink(&old->node.node_p, new_rght);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_LEFT], new_leaf);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_RGHT], old_node);
			}
			else if ((s64)new->key > (s64)old->key) {
				__ebx_setlink(&old->node.node_p, new_left);
				__ebx_setlink(&new->node.leaf_p, new_rght);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_LEFT], old_node);
				__ebx_setlink(&new->node.branches.b[EB_SIDE_RGHT], new_leaf);
			}
			else {
				struct ebx_node *ret;
				ret = __ebx_insert_dup(&old->node, &new->node);
				return container_of(ret, struct ebx64_node, node);
			}
			break;
		}

		/* walk down */
		root = &old->node.branches;
#if BITS_PER_LONG >= 64
		side = (newkey >> old_node_bit) & EB_NODE_BRANCH_MASK;
#else
		side = newkey;
		side >>= old_node_bit;
		if (old_node_bit >= 32) {
			side = newkey >> 32;
			side >>= old_node_bit & 0x1F;
		}
		side &= EB_NODE_BRANCH_MASK;
#endif
		troot = __ebx_getroot(&root->b[side]);
	}

	/* Ok, now we are inserting <new> between <root> and <old>. <old>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <side>. Update the root's leaf till we have it. Note that we can also
	 * find the side by checking the side of new->node.node_p.
	 */

	/* We need the common higher bits between new->key and old->key.
	 * What differences are there between new->key and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of new->key and old->key are identical here (otherwise they
	 * would sit on different branches).
	 */
	/* note that if EB_NODE_BITS > 1, we should check that it's still >= 0 */
	new->node.bit = flsnz(new->key ^ old->key) - EB_NODE_BITS;
	__ebx_setlink(&root->b[side], __ebx_dotag(&new->node.branches, EB_TYPE_NODE));

	return new;
}
