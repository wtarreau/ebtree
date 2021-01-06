/*
 * Compact Binary Trees - exported functions for operations on u32 keys
 *
 * Copyright (C) 2014-2021 Willy Tarreau - w@1wt.eu
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
 * - the xor between two branches of a node cannot be zero since unless the two
 *   branches are duplicate keys
 * - the xor between two nodes has *at least* the split bit set, possibly more
 * - the split bit is always strictly smaller for a node than for its parent,
 *   which implies that the xor between the keys of the lowest level node is
 *   always smaller than the xor between a higher level node. Hence the xor
 *   between the branches of a regular leaf is always strictly larger than the
 *   xor of its parent node's branches if this node is different, since the
 *   leaf is associated with a higher level node which has at least one higher
 *   level branch. The first leaf doesn't validate this but is handled by the
 *   rules below.
 * - during the descent, the node corresponding to a leaf is always visited
 *   before the leaf, unless it's the first inserted, nodeless leaf.
 * - the first key is the only one without any node, and it has both its
 *   branches pointing to itself during insertion to detect it (i.e. xor==0).
 * - a leaf is always present as a node on the path from the root, except for
 *   the inserted first key which has no node, and is recognizable by its two
 *   branches pointing to itself.
 * - a consequence of the rules above is that a non-first leaf appearing below
 *   a node will necessarily have an associated node with a split bit equal to
 *   or greater than the node's split bit.
 * - another consequence is that below a node, the split bits are different for
 *   each branches since both of them are already present above the node, thus
 *   at different levels, so their respective XOR values will be different.
 * - since all nodes in a given path have a different split bit, if a leaf has
 *   the same split bit as its parent node, it is necessary its associated leaf
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
 * has it set for both branches. For this reason it's enough to compare the key
 * with any node when the equation above is true, to know if it ought to be
 * present on the left or on the right side. This is useful for insertion and
 * for range lookups.
 */

#include <stddef.h>
#include <stdio.h>
#include "cbatree.h"

/* this structure is aliased to the common cba node during u32 operations */
struct cba_u32 {
	struct cba_node node;
	u32 key;
};

struct cba_node *cba_insert_u32(struct cba_node **root, struct cba_node *node)
{
	struct cba_u32 *p, *l, *r;
	u32 pxor = 0; // make sure we don't run the first test.
	u32 key = container_of(node, struct cba_u32, node)->key;
	u32 pxor_old = ~0;
	struct cba_u32 *p_old = 0;

	if (!*root) {
		/* empty tree */
		node->l = node->r = node;
		goto done;
		//*root = node;
		//return node;
	}

	/* When exiting the loop, pxor will be zero for nodes and first leaf,
	 * or non-zero for a leaf.
	 */

	//p = container_of(*root, struct cba_u32, node);
	//l = container_of(p->node.l, struct cba_u32, node);
	//r = container_of(p->node.r, struct cba_u32, node);
	//
	//while (1) {
	//	/* FIXME: this is wrong, the node below the root may already have a tagged branch */
	//
	//	fprintf(stderr, "l=%p r=%p\n", l, r);
	//	pxor = l->key ^ r->key;
	//	if (!pxor) {
	//		/* That's either a the topmost node of a dup tree or
	//		 * the first inserted leaf.
	//		 */
	//		break;
	//	}
	//
	//	/* check the split bit */
	//	if ((key ^ l->key) > pxor && (key ^ r->key) > pxor) {
	//		/* can't go lower, the node must be inserted above p
	//		 * (which is then necessarily a node).
	//		 */
	//		pxor = 0; // mention that we're above a node
	//		break;
	//	}
	//
	//	if ((key ^ l->key) < (key ^ r->key))
	//		root = &p->node.l;
	//	else
	//		root = &p->node.r;
	//
	//	p = container_of(*root, struct cba_u32, node);
	//
	//	if (__cba_is_dup(&p->node)) {
	//		/* This is a cover node on top of a dup tree so both of
	//		 * its branches have the same key as the node itself.
	//		 * If the key we're trying to insert is the same, we
	//		 * insert another dup and don't care about the key. If
	//		 * the key differs however we have to insert here. Thus
	//		 * it is a node again.
	//		 */
	//		pxor = 0; // mention that we're above a node
	//		break;
	//	}
	//
	//	l = container_of(p->node.l, struct cba_u32, node);
	//	r = container_of(p->node.r, struct cba_u32, node);
	//
	//	/* maybe we've reached a leaf (including ours) ? */
	//	if ((l->key ^ r->key) >= pxor) {
	//		/* this is the only case where we exit with a leaf, and
	//		 * this is reflected by pxor not being zero.
	//		 */
	//		break;
	//	}
	//}

