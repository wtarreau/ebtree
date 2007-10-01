/*
 * Elastic Binary Trees - macros and structures for operations on 32bit nodes.
 * (C) 2002-2007 - Willy Tarreau <w@1wt.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ebtree.h"


/* Return the structure of type <type> whose member <member> points to <ptr> */
#define eb32_entry(ptr, type, member) container_of(ptr, type, member)

#define EB32_ROOT	EB_ROOT
#define EB32_TREE_HEAD	EB_TREE_HEAD

/* These types may sometimes already be defined */
typedef unsigned int u32;
typedef   signed int s32;

/* This structure carries a node, a leaf, and a value. It must start with the
 * eb_node so that it can be cast into an eb_node. We could also have put some
 * sort of transparent union here to reduce the indirection level, but the fact
 * is, the end user is not meant to manipulate internals, so this is pointless.
 */
struct eb32_node {
	struct eb_node node; /* the tree node, must be at the beginning */
	u32 val;
};

/*
 * Exported functions and macros.
 * Many of them are always inlined because they are extremely small, and
 * are generally called at most once or twice in a program.
 */

/* Return leftmost node in the tree, or NULL if none */
static inline struct eb32_node *eb32_first(struct eb_root *root)
{
	return eb32_entry(eb_first(root), struct eb32_node, node);
}

/* Return rightmost node in the tree, or NULL if none */
static inline struct eb32_node *eb32_last(struct eb_root *root)
{
	return eb32_entry(eb_last(root), struct eb32_node, node);
}

/* Return next node in the tree, or NULL if none */
static inline struct eb32_node *eb32_next(struct eb32_node *eb32)
{
	return eb32_entry(eb_next(&eb32->node), struct eb32_node, node);
}

/* Return previous node in the tree, or NULL if none */
static inline struct eb32_node *eb32_prev(struct eb32_node *eb32)
{
	return eb32_entry(eb_prev(&eb32->node), struct eb32_node, node);
}

/* Delete node from the tree. Return 0 if the tree is empty afterwards. Note
 * that this function relies on a non-inlined generic function: eb_delete.
 */
static inline int eb32_delete(struct eb32_node *eb32)
{
	return eb_delete(&eb32->node);
}

/*
 * The following functions are not inlined by default. They are declared
 * in eb32tree.c, which simply relies on their inline version.
 */
struct eb32_node *eb32_lookup(struct eb_root *root, u32 x);
struct eb32_node *eb32i_lookup(struct eb_root *root, s32 x);
struct eb32_node *eb32_insert(struct eb_root *root, struct eb32_node *new);
struct eb32_node *eb32i_insert(struct eb_root *root, struct eb32_node *new);

/*
 * The following functions are less likely to be used directly, because their
 * code is larger. The non-inlined version is preferred.
 */

/* Delete node from the tree. Return 0 if the tree is empty afterwards */
static inline int __eb32_delete(struct eb32_node *eb32)
{
	return __eb_delete(&eb32->node);
}

/*
 * Find the first occurence of a value in the tree <root>. If none can be
 * found, return NULL.
 */
static inline struct eb32_node *__eb32_lookup(struct eb_root *root, u32 x)
{
	struct eb32_node *node;
	eb_troot_t *troot;

	troot = root->b[EB_LEFT];
	if (unlikely(troot == NULL))
		return NULL;

	while (1) {
		if ((eb_gettag(troot) == EB_LEAF)) {
			node = container_of(eb_untag(troot, EB_LEAF),
					    struct eb32_node, node.branches);
			if (node->val == x)
				return node;
			else
				return NULL;
		}
		node = container_of(eb_untag(troot, EB_NODE),
				    struct eb32_node, node.branches);

		if (x == node->val) {
			/* Either we found the node which holds the value, or
			 * we have a dup tree. In the later case, we have to
			 * walk it down left to get the first entry.
			 */
			if (node->node.bit < 0) {
				troot = node->node.branches.b[EB_LEFT];
				while (eb_gettag(troot) != EB_LEAF)
					troot = (eb_untag(troot, EB_NODE))->b[EB_LEFT];
				node = container_of(eb_untag(troot, EB_LEAF),
						    struct eb32_node, node.branches);
			}
			return node;
		}

		troot = node->node.branches.b[(x >> node->node.bit) & EB_NODE_BRANCH_MASK];
	}
}

