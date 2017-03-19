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

void ebx_delete(struct ebx_node *node)
{
	__ebx_delete(node);
}

/* This function is used to build a tree of duplicates by adding a new node to
 * a subtree of at least 2 entries. It is meant for use by internal functions,
 * user code should use ebx_insert_dup() instead.
 */
REGPRM1 struct ebx_node *__ebx_insert_dup(struct ebx_node *sub, struct ebx_node *new)
{
	struct ebx_node *head = sub;

	ebx_troot_t *new_left = ebx_dotag(&new->branches, EB_LEFT);
	ebx_troot_t *new_rght = ebx_dotag(&new->branches, EB_RGHT);
	ebx_troot_t *new_leaf = ebx_dotag(&new->branches, EB_LEAF);

	/* first, identify the deepest hole on the right branch */
	while (ebx_gettag(ebx_getroot(&head->branches.b[EB_RGHT])) != EB_LEAF) {
		struct ebx_node *last = head;
		head = container_of(ebx_untag(ebx_getroot(&head->branches.b[EB_RGHT]), EB_NODE),
				    struct ebx_node, branches);
		if (head->bit > last->bit + 1)
			sub = head;     /* there's a hole here */
	}

	/* Here we have a leaf attached to (head)->b[EB_RGHT] */
	if (head->bit < -1) {
		/* A hole exists just before the leaf, we insert there */
		new->bit = -1;
		sub = container_of(ebx_untag(ebx_getroot(&head->branches.b[EB_RGHT]), EB_LEAF),
				   struct ebx_node, branches);
		ebx_setlink(&head->branches.b[EB_RGHT], ebx_dotag(&new->branches, EB_NODE));

		ebx_setlink(&new->node_p, ebx_getroot(&sub->leaf_p));
		ebx_setlink(&new->leaf_p, new_rght);
		ebx_setlink(&sub->leaf_p, new_left);
		ebx_setlink(&new->branches.b[EB_LEFT], ebx_dotag(&sub->branches, EB_LEAF));
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
		side = ebx_gettag(ebx_getroot(&sub->node_p));
		head = container_of(ebx_untag(ebx_getroot(&sub->node_p), side), struct ebx_node, branches);
		ebx_setlink(&head->branches.b[side], ebx_dotag(&new->branches, EB_NODE));

		ebx_setlink(&new->node_p, ebx_getroot(&sub->node_p));
		ebx_setlink(&new->leaf_p, new_rght);
		ebx_setlink(&sub->node_p, new_left);
		ebx_setlink(&new->branches.b[EB_LEFT], ebx_dotag(&sub->branches, EB_NODE));
		ebx_setlink(&new->branches.b[EB_RGHT], new_leaf);
		return new;
	}
}