	while (1) {
		p = container_of(*root, struct cba_u32, node);
		if (p == p_old) {
			/* this is our leaf */
			break;
		}
	
		if (__cba_is_dup(&p->node)) {
			/* This is a cover node on top of a dup tree so both of
			 * its branches have the same key as the node itself.
			 * If the key we're trying to insert is the same, we
			 * insert another dup and don't care about the key. If
			 * the key differs however we have to insert here. Thus
			 * it is a node again.
			 */
			pxor = 0; // mention that we're above a node
			break;
			//if (key ^ p->key)
			//	break;
			//goto duptree;
		}

		/* neither pointer is tagged */	
		l = container_of(p->node.l, struct cba_u32, node);
		r = container_of(p->node.r, struct cba_u32, node);

		/* maybe we've reached a leaf (including ours) ? */
		//if ((l->key ^ r->key) >= pxor && pxor != 0) {
		//	/* this is the only case where we exit with a leaf, and
		//	 * this is reflected by pxor not being zero.
		//	 */
		//	break;
		//}
	
		pxor = l->key ^ r->key;
		if (!pxor) {
			/* That's either a the topmost node of a dup tree or
			 * the first inserted leaf.
			 */
			break;
			//if (l == r)
			//	break;
			//goto dupnode;
		}
	
		if (pxor > pxor_old) // this is a leaf of previous node
			break;
	
		//if ((key ^ l->key) > pxor && (key ^ l->key ^ pxor) > pxor) {
		//pxor ^= key;
		//if ((l->key) > pxor && (r->key) > pxor) {
		if ((key ^ l->key) > pxor && (key ^ r->key) > pxor) {
			/* can't go lower, the node must be inserted above p
			 * (which is then necessarily a node).
			 */
			pxor = 0; // mention that we're above a node
			//fprintf(stderr, "key=%#x lkey=%#x rkey=%#x pxor=%#x\n", key, l->key, r->key, pxor);
			break;
		}
	
		if ((key ^ l->key) < (key ^ r->key))
			root = &p->node.l;
		else
			root = &p->node.r;
		p_old = p;
		pxor_old = pxor;
	}

	/* We're going to insert <node> above leaf <p> and below <root>. It's
	 * possible that <p> is the first inserted node, that it's the topmost
	 * node of a duplicate tree, or any other regular node or leaf. However
	 * it cannot be a non-top node of a duplicate tree. Therefore we don't
	 * care about pointer tagging. We need to know two things :
	 *  - whether <p> is left or right on <root>, to unlink it properly and
	 *    to take its place
	 *  - whether we attach <p> left or right below us
	 */

	if (key < p->key) {
		node->l = node;
		node->r = &p->node;
	}
	else if (key > p->key) {
		node->l = &p->node;
		node->r = node;
	}
	else {
		/* We're installing a duplicate of p->key.
		 * FIXME: For now we always insert on top of it on the right and tag
		 * it. Normally we should fall back to pure dup walk through and
		 * insertion.
		 */
		node->l = (pxor || p->node.l == p->node.r) ? &p->node : __cba_dotag(&p->node);
		node->r = node;

		/* FIXME: if the node we're displacing was tagged, the new one must
		 * also be tagged. However for now this does not happen because we
		 * only insert on the top.
		 */
		if (__cba_tagged(*root))
			node = __cba_dotag(node);
	}