/*
 * Find the first occurence of a signed value in the tree <root>. If none can
 * be found, return NULL.
 */
static inline struct eb32_node *__eb32i_lookup(struct eb_root *root, s32 x)
{
	struct eb32_node *node;
	eb_troot_t *troot;
	u32 val = x ^ 0x80000000;

	troot = root->b[EB_LEFT];
	if (unlikely(troot == NULL))
		return NULL;

	while (1) {
		if ((eb_gettag(troot) == EB_LEAF)) {
			node = container_of(eb_untag(troot, EB_LEAF),
					    struct eb32_node, node.branches);
			if (node->val == x)
				return node;
			else
				return NULL;
		}
		node = container_of(eb_untag(troot, EB_NODE),
				    struct eb32_node, node.branches);

		if (x == node->val) {
			/* Either we found the node which holds the value, or
			 * we have a dup tree. In the later case, we have to
			 * walk it down left to get the first entry.
			 */
			if (node->node.bit < 0) {
				troot = node->node.branches.b[EB_LEFT];
				while (eb_gettag(troot) != EB_LEAF)
					troot = (eb_untag(troot, EB_NODE))->b[EB_LEFT];
				node = container_of(eb_untag(troot, EB_LEAF),
						    struct eb32_node, node.branches);
			}
			return node;
		}

		troot = node->node.branches.b[(val >> node->node.bit) & EB_NODE_BRANCH_MASK];
	}
}

/* Insert eb32_node <new> into subtree starting at node root <root>.
 * Only new->val needs be set with the value. The eb32_node is returned.
 */
