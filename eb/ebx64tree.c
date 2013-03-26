/*
 * Elastic Binary Trees - exported functions for operations on 64bit nodes.
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

/* Consult eb64tree.h for more details about those functions */

#include "ebx64tree.h"

REGPRM2 struct ebx64_node *ebx64_insert(struct ebx_root *root, struct ebx64_node *new)
{
	return __ebx64_insert(root, new);
}

REGPRM2 struct ebx64_node *ebx64i_insert(struct ebx_root *root, struct ebx64_node *new)
{
	return __ebx64i_insert(root, new);
}

REGPRM2 struct ebx64_node *ebx64_lookup(struct ebx_root *root, u64 x)
{
	return __ebx64_lookup(root, x);
}

REGPRM2 struct ebx64_node *ebx64i_lookup(struct ebx_root *root, s64 x)
{
	return __ebx64i_lookup(root, x);
}

/*
 * Find the last occurrence of the highest key in the tree <root>, which is
 * equal to or less than <x>. NULL is returned is no key matches.
 */
REGPRM2 struct ebx64_node *ebx64_lookup_le(struct ebx_root *root, u64 x)
{
	struct ebx64_node *node;
	ebx_troot_t *troot;

	if (unlikely(ebx_link_is_null(root->b[EB_LEFT])))
		return NULL;

	troot = ebx_getroot(&root->b[EB_LEFT]);

	while (1) {
		if ((ebx_gettag(troot) == EB_LEAF)) {
			/* We reached a leaf, which means that the whole upper
			 * parts were common. We will return either the current
			 * node or its next one if the former is too small.
			 */
			node = container_of(ebx_untag(troot, EB_LEAF),
					    struct ebx64_node, node.branches);
			if (node->key <= x)
				return node;
			/* return prev */
			troot = ebx_getroot(&node->node.leaf_p);
			break;
		}
		node = container_of(ebx_untag(troot, EB_NODE),
				    struct ebx64_node, node.branches);

		if (node->node.bit < 0) {
			/* We're at the top of a dup tree. Either we got a
			 * matching value and we return the rightmost node, or
			 * we don't and we skip the whole subtree to return the
			 * prev node before the subtree. Note that since we're
			 * at the top of the dup tree, we can simply return the
			 * prev node without first trying to escape from the
			 * tree.
			 */
			if (node->key <= x) {
				troot = ebx_getroot(&node->node.branches.b[EB_RGHT]);
				while (ebx_gettag(troot) != EB_LEAF)
					troot = ebx_getroot(&(ebx_untag(troot, EB_NODE))->b[EB_RGHT]);
				return container_of(ebx_untag(troot, EB_LEAF),
						    struct ebx64_node, node.branches);
			}
			/* return prev */
			troot = ebx_getroot(&node->node.node_p);
			break;
		}

		if (((x ^ node->key) >> node->node.bit) >= EB_NODE_BRANCHES) {
			/* No more common bits at all. Either this node is too
			 * small and we need to get its highest value, or it is
			 * too large, and we need to get the prev value.
			 */
			if ((node->key >> node->node.bit) < (x >> node->node.bit)) {
				troot = ebx_getroot(&node->node.branches.b[EB_RGHT]);
				return ebx64_entry(ebx_walk_down(troot, EB_RGHT), struct ebx64_node, node);
			}

			/* Further values will be too high here, so return the prev
			 * unique node (if it exists).
			 */
			troot = ebx_getroot(&node->node.node_p);
			break;
		}
		troot = ebx_getroot(&node->node.branches.b[(x >> node->node.bit) & EB_NODE_BRANCH_MASK]);
	}

	/* If we get here, it means we want to report previous node before the
	 * current one which is not above. <troot> is already initialised to
	 * the parent's branches.
	 */
	while (ebx_gettag(troot) == EB_LEFT) {
		/* Walking up from left branch. We must ensure that we never
		 * walk beyond root.
		 */
		if (unlikely(ebx_link_is_null(ebx_untag(troot, EB_LEFT)->b[EB_RGHT])))
			return NULL;
		troot = ebx_getroot(&(ebx_root_to_node(ebx_untag(troot, EB_LEFT)))->node_p);
	}
	/* Note that <troot> cannot be NULL at this stage */
	troot = ebx_getroot(&(ebx_untag(troot, EB_RGHT))->b[EB_LEFT]);
	node = ebx64_entry(ebx_walk_down(troot, EB_RGHT), struct ebx64_node, node);
	return node;
}