 done:
	//fprintf(stderr, "*(%p) = %p -> %p [%p %p]\n", root, *root, node, node->l, node->r);
	*root = node;
	return node;
}

///* returns the highest node which is less than or equal to data. This is
// * typically used to know what memory area <data> belongs to.
// */
//struct cba_node *cba_lookup_le(struct cba_node **root, void *data)
//{
//	struct cba_node *p, *last_r;
//	u32 pxor;
//
//	pxor = 0;
//	p = *root;
//
//	if (!p)
//		return p;
//
//	last_r = NULL;
//	while (1) {
//		if (!p->l || (xorptr(p->l, p->r) >= pxor && pxor != 0)) {
//			/* first leaf inserted, or regular leaf. Either
//			 * the entry fits our expectations or we have to
//			 * roll back and go down the opposite direction.
//			 */
//			if ((u32)p > (u32)data)
//				break;
//			return p;
//		}
//
//		pxor = xorptr(p->l, p->r);
//		if (xorptr(data, p->l) > pxor && xorptr(data, p->r) > pxor) {
//			/* The difference between the looked up key and the branches
//			 * is higher than the difference between the branches, which
//			 * means that the key ought to have been found upper in the
//			 * chain. Since we won't find the key below, either we have
//			 * a chance to find the largest inferior one below and we
//			 * walk down, or we need to rewind.
//			 */
//			if ((u32)p->l > (u32)data)
//				break;
//
//			p = p->r;
//			goto walkdown;
//		}
//
//		if (xorptr(data, p->l) < xorptr(data, p->r)) {
//			root = &p->l;
//		}
//		else {
//			last_r = p;
//			root = &p->r;
//		}
//
//		p = *root;
//	}
//
//	/* first roll back to last node where we turned right, and go down left
//	 * or stop at first leaf if any. If we only went down left, we already
//	 * found the smallest key so none will suit us.
//	 */
//	if (!last_r)
//		return NULL;
//
//	pxor = xorptr(last_r->l, last_r->r);
//	p = last_r->l;
//
// walkdown:
//	/* switch to the other branch last time we turned right */
//	while (p->r) {
//		if (xorptr(p->l, p->r) >= pxor)
//			break;
//		pxor = xorptr(p->l, p->r);
//		p = p->r;
//	}
//
//	if ((u32)p > (u32)data)
//		return NULL;
//	return p;
//}
//
///* returns the note which equals <data> or NULL if <data> is not in the tree */
//struct cba_node *cba_lookup(struct cba_node **root, void *data)
//{
//	struct cba_node *p;
//	u32 pxor;
//
//	pxor = 0;
//	p = *root;
//
//	if (!p)
//		return p;
//
//	while (1) {
//		if (!p->l || (xorptr(p->l, p->r) >= pxor && pxor != 0)) {
//			/* first leaf inserted, or regular leaf */
//			if ((u32)p != (u32)data)
//				p = NULL;
//			break;
//		}
//
//		pxor = xorptr(p->l, p->r);
//		if (xorptr(data, p->l) > pxor && xorptr(data, p->r) > pxor) {
//			/* The difference between the looked up key and the branches
//			 * is higher than the difference between the branches, which
//			 * means that the key ought to have been found upper in the
//			 * chain.
//			 */
//			p = NULL;
//			break;
//		}
//
//		if (xorptr(data, p->l) < xorptr(data, p->r))
//			root = &p->l;
//		else
//			root = &p->r;
//
//		p = *root;
//	}
//	return p;
//}
//
///* returns the lowest node which is greater than or equal to data. This is
// * typically used to know the distance between <data> and the next memory
// * area.
// */
//struct cba_node *cba_lookup_ge(struct cba_node **root, void *data)
//{
//	struct cba_node *p, *last_l;
//	u32 pxor;
//
//	pxor = 0;
//	p = *root;
//
//	if (!p)
//		return p;
//
//	last_l = NULL;
//	while (1) {
//		if (!p->l || (xorptr(p->l, p->r) >= pxor && pxor != 0)) {
//			/* first leaf inserted, or regular leaf. Either
//			 * the entry fits our expectations or we have to
//			 * roll back and go down the opposite direction.
//			 */
//			if ((u32)p < (u32)data)
//				break;
//			return p;
//		}
//
//		pxor = xorptr(p->l, p->r);
//		if (xorptr(data, p->l) > pxor && xorptr(data, p->r) > pxor) {
//			/* The difference between the looked up key and the branches
//			 * is higher than the difference between the branches, which
//			 * means that the key ought to have been found upper in the
//			 * chain. Since we won't find the key below, either we have
//			 * a chance to find the smallest superior one below and we
//			 * walk down, or we need to rewind.
//			 */
//			if ((u32)p->l < (u32)data)
//				break;
//
//			p = p->l;
//			goto walkdown;
//		}
//
//		if (xorptr(data, p->l) < xorptr(data, p->r)) {
//			last_l = p;
//			root = &p->l;
//		}
//		else {
//			root = &p->r;
//		}
//
//		p = *root;
//	}
//
//	/* first roll back to last node where we turned right, and go down left
//	 * or stop at first leaf if any. If we only went down left, we already
//	 * found the smallest key so none will suit us.
//	 */
//	if (!last_l)
//		return NULL;
//
//	pxor = xorptr(last_l->l, last_l->r);
//	p = last_l->r;
//
// walkdown:
//	/* switch to the other branch last time we turned left */
//	while (p->l) {
//		if (xorptr(p->l, p->r) >= pxor)
//			break;
//		pxor = xorptr(p->l, p->r);
//		p = p->l;
//	}
//
//	if ((u32)p < (u32)data)
//		return NULL;
//	return p;
//}

