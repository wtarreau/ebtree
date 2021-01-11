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

/* walk down a dup tree starting at <root> which is known to point to a dup
 * tree's topmost node (hence is untagged), and insert <node> at the best
 * place. For this we descend as far as possible on the right and remember
 * the latest fork point going to the left. We measure the depth of the
 * rightmost branch at the left fork and compare it to the current one. If
 * the right branch isn't as deep as the left one, there's room to install
 * at the end of it, otherwise we have to add a new level and install
 * above the topmost node.

algo: put all intermediary nodes into a stack and stop descending right once
      a leaf is reached. The whole depth is the right depth. Then measure the
      rightmost branch starting from the left node. If it's deeper than the
      right branch (total - current) then the right node is the right place
      to insert. Otherwise go back one step in the tree and try again until
      reaching the first place.

 */
struct cba_node *cba_insert_dup(struct cba_node **root, struct cba_node *node)
{
	struct cba_node *parent;
	struct cba_node *top = *root;
	struct cba_node *stack[128]; // like ebtree, 2^128 dups are enough
	int ldepth, rdepth;
	int pos;


	/* If none of top's branches is tagged, we're either on a leaf or on
	 * two leaves.
	 */
	if (!__cba_is_dup(top)) {
		if (top->l == top->r) { /* top is the nodeless leaf */
			node->l = top;
			node->r = node;
			*root = node;
			goto done;
		}
		/* then one of the branches is top */
		// FIXME: still fails the 1 4 5 4 test
		if (top->l == top) {
			root = &top->l;
			node->l = *root;//top;
			node->r = node;
			*root = __cba_dotag(node);//node;
			goto done;
		}
		else if (top->r == top) {// solves the 1 4 5 4 test
			root = &top->r;
			// install above
			node->l = *root;
			node->r = node;
			*root = __cba_dotag(node);//node;
			goto done;
		}
		// FIXME: still fails the 4 4 4 1 4 test
		node->l = *root;//top;
		node->r = node;
		*root = node;//__cba_dotag(node);//node;
		goto done;
	}

	ldepth = rdepth = 0;

	/* Walk down right and stop once we meet an untagged branch,
	 * indicating a leaf.
	 */
	parent = top;
	stack[rdepth++] = parent;
	while (__cba_tagged(parent->r)) {
		parent = __cba_untag(parent->r);
		stack[rdepth++] = parent;
	}

	/* now let's find the deepest stack entry having a longer left branch
	 * than right branch. The left branch's left is measured by walking on
	 * the rightmost side. In the worst case if none is found we'll be left
	 * with pos==-1, indicating we need to insert a new level between the
	 * root and the topmost node. In any case the key will be inserted as a
	 * leaf on the right of its node part.
	 */
	node->r = node;
	pos = rdepth - 1;
	do {
		ldepth = pos;
		parent = stack[pos]->l;
		while (__cba_tagged(parent)) {
			parent = __cba_untag(parent)->r;
			ldepth++;
			if (ldepth >= rdepth) {
				/* there's some room on the right, install there */
				node->l = stack[pos]->r;
				stack[pos]->r = __cba_dotag(node);
				goto done;
			}
		}
	} while (--pos >= 0);

	/* no more room inside the tree, we need to create a new level above
	 * the top, with the current top node on the left and the new one on
	 * the right.
	 */
	node->l = __cba_dotag(top);
	*root   = node;
 done:
	return node;
}

struct cba_node *cba_insert_u32(struct cba_node **root, struct cba_node *node)
{
	struct cba_u32 *p, *l, *r;
	u32 pxor = ~0; // make sure we don't run the first test.
	u32 key = container_of(node, struct cba_u32, node)->key;
	//u32 pxor_old = ~0;
	//struct cba_u32 *p_old = 0;
	int isdup = 0;

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

