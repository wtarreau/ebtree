/*
 * Elastic Binary Trees - macros to manipulate Indirect String data nodes.
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

/* These functions and macros rely on Multi-Byte nodes */

#ifndef _EBXISTREE_H
#define _EBXISTREE_H

#include <string.h>
#include "ebxtree.h"
#include "ebxpttree.h"
#include "ebximtree.h"

/* These functions and macros rely on Pointer nodes and use the <key> entry as
 * a pointer to an indirect key. Most operations are performed using ebxpt_*.
 */

/* The following functions are not inlined by default. They are declared
 * in ebistree.c, which simply relies on their inline version.
 */
REGPRM2 struct ebxpt_node *ebis_lookup(struct ebx_root *root, const char *x);
REGPRM2 struct ebxpt_node *ebis_insert(struct ebx_root *root, struct ebxpt_node *new);

/* Find the first occurence of a length <len> string <x> in the tree <root>.
 * It's the caller's reponsibility to use this function only on trees which
 * only contain zero-terminated strings, and that no null character is present
 * in string <x> in the first <len> chars. If none can be found, return NULL.
 */
static forceinline struct ebxpt_node *
ebis_lookup_len(struct ebx_root *root, const char *x, unsigned int len)
{
	struct ebxpt_node *node;

	node = ebim_lookup(root, x, len);
	if (!node || ((const char *)node->key)[len] != 0)
		return NULL;
	return node;
}

/* Find the first occurence of a zero-terminated string <x> in the tree <root>.
 * It's the caller's reponsibility to use this function only on trees which
 * only contain zero-terminated strings. If none can be found, return NULL.
 */
static forceinline struct ebxpt_node *__ebis_lookup(struct ebx_root *root, const void *x)
{
	struct ebxpt_node *node;
	ebx_troot_t *troot;
	int bit;
	int node_bit;

	troot = ebx_getroot(&root->b[EB_LEFT]);
	if (unlikely(troot == NULL))
		return NULL;

	bit = 0;
	while (1) {
		if ((ebx_gettag(troot) == EB_LEAF)) {
			node = container_of(ebx_untag(troot, EB_LEAF),
					    struct ebxpt_node, node.branches);
			if (strcmp(node->key, x) == 0)
				return node;
			else
				return NULL;
		}
		node = container_of(ebx_untag(troot, EB_NODE),
				    struct ebxpt_node, node.branches);
		node_bit = node->node.bit;

		if (node_bit < 0) {
			/* We have a dup tree now. Either it's for the same
			 * value, and we walk down left, or it's a different
			 * one and we don't have our key.
			 */
			if (strcmp(node->key, x) != 0)
				return NULL;

			troot = ebx_getroot(&node->node.branches.b[EB_LEFT]);
			while (ebx_gettag(troot) != EB_LEAF)
				troot = ebx_getroot(&(ebx_untag(troot, EB_NODE))->b[EB_LEFT]);
			node = container_of(ebx_untag(troot, EB_LEAF),
					    struct ebxpt_node, node.branches);
			return node;
		}

		/* OK, normal data node, let's walk down but don't compare data
		 * if we already reached the end of the key.
		 */
		if (likely(bit >= 0)) {
			bit = string_equal_bits(x, node->key, bit);
			if (likely(bit < node_bit)) {
				if (bit >= 0)
					return NULL; /* no more common bits */

				/* bit < 0 : we reached the end of the key. If we
				 * are in a tree with unique keys, we can return
				 * this node. Otherwise we have to walk it down
				 * and stop comparing bits.
				 */
				if (ebx_gettag(ebx_getroot(&root->b[EB_RGHT])))
					return node;
			}
			/* if the bit is larger than the node's, we must bound it
			 * because we might have compared too many bytes with an
			 * inappropriate leaf. For a test, build a tree from "0",
			 * "WW", "W", "S" inserted in this exact sequence and lookup
			 * "W" => "S" is returned without this assignment.
			 */
			else
				bit = node_bit;
		}

		troot = ebx_getroot(&node->node.branches.b[(((unsigned char*)x)[node_bit >> 3] >>
							  (~node_bit & 7)) & 1]);
	}
}

/* Insert ebxpt_node <new> into subtree starting at node root <root>. Only
 * new->key needs be set with the zero-terminated string key. The ebxpt_node is
 * returned. If root->b[EB_RGHT]==1, the tree may only contain unique keys. The
 * caller is responsible for properly terminating the key with a zero.
 */