/* Dumps a tree through the specified callbacks. */
void *cba_dump_tree_u32(struct cba_node *node, u32 pxor, void *last,
			int level,
			void (*node_dump)(struct cba_node *node, int level),
			void (*leaf_dump)(struct cba_node *node, int level))
{
	u32 xor;

	if (!node) /* empty tree */
		return node;

	fprintf(stderr, "node=%p level=%d\n", node, level);

	if (level < 0) {
		/* we're inside a dup tree. Tagged pointers indicate nodes,
		 * untagged ones leaves.
		 */
		level--;
		if (__cba_tagged(node->l)) {
			last = cba_dump_tree_u32(__cba_untag(node->l), 0, last, level, node_dump, leaf_dump);
			if (node_dump)
				node_dump(__cba_untag(node->l), level);
		} else if (leaf_dump)
			leaf_dump(node->l, level);

		if (__cba_tagged(node->r)) {
			last = cba_dump_tree_u32(__cba_untag(node->r), 0, last, level, node_dump, leaf_dump);
			if (node_dump)
				node_dump(__cba_untag(node->r), level);
		} else if (leaf_dump)
			leaf_dump(node->r, level);
		return node;
	}

	/* regular nodes, all branches are canonical */

	if (node->l == node->r) {
		/* first inserted leaf */
		if (leaf_dump)
			leaf_dump(node, level);
		return node;
	}

	if (__cba_is_dup(node)) {
		if (node_dump)
			node_dump(node, -1);
		return cba_dump_tree_u32(node, 0, last, -1, node_dump, leaf_dump);
	}

	xor = ((struct cba_u32*)node->l)->key ^ ((struct cba_u32*)node->r)->key;
	if (pxor && xor >= pxor) {
		/* that's a leaf */
		if (leaf_dump)
			leaf_dump(node, level);
		return node;
	}

	if (!xor) {
		/* start of a dup */
		if (node_dump)
			node_dump(node, -1);
		return cba_dump_tree_u32(node, 0, last, -1, node_dump, leaf_dump);
	}

	/* that's a regular node */
	if (node_dump)
		node_dump(node, level);

	last = cba_dump_tree_u32(node->l, xor, last, level + 1, node_dump, leaf_dump);
	return cba_dump_tree_u32(node->r, xor, last, level + 1, node_dump, leaf_dump);
}