	/* the previous xor is initialized to the largest possible inter-branch
	 * value so that it can never match on the first test as we want to use
	 * it to detect a leaf vs node.
	 */
	while (1) {
		p = container_of(*root, struct cba_u32, node);

		if (__cba_is_dup(&p->node)) { // test using 4 4 4 1
			/* This is a cover node on top of a dup tree so both of
			 * its branches have the same key as the node itself.
			 * If the key we're trying to insert is the same, we
			 * insert another dup and don't care about the key. If
			 * the key differs however we have to insert here. Thus
			 * it is a node again.
			 */
			//fprintf(stderr, "[%u@%p] *(%p) = %p [%p %p]\n", key, node, root, *root, (*root)->l, (*root)->r);
			/* this node has the same value as its parent so we
			 * always have key != p->key here, hence insertion must
			 * not fall back to a dup insert.
			 */
			isdup = 1;
			//pxor = 0;
			break;
		}

		/* neither pointer is tagged */	
		l = container_of(p->node.l, struct cba_u32, node);
		r = container_of(p->node.r, struct cba_u32, node);

		/* two equal pointers identifies the nodeless leaf */
		if (l == r)
			break;

		/* so that's either a node or a leaf. Each leaf we visit had
		 * its node part already visited. The only way to distinguish
		 * them is that the inter-branch xor of the leaf will be the
		 * node's one, and will necessarily be larger than the previous
		 * node's xor if the node is above (we've already checked for
		 * direct descendent below). Said differently, if an inter-
		 * branch xor is strictly larger than the previous one, it
		 * necessarily is the one of an upper node, so what we're
		 * seeing cannot be the node, hence it's the leaf.
		 */
		if ((l->key ^ r->key) > pxor) { // test using 2 4 6 4
			break;
		}

		pxor = l->key ^ r->key;
		if (pxor == 0) { // test using 4 4 1
			/* That's the topmost node of a dup tree */
			//fprintf(stderr, "[%u] *(%p) = %p [%p %p]\n", key, root, *root, (*root)->l, (*root)->r);
			isdup = 1;
			break;
		}

		//if (!pxor) {
		//	/* That's the topmost node of a dup tree */
		//	goto insert_dup;
		//}
	
		//if (pxor > pxor_old) // this is a leaf of previous node
		//	break;
		//pxor_old=pxor;
		//p_old = p;

		//if ((key ^ l->key) > pxor && (key ^ l->key ^ pxor) > pxor) {
		//pxor ^= key;
		//if ((l->key) > pxor && (r->key) > pxor) {
		//if ((key ^ (l->key & r->key)) >= pxor) {
		if ((key ^ l->key) > pxor && (key ^ r->key) > pxor) {
			/* can't go lower, the node must be inserted above p
			 * (which is then necessarily a node). We also know
			 * that (key != p->key) because p->key differs from at
			 * least one of its subkeys by a higher bit than the
			 * split bit.
			 */
			//fprintf(stderr, "key=%#x lkey=%#x rkey=%#x pxor=%#x\n", key, l->key, r->key, pxor);
			break;
		}

		//if ((key & (l->key & r->key)) > l->key)
		if ((key ^ l->key) < (key ^ r->key))
			root = &p->node.l;
		else
			root = &p->node.r;

		if (p == container_of(*root, struct cba_u32, node)) {
			// this is the previous node's leaf
			break;
		}
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
	else if (key == p->key && /*!pxor*/ 1/* isdup*/) {
		// 1 4 4 or 1 4 5 4
		l = (typeof(l))(*root)->l;
		r = (typeof(r))(*root)->r;
		fprintf(stderr, "[%u@%p] *(%p) = %p [%#x@%p %#x@%p ^=%#x] pxor=%#x isdup=%d\n", key, node, root, *root, l->key, (*root)->l, r->key, (*root)->r, l->key^r->key, pxor, isdup);
		//if (!isdup && l != r)
		//	goto append;
		return cba_insert_dup(root, node);
	}
	else {
	append:
		//if (key == p->key) {
		//	// 1 4 4 or 1 4 5 4
		//	l = (typeof(l))(*root)->l;
		//	r = (typeof(r))(*root)->r;
		//	fprintf(stderr, "[%u@%p] *(%p) = %p [%#x@%p %#x@%p ^=%#x] pxor=%#x\n", key, node, root, *root, l->key, (*root)->l, r->key, (*root)->r, l->key^r->key, pxor);
		//}
		/* either larger, or equal above a leaf */
		node->l = &p->node;
		node->r = node;
	}

 done:
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

	fprintf(stderr, "node=%p level=%d key=%u l=%p r=%p\n", node, level, *(unsigned *)((char*)(node)+16), node->l, node->r);

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