static inline struct eb32_node *
__eb32_insert(struct eb_root *root, struct eb32_node *new) {
	struct eb32_node *old;
	unsigned int side;
	eb_troot_t *troot;
	u32 newval; /* caching the value saves approximately one cycle */

	side = EB_LEFT;
	troot = root->b[EB_LEFT];
	if (unlikely(troot == NULL)) {
		/* Tree is empty, insert the leaf part below the left branch */
		root->b[EB_LEFT] = eb_dotag(&new->node.branches, EB_LEAF);
		new->node.leaf_p = eb_dotag(root, EB_LEFT);
		new->node.node_p = NULL; /* node part unused */
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
	 * was attached. <newval> carries the value being inserted.
	 */
	newval = new->val;

	while (1) {
		if (unlikely(eb_gettag(troot) == EB_LEAF)) {
			eb_troot_t *new_left, *new_rght;
			eb_troot_t *new_leaf, *old_leaf;

			old = container_of(eb_untag(troot, EB_LEAF),
					    struct eb32_node, node.branches);

			new_left = eb_dotag(&new->node.branches, EB_LEFT);
			new_rght = eb_dotag(&new->node.branches, EB_RGHT);
			new_leaf = eb_dotag(&new->node.branches, EB_LEAF);
			old_leaf = eb_dotag(&old->node.branches, EB_LEAF);

			new->node.node_p = old->node.leaf_p;

			/* Right here, we have 3 possibilities :
			   - the tree does not contain the value, and we have
			     new->val < old->val. We insert new above old, on
			     the left ;

			   - the tree does not contain the value, and we have
			     new->val > old->val. We insert new above old, on
			     the right ;

			   - the tree does contain the value, which implies it
			     is alone. We add the new value next to it as a
			     first duplicate.

			   The last two cases can easily be partially merged.
			*/
			 
			if (new->val < old->val) {
				new->node.leaf_p = new_left;
				old->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = new_leaf;
				new->node.branches.b[EB_RGHT] = old_leaf;
			} else {
				/* new->val >= old->val, new goes the right */
				old->node.leaf_p = new_left;
				new->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = old_leaf;
				new->node.branches.b[EB_RGHT] = new_leaf;

				if (new->val == old->val) {
					new->node.bit = -1;
					root->b[side] = eb_dotag(&new->node.branches, EB_NODE);
					return new;
				}
			}
			break;
		}

		/* OK we're walking down this link */
		old = container_of(eb_untag(troot, EB_NODE),
				    struct eb32_node, node.branches);

		/* Stop going down when we don't have common bits anymore. We
		 * also stop in front of a duplicates tree because it means we
		 * have to insert above.
		 */

		if ((old->node.bit < 0) || /* we're above a duplicate tree, stop here */
		    (((new->val ^ old->val) >> old->node.bit) >= EB_NODE_BRANCHES)) {
			/* The tree did not contain the value, so we insert <new> before the node
			 * <old>, and set ->bit to designate the lowest bit position in <new>
			 * which applies to ->branches.b[].
			 */
			eb_troot_t *new_left, *new_rght;
			eb_troot_t *new_leaf, *old_node;

			new_left = eb_dotag(&new->node.branches, EB_LEFT);
			new_rght = eb_dotag(&new->node.branches, EB_RGHT);
			new_leaf = eb_dotag(&new->node.branches, EB_LEAF);
			old_node = eb_dotag(&old->node.branches, EB_NODE);

			new->node.node_p = old->node.node_p;

			if (new->val < old->val) {
				new->node.leaf_p = new_left;
				old->node.node_p = new_rght;
				new->node.branches.b[EB_LEFT] = new_leaf;
				new->node.branches.b[EB_RGHT] = old_node;
			}
			else if (new->val > old->val) {
				old->node.node_p = new_left;
				new->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = old_node;
				new->node.branches.b[EB_RGHT] = new_leaf;
			}
			else {
				struct eb_node *ret;
				ret = eb_insert_dup(&old->node, &new->node);
				return container_of(ret, struct eb32_node, node);
			}
			break;
		}

		/* walk down */
		root = &old->node.branches;
		side = (newval >> old->node.bit) & EB_NODE_BRANCH_MASK;
		troot = root->b[side];
	}

	/* Ok, now we are inserting <new> between <root> and <old>. <old>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <side>. Update the root's leaf till we have it. Note that we can also
	 * find the side by checking the side of new->node.node_p.
	 */

	/* We need the common higher bits between new->val and old->val.
	 * What differences are there between new->val and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of new->val and old->val are identical here (otherwise they
	 * would sit on different branches).
	 */
	// note that if EB_NODE_BITS > 1, we should check that it's still >= 0
	new->node.bit = flsnz(new->val ^ old->val) - EB_NODE_BITS;
	root->b[side] = eb_dotag(&new->node.branches, EB_NODE);

	return new;
}

/* Insert eb32_node <new> into subtree starting at node root <root>, using
 * signed values. Only new->val needs be set with the value. The eb32_node
 * is returned.
 */
static inline struct eb32_node *
__eb32i_insert(struct eb_root *root, struct eb32_node *new) {
	struct eb32_node *old;
	unsigned int side;
	eb_troot_t *troot;
	int newval; /* caching the value saves approximately one cycle */

	side = EB_LEFT;
	troot = root->b[EB_LEFT];
	if (unlikely(troot == NULL)) {
		/* Tree is empty, insert the leaf part below the left branch */
		root->b[EB_LEFT] = eb_dotag(&new->node.branches, EB_LEAF);
		new->node.leaf_p = eb_dotag(root, EB_LEFT);
		new->node.node_p = NULL; /* node part unused */
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
	 * was attached. <newval> carries a high bit shift of the value being
	 * inserted in order to have negative values stored before positive
	 * ones.
	 */
	newval = new->val + 0x80000000;

	while (1) {
		if (unlikely(eb_gettag(troot) == EB_LEAF)) {
			eb_troot_t *new_left, *new_rght;
			eb_troot_t *new_leaf, *old_leaf;

			old = container_of(eb_untag(troot, EB_LEAF),
					    struct eb32_node, node.branches);

			new_left = eb_dotag(&new->node.branches, EB_LEFT);
			new_rght = eb_dotag(&new->node.branches, EB_RGHT);
			new_leaf = eb_dotag(&new->node.branches, EB_LEAF);
			old_leaf = eb_dotag(&old->node.branches, EB_LEAF);

			new->node.node_p = old->node.leaf_p;

			/* Right here, we have 3 possibilities :
			   - the tree does not contain the value, and we have
			     new->val < old->val. We insert new above old, on
			     the left ;

			   - the tree does not contain the value, and we have
			     new->val > old->val. We insert new above old, on
			     the right ;

			   - the tree does contain the value, which implies it
			     is alone. We add the new value next to it as a
			     first duplicate.

			   The last two cases can easily be partially merged.
			*/
			 
			if ((s32)new->val < (s32)old->val) {
				new->node.leaf_p = new_left;
				old->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = new_leaf;
				new->node.branches.b[EB_RGHT] = old_leaf;
			} else {
				/* new->val >= old->val, new goes the right */
				old->node.leaf_p = new_left;
				new->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = old_leaf;
				new->node.branches.b[EB_RGHT] = new_leaf;

				if (new->val == old->val) {
					new->node.bit = -1;
					root->b[side] = eb_dotag(&new->node.branches, EB_NODE);
					return new;
				}
			}
			break;
		}

		/* OK we're walking down this link */
		old = container_of(eb_untag(troot, EB_NODE),
				    struct eb32_node, node.branches);

		/* Stop going down when we don't have common bits anymore. We
		 * also stop in front of a duplicates tree because it means we
		 * have to insert above.
		 */

		if ((old->node.bit < 0) || /* we're above a duplicate tree, stop here */
		    (((new->val ^ old->val) >> old->node.bit) >= EB_NODE_BRANCHES)) {
			/* The tree did not contain the value, so we insert <new> before the node
			 * <old>, and set ->bit to designate the lowest bit position in <new>
			 * which applies to ->branches.b[].
			 */
			eb_troot_t *new_left, *new_rght;
			eb_troot_t *new_leaf, *old_node;

			new_left = eb_dotag(&new->node.branches, EB_LEFT);
			new_rght = eb_dotag(&new->node.branches, EB_RGHT);
			new_leaf = eb_dotag(&new->node.branches, EB_LEAF);
			old_node = eb_dotag(&old->node.branches, EB_NODE);

			new->node.node_p = old->node.node_p;

			if ((s32)new->val < (s32)old->val) {
				new->node.leaf_p = new_left;
				old->node.node_p = new_rght;
				new->node.branches.b[EB_LEFT] = new_leaf;
				new->node.branches.b[EB_RGHT] = old_node;
			}
			else if ((s32)new->val > (s32)old->val) {
				old->node.node_p = new_left;
				new->node.leaf_p = new_rght;
				new->node.branches.b[EB_LEFT] = old_node;
				new->node.branches.b[EB_RGHT] = new_leaf;
			}
			else {
				struct eb_node *ret;
				ret = eb_insert_dup(&old->node, &new->node);
				return container_of(ret, struct eb32_node, node);
			}
			break;
		}

		/* walk down */
		root = &old->node.branches;
		side = (newval >> old->node.bit) & EB_NODE_BRANCH_MASK;
		troot = root->b[side];
	}

	/* Ok, now we are inserting <new> between <root> and <old>. <old>'s
	 * parent is already set to <new>, and the <root>'s branch is still in
	 * <side>. Update the root's leaf till we have it. Note that we can also
	 * find the side by checking the side of new->node.node_p.
	 */

	/* We need the common higher bits between new->val and old->val.
	 * What differences are there between new->val and the node here ?
	 * NOTE that bit(new) is always < bit(root) because highest
	 * bit of new->val and old->val are identical here (otherwise they
	 * would sit on different branches).
	 */
	// note that if EB_NODE_BITS > 1, we should check that it's still >= 0
	new->node.bit = flsnz(new->val ^ old->val) - EB_NODE_BITS;
	root->b[side] = eb_dotag(&new->node.branches, EB_NODE);

	return new;
}
