/*
 * Elastic Binary Trees - macros and structures for operations on 32bit nodes.
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

#ifndef _EBX32TREE_H
#define _EBX32TREE_H

#include "ebxtree.h"


/* Return the structure of type <type> whose member <member> points to <ptr> */
#define ebx32_entry(ptr, type, member) container_of(ptr, type, member)

#define EB32_ROOT	EB_ROOT
#define EB32_TREE_HEAD	EB_TREE_HEAD

/* These types may sometimes already be defined */
typedef unsigned int u32;
typedef   signed int s32;

/* This structure carries a node, a leaf, and a key. It must start with the
 * ebx_node so that it can be cast into an ebx_node. We could also have put some
 * sort of transparent union here to reduce the indirection level, but the fact
 * is, the end user is not meant to manipulate internals, so this is pointless.
 */
struct ebx32_node {
	struct ebx_node node; /* the tree node, must be at the beginning */
	u32 key;
};

/*
 * Exported functions and macros.
 * Many of them are always inlined because they are extremely small, and
 * are generally called at most once or twice in a program.
 */

/* Return leftmost node in the tree, or NULL if none */
static inline struct ebx32_node *ebx32_first(struct ebx_root *root)
{
	return ebx32_entry(ebx_first(root), struct ebx32_node, node);
}

/* Return rightmost node in the tree, or NULL if none */
static inline struct ebx32_node *ebx32_last(struct ebx_root *root)
{
	return ebx32_entry(ebx_last(root), struct ebx32_node, node);
}

/* Return next node in the tree, or NULL if none */
static inline struct ebx32_node *ebx32_next(struct ebx32_node *eb32)
{
	return ebx32_entry(ebx_next(&eb32->node), struct ebx32_node, node);
}

/* Return previous node in the tree, or NULL if none */
static inline struct ebx32_node *ebx32_prev(struct ebx32_node *eb32)
{
	return ebx32_entry(ebx_prev(&eb32->node), struct ebx32_node, node);
}

/* Return next leaf node within a duplicate sub-tree, or NULL if none. */
static inline struct ebx32_node *ebx32_next_dup(struct ebx32_node *eb32)
{
	return ebx32_entry(ebx_next_dup(&eb32->node), struct ebx32_node, node);
}

/* Return previous leaf node within a duplicate sub-tree, or NULL if none. */
static inline struct ebx32_node *ebx32_prev_dup(struct ebx32_node *eb32)
{
	return ebx32_entry(ebx_prev_dup(&eb32->node), struct ebx32_node, node);
}

/* Return next node in the tree, skipping duplicates, or NULL if none */
static inline struct ebx32_node *ebx32_next_unique(struct ebx32_node *eb32)
{
	return ebx32_entry(ebx_next_unique(&eb32->node), struct ebx32_node, node);
}

/* Return previous node in the tree, skipping duplicates, or NULL if none */
static inline struct ebx32_node *ebx32_prev_unique(struct ebx32_node *eb32)
{
	return ebx32_entry(ebx_prev_unique(&eb32->node), struct ebx32_node, node);
}

/* Delete node from the tree if it was linked in. Mark the node unused. Note
 * that this function relies on a non-inlined generic function: ebx_delete.
 */
static inline void eb32_delete(struct ebx32_node *eb32)
{
	ebx_delete(&eb32->node);
}

/*
 * The following functions are not inlined by default. They are declared
 * in eb32tree.c, which simply relies on their inline version.
 */
REGPRM2 struct ebx32_node *eb32_lookup(struct ebx_root *root, u32 x);
REGPRM2 struct ebx32_node *eb32i_lookup(struct ebx_root *root, s32 x);
REGPRM2 struct ebx32_node *eb32_lookup_le(struct ebx_root *root, u32 x);
REGPRM2 struct ebx32_node *eb32_lookup_ge(struct ebx_root *root, u32 x);
REGPRM2 struct ebx32_node *eb32_insert(struct ebx_root *root, struct ebx32_node *new);
REGPRM2 struct ebx32_node *eb32i_insert(struct ebx_root *root, struct ebx32_node *new);

/*
 * The following functions are less likely to be used directly, because their
 * code is larger. The non-inlined version is preferred.
 */