static forceinline struct ebxpt_node *
__ebis_insert(struct ebx_root *root, struct ebxpt_node *new)
{
	struct ebxpt_node *old;
	unsigned int side;
	ebx_troot_t *troot;
	ebx_troot_t *root_right;
	int diff;
	int bit;
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
	 * was attached.
	 */

	bit = 0;
	while (1) {
		if (unlikely(ebx_gettag(troot) == EB_LEAF)) {
			ebx_troot_t *new_left, *new_rght;
			ebx_troot_t *new_leaf, *old_leaf;

			old = container_of(ebx_untag(troot, EB_LEAF),
					    struct ebxpt_node, node.branches);

			new_left = ebx_dotag(&new->node.branches, EB_LEFT);
			new_rght = ebx_dotag(&new->node.branches, EB_RGHT);
			new_leaf = ebx_dotag(&new->node.branches, EB_LEAF);
			old_leaf = ebx_dotag(&old->node.branches, EB_LEAF);

			ebx_setlink(&new->node.node_p, ebx_getroot(&old->node.leaf_p));

			/* Right here, we have 3 possibilities :
			 * - the tree does not contain the key, and we have
			 *   new->key < old->key. We insert new above old, on
			 *   the left ;
			 *
			 * - the tree does not contain the key, and we have
			 *   new->key > old->key. We insert new above old, on
			 *   the right ;
			 *
			 * - the tree does contain the key, which implies it
			 *   is alone. We add the new key next to it as a
			 *   first duplicate.
			 *
			 * The last two cases can easily be partially merged.
			 */
			if (bit >= 0)
				bit = string_equal_bits(new->key, old->key, bit);

			if (bit < 0) {
				/* key was already there */

				/* we may refuse to duplicate this key if the tree is
				 * tagged as containing only unique keys.
				 */
				if (ebx_gettag(root_right))
					return old;

				/* new arbitrarily goes to the right and tops the dup tree */
				ebx_setlink(&old->node.leaf_p, new_left);
				ebx_setlink(&new->node.leaf_p, new_rght);
				ebx_setlink(&new->node.branches.b[EB_LEFT], old_leaf);
				ebx_setlink(&new->node.branches.b[EB_RGHT], new_leaf);
				new->node.bit = -1;
				ebx_setlink(&root->b[side], ebx_dotag(&new->node.branches, EB_NODE));
				return new;
			}

			diff = cmp_bits(new->key, old->key, bit);
			if (diff < 0) {
				/* new->key < old->key, new takes the left */
				ebx_setlink(&new->node.leaf_p, new_left);
				ebx_setlink(&old->node.leaf_p, new_rght);
				ebx_setlink(&new->node.branches.b[EB_LEFT], new_leaf);
				ebx_setlink(&new->node.branches.b[EB_RGHT], old_leaf);
			} else {
				/* new->key > old->key, new takes the right */
				ebx_setlink(&old->node.leaf_p, new_left);
				ebx_setlink(&new->node.leaf_p, new_rght);
				ebx_setlink(&new->node.branches.b[EB_LEFT], old_leaf);
				ebx_setlink(&new->node.branches.b[EB_RGHT], new_leaf);
			}
			break;
		}

		/* OK we're walking down this link */
		old = container_of(ebx_untag(troot, EB_NODE),
				   struct ebxpt_node, node.branches);
		old_node_bit = old->node.bit;

		/* Stop going down when we don't have common bits anymore. We
		 * also stop in front of a duplicates tree because it means we
		 * have to insert above. Note: we can compare more bits than
		 * the current node's because as long as they are identical, we
		 * know we descend along the correct side.
		 */
		if (bit >= 0 && (bit < old_node_bit || old_node_bit < 0))
			bit = string_equal_bits(new->key, old->key, bit);

		if (unlikely(bit < 0)) {
			/* Perfect match, we must only stop on head of dup tree
			 * or walk down to a leaf.
			 */
			if (old_node_bit < 0) {
				/* We know here that string_equal_bits matched all
				 * bits and that we're on top of a dup tree, then
				 * we can perform the dup insertion and return.
				 */
				struct ebx_node *ret;
				ret = ebx_insert_dup(&old->node, &new->node);
				return container_of(ret, struct ebxpt_node, node);
			}
			/* OK so let's walk down */
		}
		else if (bit < old_node_bit || old_node_bit < 0) {
			/* The tree did not contain the key, or we stopped on top of a dup
			 * tree, possibly containing the key. In the former case, we insert
			 * <new> before the node <old>, and set ->bit to designate the lowest
			 * bit position in <new> which applies to ->branches.b[]. In the later
			 * case, we add the key to the existing dup tree. Note that we cannot
			 * enter here if we match an intermediate node's key that is not the
			 * head of a dup tree.
			 */
			ebx_troot_t *new_left, *new_rght;
			ebx_troot_t *new_leaf, *old_node;

			new_left = ebx_dotag(&new->node.branches, EB_LEFT);
			new_rght = ebx_dotag(&new->node.branches, EB_RGHT);
			new_leaf = ebx_dotag(&new->node.branches, EB_LEAF);
			old_node = ebx_dotag(&old->node.branches, EB_NODE);

			ebx_setlink(&new->node.node_p, ebx_getroot(&old->node.node_p));

			/* we can never match all bits here */
			diff = cmp_bits(new->key, old->key, bit);
			if (diff < 0) {
				ebx_setlink(&new->node.leaf_p, new_left);
				ebx_setlink(&old->node.node_p, new_rght);
				ebx_setlink(&new->node.branches.b[EB_LEFT], new_leaf);
				ebx_setlink(&new->node.branches.b[EB_RGHT], old_node);
			}
			else {
				ebx_setlink(&old->node.node_p, new_left);
				ebx_setlink(&new->node.leaf_p, new_rght);
				ebx_setlink(&new->node.branches.b[EB_LEFT], old_node);
				ebx_setlink(&new->node.branches.b[EB_RGHT], new_leaf);
			}
			break;
		}

		/* walk down */
		root = &old->node.branches;
		side = (((unsigned char *)new->key)[old_node_bit >> 3] >> (~old_node_bit & 7)) & 1;
		troot = ebx_getroot(&root->b[side]);
	}

	/* Ok, now we are inserting <new> between <root> and <old>. <old>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <side>. Update the root's leaf till we have it. Note that we can also
	 * find the side by checking the side of new->node.node_p.
	 */

	/* We need the common higher bits between new->key and old->key.
	 * This number of bits is already in <bit>.
	 * NOTE: we can't get here whit bit < 0 since we found a dup !
	 */
	new->node.bit = bit;
	ebx_setlink(&root->b[side], ebx_dotag(&new->node.branches, EB_NODE));
	return new;
}

#endif /* _EBXISTREE_H */