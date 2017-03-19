/*
 * Elastic Binary Trees - exported generic functions
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

/* Removes a leaf node from the tree if it was still in it. Marks the node
 * as unlinked. It is meant to be used by internal functions only as it's
 * not re-entrant. Use ebx_delete() instead.
 */
void __ebx_delete(struct ebx_node *node)
{
	unsigned int pside, gpside, sibtype;
	struct ebx_node *parent;
	struct ebx_root *gparent;

	if (!node->leaf_p)
		return;

	/* we need the parent, our side, and the grand parent */
	pside = __ebx_gettag(ebx_getroot(&node->leaf_p));
	parent = ebx_root_to_node(__ebx_untag(ebx_getroot(&node->leaf_p), pside));

	/* We likely have to release the parent link, unless it's the root,
	 * in which case we only set our branch to NULL. Note that we can
	 * only be attached to the root by its left branch.
	 */
	if (ebx_is_root(&parent->branches)) {
		/* we're just below the root, it's trivial. */
		parent->branches.b[EB_LEFT] = 0;
		goto delete_unlink;
	}

	/* To release our parent, we have to identify our sibling, and reparent
	 * it directly to/from the grand parent. Note that the sibling can
	 * either be a link or a leaf.
	 */

	gpside = __ebx_gettag(ebx_getroot(&parent->node_p));
	gparent = __ebx_untag(ebx_getroot(&parent->node_p), gpside);

	ebx_setlink(&gparent->b[gpside], ebx_getroot(&parent->branches.b[!pside]));
	sibtype = __ebx_gettag(ebx_getroot(&gparent->b[gpside]));

	if (sibtype == EB_LEAF) {
		ebx_setlink(&ebx_root_to_node(__ebx_untag(ebx_getroot(&gparent->b[gpside]), EB_LEAF))->leaf_p, __ebx_dotag(gparent, gpside));
	} else {
		ebx_setlink(&ebx_root_to_node(__ebx_untag(ebx_getroot(&gparent->b[gpside]), EB_NODE))->node_p, __ebx_dotag(gparent, gpside));
	}
	/* Mark the parent unused. Note that we do not check if the parent is
	 * our own node, but that's not a problem because if it is, it will be
	 * marked unused at the same time, which we'll use below to know we can
	 * safely remove it.
	 */
	parent->node_p = 0;

	/* The parent node has been detached, and is currently unused. It may
	 * belong to another node, so we cannot remove it that way. Also, our
	 * own node part might still be used. so we can use this spare node
	 * to replace ours if needed.
	 */

	/* If our link part is unused, we can safely exit now */
	if (!node->node_p)
		goto delete_unlink;

	/* From now on, <node> and <parent> are necessarily different, and the
	 * <node>'s node part is in use. By definition, <parent> is at least
	 * below <node>, so keeping its key for the bit string is OK.
	 */

	ebx_setlink(&parent->node_p, ebx_getroot(&node->node_p));
	ebx_setlink(&parent->branches.b[EB_LEFT], ebx_getroot(&node->branches.b[EB_LEFT]));
	ebx_setlink(&parent->branches.b[EB_RGHT], ebx_getroot(&node->branches.b[EB_RGHT]));
	parent->bit = node->bit;

	/* We must now update the new node's parent... */
	gpside = __ebx_gettag(ebx_getroot(&parent->node_p));
	gparent = __ebx_untag(ebx_getroot(&parent->node_p), gpside);
	ebx_setlink(&gparent->b[gpside], __ebx_dotag(&parent->branches, EB_NODE));

	/* ... and its branches */
	for (pside = 0; pside <= 1; pside++) {
		if (__ebx_gettag(ebx_getroot(&parent->branches.b[pside])) == EB_NODE) {
			ebx_setlink(&ebx_root_to_node(__ebx_untag(ebx_getroot(&parent->branches.b[pside]), EB_NODE))->node_p,
				__ebx_dotag(&parent->branches, pside));
		} else {
			ebx_setlink(&ebx_root_to_node(__ebx_untag(ebx_getroot(&parent->branches.b[pside]), EB_LEAF))->leaf_p,
				__ebx_dotag(&parent->branches, pside));
		}
	}
 delete_unlink:
	/* Now the node has been completely unlinked */
	node->leaf_p = 0;
	return; /* tree is not empty yet */
}


/* This function is used to build a tree of duplicates by adding a new node to
 * a subtree of at least 2 entries. It is meant for use by internal functions,
 * user code should use ebx_insert_dup() instead.
 */
REGPRM1 struct ebx_node *__ebx_insert_dup(struct ebx_node *sub, struct ebx_node *new)
{
	struct ebx_node *head = sub;

	ebx_troot_t *new_left = __ebx_dotag(&new->branches, EB_LEFT);
	ebx_troot_t *new_rght = __ebx_dotag(&new->branches, EB_RGHT);
	ebx_troot_t *new_leaf = __ebx_dotag(&new->branches, EB_LEAF);

	/* first, identify the deepest hole on the right branch */
	while (__ebx_gettag(ebx_getroot(&head->branches.b[EB_RGHT])) != EB_LEAF) {
		struct ebx_node *last = head;
		head = container_of(__ebx_untag(ebx_getroot(&head->branches.b[EB_RGHT]), EB_NODE),
				    struct ebx_node, branches);
		if (head->bit > last->bit + 1)
			sub = head;     /* there's a hole here */
	}

	/* Here we have a leaf attached to (head)->b[EB_RGHT] */
	if (head->bit < -1) {
		/* A hole exists just before the leaf, we insert there */
		new->bit = -1;
		sub = container_of(__ebx_untag(ebx_getroot(&head->branches.b[EB_RGHT]), EB_LEAF),
				   struct ebx_node, branches);
		ebx_setlink(&head->branches.b[EB_RGHT], __ebx_dotag(&new->branches, EB_NODE));

		ebx_setlink(&new->node_p, ebx_getroot(&sub->leaf_p));
		ebx_setlink(&new->leaf_p, new_rght);
		ebx_setlink(&sub->leaf_p, new_left);
		ebx_setlink(&new->branches.b[EB_LEFT], __ebx_dotag(&sub->branches, EB_LEAF));
		ebx_setlink(&new->branches.b[EB_RGHT], new_leaf);
		return new;
	} else {
		int side;
		/* No hole was found before a leaf. We have to insert above
		 * <sub>. Note that we cannot be certain that <sub> is attached
		 * to the right of its parent, as this is only true if <sub>
		 * is inside the dup tree, not at the head.
		 */
		new->bit = sub->bit - 1; /* install at the lowest level */
		side = __ebx_gettag(ebx_getroot(&sub->node_p));
		head = container_of(__ebx_untag(ebx_getroot(&sub->node_p), side), struct ebx_node, branches);
		ebx_setlink(&head->branches.b[side], __ebx_dotag(&new->branches, EB_NODE));

		ebx_setlink(&new->node_p, ebx_getroot(&sub->node_p));
		ebx_setlink(&new->leaf_p, new_rght);
		ebx_setlink(&sub->node_p, new_left);
		ebx_setlink(&new->branches.b[EB_LEFT], __ebx_dotag(&sub->branches, EB_NODE));
		ebx_setlink(&new->branches.b[EB_RGHT], new_leaf);
		return new;
	}
}