/* Delete node from the tree if it was linked in. Mark the node unused. */
static forceinline void __ebx32_delete(struct ebx32_node *eb32)
{
	__ebx_delete(&eb32->node);
}

/*
 * Find the first occurence of a key in the tree <root>. If none can be
 * found, return NULL.
 */
static forceinline struct ebx32_node *__ebx32_lookup(struct ebx_root *root, u32 x)
{
	struct ebx32_node *node;
	ebx_troot_t *troot;
	u32 y;
	int node_bit;

	troot = ebx_getroot(&root->b[EB_LEFT]);
	if (unlikely(troot == NULL))
		return NULL;

	while (1) {
		if ((ebx_gettag(troot) == EB_LEAF)) {
			node = container_of(ebx_untag(troot, EB_LEAF),
					    struct ebx32_node, node.branches);
			if (node->key == x)
				return node;
			else
				return NULL;
		}
		node = container_of(ebx_untag(troot, EB_NODE),
				    struct ebx32_node, node.branches);
		node_bit = node->node.bit;

		y = node->key ^ x;
		if (!y) {
			/* Either we found the node which holds the key, or
			 * we have a dup tree. In the later case, we have to
			 * walk it down left to get the first entry.
			 */
			if (node_bit < 0) {
				troot = ebx_getroot(&node->node.branches.b[EB_LEFT]);
				while (ebx_gettag(troot) != EB_LEAF)
					troot = ebx_getroot(&(ebx_untag(troot, EB_NODE))->b[EB_LEFT]);
				node = container_of(ebx_untag(troot, EB_LEAF),
						    struct ebx32_node, node.branches);
			}
			return node;
		}

		if ((y >> node_bit) >= EB_NODE_BRANCHES)
			return NULL; /* no more common bits */

		troot = ebx_getroot(&node->node.branches.b[(x >> node_bit) & EB_NODE_BRANCH_MASK]);
	}
}

/*
 * Find the first occurence of a signed key in the tree <root>. If none can
 * be found, return NULL.
 */
static forceinline struct ebx32_node *__ebx32i_lookup(struct ebx_root *root, s32 x)
{
	struct ebx32_node *node;
	ebx_troot_t *troot;
	u32 key = x ^ 0x80000000;
	u32 y;
	int node_bit;

	troot = ebx_getroot(&root->b[EB_LEFT]);
	if (unlikely(troot == NULL))
		return NULL;

	while (1) {
		if ((ebx_gettag(troot) == EB_LEAF)) {
			node = container_of(ebx_untag(troot, EB_LEAF),
					    struct ebx32_node, node.branches);
			if (node->key == (u32)x)
				return node;
			else
				return NULL;
		}
		node = container_of(ebx_untag(troot, EB_NODE),
				    struct ebx32_node, node.branches);
		node_bit = node->node.bit;

		y = node->key ^ x;
		if (!y) {
			/* Either we found the node which holds the key, or
			 * we have a dup tree. In the later case, we have to
			 * walk it down left to get the first entry.
			 */
			if (node_bit < 0) {
				troot = ebx_getroot(&node->node.branches.b[EB_LEFT]);
				while (ebx_gettag(troot) != EB_LEAF)
					troot = ebx_getroot(&(ebx_untag(troot, EB_NODE))->b[EB_LEFT]);
				node = container_of(ebx_untag(troot, EB_LEAF),
						    struct ebx32_node, node.branches);
			}
			return node;
		}

		if ((y >> node_bit) >= EB_NODE_BRANCHES)
			return NULL; /* no more common bits */

		troot = ebx_getroot(&node->node.branches.b[(key >> node_bit) & EB_NODE_BRANCH_MASK]);
	}
}

/* Insert ebx32_node <new> into subtree starting at node root <root>.
 * Only new->key needs be set with the key. The ebx32_node is returned.
 * If root->b[EB_RGHT]==1, the tree may only contain unique keys.
 */
