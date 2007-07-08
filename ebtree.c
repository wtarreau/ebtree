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

int eb_delete(struct eb_node *node) {
    return __eb_delete(node);
}

struct eb32_node *eb32_insert(struct eb32_node *root, struct eb32_node *new) {
    return __eb32_insert(root, new);
}

struct eb32_node *eb32_lookup(struct eb32_node *root, unsigned long x) {
    return __eb32_lookup(root, x);
}

struct eb64_node *eb64_insert(struct eb64_node *root, struct eb64_node *new) {
    return __eb64_insert(root, new);
}