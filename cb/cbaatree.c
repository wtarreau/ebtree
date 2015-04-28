/*
 * Compact Binary Trees - exported functions for operations on node's address
 *
 * Copyright (C) 2014-2015 Willy Tarreau - w@1wt.eu
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
 * These trees are optimized for adding the minimalest overhead to the stored
 * data. This version uses the node's pointer as the key, for the purpose of
 * quickly finding its neighbours.
 *
 * A few properties :
 * - the xor between two branches of a node cannot be zero since there are no
 *   duplicate keys
 * - the xor between two nodes has *at least* the split bit set, possibly more
 * - the split bit is always strictly smaller for a node than for its parent
 * - the first key is the only one without any node, and it has its branches
 *   set to NULL during insertion to detect it.
 * - a leaf is always present as a node on the path from the root, except for
 *   the inserted first key which has no node
 * - a consequence of the rules above is that a non-first leaf appearing below
 *   a node will necessarily have an associated node with a split bit equal to
 *   or greater than the node's split bit.
 * - another consequence is that below a node, the split bits are different for
 *   each branches since both of them are already present above the node, thus
 *   at different levels, so their respective XOR values will be different.
 * - since all nodes in a given path have a different split bit, if a leaf has
 *   the same split bit as its parent node, it is necessary its assocaited leaf
 *
 * When descending along the tree, it is possible to know that a search key is
 * not present, because its XOR with both of the branches is stricly higher
 * than the inter-branch XOR. The reason is simple : the inter-branch XOR will
 * have its highest bit set indicating the split bit. Since it's the bit that
 * differs between the two branches, the key cannot have it both set and
 * cleared when comparing to the branch values. So xoring the key with both
 * branches will emit a higher bit only when the key's bit differs from both
 * branches' similar bit. Thus, the following equation :
 *      (XOR(key, L) > XOR(L, R)) && (XOR(key, R) > XOR(L, R))
 * is only true when the key should be placed above that node. Since the key
 * has a higher bit which differs from the node, either it has it set and the
 * node has it clear (same for both branches), or it has it clear and the node
 * has it set for both branches. For this reason it's enough to copare the key
 * with any node when the equation above is true, to know if it ought to be
 * present on the left or on the right side. This is useful for insertion and
 * for range lookups.
 */

#include <stddef.h>
#include "cbaatree.h"

static inline cb_ulink_t xorptr(cb_link_t a, cb_link_t b)
{
	return ((cb_ulink_t)a) ^ ((cb_ulink_t)b);
}

struct cbaa_node *cbaa_insert(struct cbaa_node **root, struct cbaa_node *node)
{
	struct cbaa_node *p;
	cb_ulink_t pxor;

	pxor = 0;
	p = *root;

	if (!p) {
		node->l = node->r = NULL;
		goto out;
	}

	while (1) {
		if (!p->l)
			goto leaf;

		if (xorptr(p->l, p->r) >= pxor && pxor != 0)
			goto leaf;

		pxor = xorptr(p->l, p->r);
		if (xorptr(node, p->l) > pxor && xorptr(node, p->r) > pxor) {
			/* can't go below, this node must be below us */
			break;
		}

		if (xorptr(node, p->l) < xorptr(node, p->r))
			root = &p->l;
		else
			root = &p->r;

		p = *root;
	}

 leaf:
	/* We're going to insert <node> above leaf <p> and below <root>. We need
	 * to know two things :
	 *  - whether <p> is left or right on <root>, to unlink it properly and
	 *    to take its place
	 *  - whether we attach <p> left or right below us
	 */

	if (p < node) {
		node->l = p;
		node->r = node;
	}
	else if (p > node) {
		node->l = node;
		node->r = p;
	}

	/* else do nothing if the node was already present */
 out:
	*root = node;
	return node;
}

/* returns the highest node which is less than or equal to data. This is
 * typically used to know what memory area <data> belongs to.
 */
struct cbaa_node *cbaa_lookup_le(struct cbaa_node **root, void *data)
{
	struct cbaa_node *p, *last_r;
	cb_ulink_t pxor;

	pxor = 0;
	p = *root;

	if (!p)
		return p;

	last_r = NULL;
	while (1) {
		if (!p->l || (xorptr(p->l, p->r) >= pxor && pxor != 0)) {
			/* first leaf inserted, or regular leaf. Either
			 * the entry fits our expectations or we have to
			 * roll back and go down the opposite direction.
			 */
			if ((cb_ulink_t)p > (cb_ulink_t)data)
				break;
			return p;
		}

		pxor = xorptr(p->l, p->r);
		if (xorptr(data, p->l) > pxor && xorptr(data, p->r) > pxor) {
			/* The difference between the looked up key and the branches
			 * is higher than the difference between the branches, which
			 * means that the key ought to have been found upper in the
			 * chain. Since we won't find the key below, either we have
			 * a chance to find the largest inferior one below and we
			 * walk down, or we need to rewind.
			 */
			if ((cb_ulink_t)p->l > (cb_ulink_t)data)
				break;

			p = p->r;
			goto walkdown;
		}

		if (xorptr(data, p->l) < xorptr(data, p->r)) {
			root = &p->l;
		}
		else {
			last_r = p;
			root = &p->r;
		}

		p = *root;
	}