static forceinline struct ebx32_node *
__ebx32_insert(struct ebx_root *root, struct ebx32_node *new)
{
	struct ebx32_node *old;
	unsigned int side;
	ebx_troot_t *troot;
	ebx_link_t *up_ptr;
	u32 newkey; /* caching the key saves approximately one cycle */
	ebx_troot_t *root_right;
	ebx_troot_t *new_left, *new_rght;
	ebx_troot_t *new_leaf;
	int old_node_bit;

	side = EB_LEFT;
	troot = ebx_getroot(&root->b[EB_LEFT]);
	root_right = ebx_getroot(&root->b[EB_RGHT]);
	if (unlikely(troot == NULL)) {
		/* Tree is empty, insert the leaf part below the left branch */
		ebx_setlink(&root->b[EB_LEFT], ebx_dotag(&new->node.branches, EB_LEAF));
		ebx_setlink(&new->node.leaf_p, ebx_dotag(root, EB_LEFT));
		new->node.node_p = 0; /* node part unused */
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
	 * was attached. <newkey> carries the key being inserted.
	 */
	newkey = new->key;

	while (1) {
		if (ebx_gettag(troot) == EB_LEAF) {
			/* insert above a leaf */
			old = container_of(ebx_untag(troot, EB_LEAF),
					    struct ebx32_node, node.branches);
			ebx_setlink(&new->node.node_p, ebx_getroot(&old->node.leaf_p));
			up_ptr = &old->node.leaf_p;
			break;
		}

		/* OK we're walking down this link */
		old = container_of(ebx_untag(troot, EB_NODE),
				    struct ebx32_node, node.branches);
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
			ebx_setlink(&new->node.node_p, ebx_getroot(&old->node.node_p));
			up_ptr = &old->node.node_p;
			break;
		}

		/* walk down */
		root = &old->node.branches;
		side = (newkey >> old_node_bit) & EB_NODE_BRANCH_MASK;
		troot = ebx_getroot(&root->b[side]);
	}

	new_left = ebx_dotag(&new->node.branches, EB_LEFT);
	new_rght = ebx_dotag(&new->node.branches, EB_RGHT);
	new_leaf = ebx_dotag(&new->node.branches, EB_LEAF);

	/* We need the common higher bits between new->key and old->key.
	 * What differences are there between new->key and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of new->key and old->key are identical here (otherwise they
	 * would sit on different branches).
	 */

	/* note that if EB_NODE_BITS > 1, we should check that it's still >= 0 */
	new->node.bit = flsnz(new->key ^ old->key) - EB_NODE_BITS;

	if (new->key == old->key) {
		new->node.bit = -1; /* mark as new dup tree, just in case */

		if (likely(ebx_gettag(root_right))) {
			/* we refuse to duplicate this key if the tree is
			 * tagged as containing only unique keys.
			 */
			return old;
		}

		if (ebx_gettag(troot) != EB_LEAF) {
			/* there was already a dup tree below */
			struct ebx_node *ret;
			ret = ebx_insert_dup(&old->node, &new->node);
			return container_of(ret, struct ebx32_node, node);
		}
		/* otherwise fall through */
	}

	if (new->key >= old->key) {
		ebx_setlink(&new->node.branches.b[EB_LEFT], troot);
		ebx_setlink(&new->node.branches.b[EB_RGHT], new_leaf);
		ebx_setlink(&new->node.leaf_p, new_rght);
		ebx_setlink(up_ptr, new_left);
	}
	else {
		ebx_setlink(&new->node.branches.b[EB_LEFT], new_leaf);
		ebx_setlink(&new->node.branches.b[EB_RGHT], troot);
		ebx_setlink(&new->node.leaf_p, new_left);
		ebx_setlink(up_ptr, new_rght);
	}

	/* Ok, now we are inserting <new> between <root> and <old>. <old>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <side>. Update the root's leaf till we have it. Note that we can also
	 * find the side by checking the side of new->node.node_p.
	 */

	ebx_setlink(&root->b[side], ebx_dotag(&new->node.branches, EB_NODE));
	return new;
}

/* Insert ebx32_node <new> into subtree starting at node root <root>, using
 * signed keys. Only new->key needs be set with the key. The ebx32_node
 * is returned. If root->b[EB_RGHT]==1, the tree may only contain unique keys.
 */