/*
 * Find the first occurrence of the lowest key in the tree <root>, which is
 * equal to or greater than <x>. NULL is returned is no key matches.
 */
REGPRM2 struct ebx64_node *ebx64_lookup_ge(struct ebx_root *root, u64 x)
{
	struct ebx64_node *node;
	ebx_troot_t *troot;

	if (unlikely(ebx_link_is_null(root->b[EB_LEFT])))
		return NULL;

	troot = ebx_getroot(&root->b[EB_LEFT]);

	while (1) {
		if ((ebx_gettag(troot) == EB_LEAF)) {
			/* We reached a leaf, which means that the whole upper
			 * parts were common. We will return either the current
			 * node or its next one if the former is too small.
			 */
			node = container_of(ebx_untag(troot, EB_LEAF),
					    struct ebx64_node, node.branches);
			if (node->key >= x)
				return node;
			/* return next */
			troot = ebx_getroot(&node->node.leaf_p);
			break;
		}
		node = container_of(ebx_untag(troot, EB_NODE),
				    struct ebx64_node, node.branches);

		if (node->node.bit < 0) {
			/* We're at the top of a dup tree. Either we got a
			 * matching value and we return the leftmost node, or
			 * we don't and we skip the whole subtree to return the
			 * next node after the subtree. Note that since we're
			 * at the top of the dup tree, we can simply return the
			 * next node without first trying to escape from the
			 * tree.
			 */
			if (node->key >= x) {
				troot = ebx_getroot(&node->node.branches.b[EB_LEFT]);
				while (ebx_gettag(troot) != EB_LEAF)
					troot = ebx_getroot(&(ebx_untag(troot, EB_NODE))->b[EB_LEFT]);
				return container_of(ebx_untag(troot, EB_LEAF),
						    struct ebx64_node, node.branches);
			}
			/* return next */
			troot = ebx_getroot(&node->node.node_p);
			break;
		}

		if (((x ^ node->key) >> node->node.bit) >= EB_NODE_BRANCHES) {
			/* No more common bits at all. Either this node is too
			 * large and we need to get its lowest value, or it is too
			 * small, and we need to get the next value.
			 */
			if ((node->key >> node->node.bit) > (x >> node->node.bit)) {
				troot = ebx_getroot(&node->node.branches.b[EB_LEFT]);
				return ebx64_entry(ebx_walk_down(troot, EB_LEFT), struct ebx64_node, node);
			}

			/* Further values will be too low here, so return the next
			 * unique node (if it exists).
			 */
			troot = ebx_getroot(&node->node.node_p);
			break;
		}
		troot = ebx_getroot(&node->node.branches.b[(x >> node->node.bit) & EB_NODE_BRANCH_MASK]);
	}

	/* If we get here, it means we want to report next node after the
	 * current one which is not below. <troot> is already initialised
	 * to the parent's branches.
	 */
	while (ebx_gettag(troot) != EB_LEFT)
		/* Walking up from right branch, so we cannot be below root */
		troot = ebx_getroot(&(ebx_root_to_node(ebx_untag(troot, EB_RGHT)))->node_p);

	/* Note that <troot> cannot be NULL at this stage */
	if (ebx_link_is_null(ebx_untag(troot, EB_LEFT)->b[EB_RGHT]))
		return NULL;

	troot = ebx_getroot(&(ebx_untag(troot, EB_LEFT))->b[EB_RGHT]);
	node = ebx64_entry(ebx_walk_down(troot, EB_LEFT), struct ebx64_node, node);
	return node;
}
