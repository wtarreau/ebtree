/*
 * Elastic Binary Trees
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

int eb_delete(struct eb_node *node)
{
    return __eb_delete(node);
}

/* Returns the first leaf in the tree starting at <root>, or NULL if none */
__attribute__((noinline))
struct eb_node *eb_first(struct eb_root *root)
{
	return __eb_first(root);
}

/* Returns the last leaf in the tree starting at <root>, or NULL if none */
__attribute__((noinline))
struct eb_node *eb_last(struct eb_root *root)
{
	return __eb_last(root);
}

/* returns previous leaf node before an existing leaf node, or NULL if none. */
__attribute__((noinline))
struct eb_node *eb_prev(struct eb_node *node)
{
	return __eb_prev(node);
}

/* returns next leaf node after an existing leaf node, or NULL if none. */
__attribute__((noinline))
struct eb_node *eb_next(struct eb_node *node)
{
	return __eb_next(node);
}

/* used by insertion primitives */
__attribute__((noinline))
struct eb_node *eb_insert_dup(struct eb_node *sub, struct eb_node *new)
{
	return __eb_insert_dup(sub, new);
}

struct eb32_node *eb32_insert(struct eb_root *root, struct eb32_node *new)
{
    return __eb32_insert(root, new);
}

struct eb32_node *eb32i_insert(struct eb_root *root, struct eb32_node *new)
{
    return __eb32i_insert(root, new);
}

struct eb32_node *eb32_lookup(struct eb_root *root, u32 x) {
    return __eb32_lookup(root, x);
}

struct eb64_node *eb64_insert(struct eb_root *root, struct eb64_node *new) {
    return __eb64_insert(root, new);
}

struct eb64_node *eb64i_insert(struct eb_root *root, struct eb64_node *new) {
    return __eb64i_insert(root, new);
}