static forceinline struct ebx32_node *
__ebx32i_insert(struct ebx_root *root, struct ebx32_node *new)
{
	struct ebx32_node *old;
	unsigned int side;
	ebx_troot_t *troot;
	ebx_link_t *up_ptr;
	int newkey; /* caching the key saves approximately one cycle */
	ebx_troot_t *root_right;
	ebx_troot_t *new_left, *new_rght;
	ebx_troot_t *new_leaf;
	int old_node_bit;

	side = EB_LEFT;
	troot = ebx_getroot(&root->b[EB_LEFT]);
	root_right = ebx_getroot(&root->b[EB_RGHT]);
	if (unlikely(troot == NULL)) {
		/* Tree is empty, insert the leaf part below the left branch */
		ebx_setlink(&root->b[EB_LEFT], ebx_dotag(&new->node.branches, EB_LEAF));
		ebx_setlink(&new->node.leaf_p, ebx_dotag(root, EB_LEFT));
		new->node.node_p = 0; /* node part unused */
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
	 * was attached. <newkey> carries a high bit shift of the key being
	 * inserted in order to have negative keys stored before positive
	 * ones.
	 */
	newkey = new->key + 0x80000000;

	while (1) {
		if (ebx_gettag(troot) == EB_LEAF) {
			old = container_of(ebx_untag(troot, EB_LEAF),
					    struct ebx32_node, node.branches);
			ebx_setlink(&new->node.node_p, ebx_getroot(&old->node.leaf_p));
			up_ptr = &old->node.leaf_p;
			break;
		}

		/* OK we're walking down this link */
		old = container_of(ebx_untag(troot, EB_NODE),
				    struct ebx32_node, node.branches);
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
			ebx_setlink(&new->node.node_p, ebx_getroot(&old->node.node_p));
			up_ptr = &old->node.node_p;
			break;
		}

		/* walk down */
		root = &old->node.branches;
		side = (newkey >> old_node_bit) & EB_NODE_BRANCH_MASK;
		troot = ebx_getroot(&root->b[side]);
	}

	new_left = ebx_dotag(&new->node.branches, EB_LEFT);
	new_rght = ebx_dotag(&new->node.branches, EB_RGHT);
	new_leaf = ebx_dotag(&new->node.branches, EB_LEAF);

	/* We need the common higher bits between new->key and old->key.
	 * What differences are there between new->key and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of new->key and old->key are identical here (otherwise they
	 * would sit on different branches).
	 */

	/* note that if EB_NODE_BITS > 1, we should check that it's still >= 0 */
	new->node.bit = flsnz(new->key ^ old->key) - EB_NODE_BITS;

	if (new->key == old->key) {
		new->node.bit = -1; /* mark as new dup tree, just in case */

		if (likely(ebx_gettag(root_right))) {
			/* we refuse to duplicate this key if the tree is
			 * tagged as containing only unique keys.
			 */
			return old;
		}

		if (ebx_gettag(troot) != EB_LEAF) {
			/* there was already a dup tree below */
			struct ebx_node *ret;
			ret = ebx_insert_dup(&old->node, &new->node);
			return container_of(ret, struct ebx32_node, node);
		}
		/* otherwise fall through */
	}

	if ((s32)new->key >= (s32)old->key) {
		ebx_setlink(&new->node.branches.b[EB_LEFT], troot);
		ebx_setlink(&new->node.branches.b[EB_RGHT], new_leaf);
		ebx_setlink(&new->node.leaf_p, new_rght);
		ebx_setlink(up_ptr, new_left);
	}
	else {
		ebx_setlink(&new->node.branches.b[EB_LEFT], new_leaf);
		ebx_setlink(&new->node.branches.b[EB_RGHT], troot);
		ebx_setlink(&new->node.leaf_p, new_left);
		ebx_setlink(up_ptr, new_rght);
	}

	/* Ok, now we are inserting <new> between <root> and <old>. <old>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <side>. Update the root's leaf till we have it. Note that we can also
	 * find the side by checking the side of new->node.node_p.
	 */

	ebx_setlink(&root->b[side], ebx_dotag(&new->node.branches, EB_NODE));
	return new;
}

#endif /* _EBX32_TREE_H */