	/* first roll back to last node where we turned right, and go down left
	 * or stop at first leaf if any. If we only went down left, we already
	 * found the smallest key so none will suit us.
	 */
	if (!last_r)
		return NULL;

	pxor = xorptr(last_r->l, last_r->r);
	p = last_r->l;

 walkdown:
	/* switch to the other branch last time we turned right */
	while (p->r) {
		if (xorptr(p->l, p->r) >= pxor)
			break;
		pxor = xorptr(p->l, p->r);
		p = p->r;
	}

	if ((cb_ulink_t)p > (cb_ulink_t)data)
		return NULL;
	return p;
}

/* returns the note which equals <data> or NULL if <data> is not in the tree */
struct cbaa_node *cbaa_lookup(struct cbaa_node **root, void *data)
{
	struct cbaa_node *p;
	cb_ulink_t pxor;

	pxor = 0;
	p = *root;

	if (!p)
		return p;

	while (1) {
		if (!p->l || (xorptr(p->l, p->r) >= pxor && pxor != 0)) {
			/* first leaf inserted, or regular leaf */
			if ((cb_ulink_t)p != (cb_ulink_t)data)
				p = NULL;
			break;
		}

		pxor = xorptr(p->l, p->r);
		if (xorptr(data, p->l) > pxor && xorptr(data, p->r) > pxor) {
			/* The difference between the looked up key and the branches
			 * is higher than the difference between the branches, which
			 * means that the key ought to have been found upper in the
			 * chain.
			 */
			p = NULL;
			break;
		}

		if (xorptr(data, p->l) < xorptr(data, p->r))
			root = &p->l;
		else
			root = &p->r;

		p = *root;
	}
	return p;
}

/* returns the lowest node which is greater than or equal to data. This is
 * typically used to know the distance between <data> and the next memory
 * area.
 */
struct cbaa_node *cbaa_lookup_ge(struct cbaa_node **root, void *data)
{
	struct cbaa_node *p, *last_l;
	cb_ulink_t pxor;

	pxor = 0;
	p = *root;

	if (!p)
		return p;

	last_l = NULL;
	while (1) {
		if (!p->l || (xorptr(p->l, p->r) >= pxor && pxor != 0)) {
			/* first leaf inserted, or regular leaf. Either
			 * the entry fits our expectations or we have to
			 * roll back and go down the opposite direction.
			 */
			if ((cb_ulink_t)p < (cb_ulink_t)data)
				break;
			return p;
		}

		pxor = xorptr(p->l, p->r);
		if (xorptr(data, p->l) > pxor && xorptr(data, p->r) > pxor) {
			/* The difference between the looked up key and the branches
			 * is higher than the difference between the branches, which
			 * means that the key ought to have been found upper in the
			 * chain. Since we won't find the key below, either we have
			 * a chance to find the smallest superior one below and we
			 * walk down, or we need to rewind.
			 */
			if ((cb_ulink_t)p->l < (cb_ulink_t)data)
				break;

			p = p->l;
			goto walkdown;
		}

		if (xorptr(data, p->l) < xorptr(data, p->r)) {
			last_l = p;
			root = &p->l;
		}
		else {
			root = &p->r;
		}

		p = *root;
	}

	/* first roll back to last node where we turned right, and go down left
	 * or stop at first leaf if any. If we only went down left, we already
	 * found the smallest key so none will suit us.
	 */
	if (!last_l)
		return NULL;

	pxor = xorptr(last_l->l, last_l->r);
	p = last_l->r;

 walkdown:
	/* switch to the other branch last time we turned left */
	while (p->l) {
		if (xorptr(p->l, p->r) >= pxor)
			break;
		pxor = xorptr(p->l, p->r);
		p = p->l;
	}

	if ((cb_ulink_t)p < (cb_ulink_t)data)
		return NULL;
	return p;
}

/* Dumps a tree through the specified callbacks. */
void *cbaa_dump_tree(struct cbaa_node *node, cb_ulink_t pxor, void *last,
                    int level,
                    void (*node_dump)(struct cbaa_node *node, int level),
                    void (*leaf_dump)(struct cbaa_node *node, int level))
{
	cb_ulink_t xor;

	if (!node) /* empty tree */
		return node;

	if (!node->l) {
		/* first inserted leaf */
		if (leaf_dump)
			leaf_dump(node, level);
		return node;
	}

	xor = xorptr(node->l, node->r);
	if (pxor && xor >= pxor) {
		/* that's a leaf */
		if (leaf_dump)
			leaf_dump(node, level);
		return node;
	}

	/* that's a regular node */
	if (node_dump)
		node_dump(node, level);

	last = cbaa_dump_tree(node->l, xor, last, level + 1, node_dump, leaf_dump);
	return cbaa_dump_tree(node->r, xor, last, level + 1, node_dump, leaf_dump);
}
