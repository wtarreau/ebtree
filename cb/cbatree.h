/*
 * Compact Binary Trees - exported functions for operations on node's address
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

#ifndef _CBATREE_H
#define _CBATREE_H

#include <stddef.h>
#include "../common/tools.h"

/* Branch pointer tags used to find the downstream node type. These are stored
 * in the lowest significant bit of the tree pointer.
 */
#define CB_TYPE_MASK     1
#define CB_TYPE_LEAF     0
#define CB_TYPE_NODE     1

/* Tree pointers are tagged absolute pointers to the next cba_node */
typedef (void *) cba_tree_t;

/* Standard node when using absolute pointers */
struct cba_node {
	cba_tree_t l;
	cba_tree_t r;
};

/* Returns the type of the branch pointed to by <tree> among CB_TYPE_LEAF and
 * CB_TYPE_NODE.
 */
static inline size_t __cba_get_branch_type(cba_tree_t tree)
{
	return (size_t)tree & CB_TYPE_MASK;
}

/* Converts a cba_node pointer to its equivalent tagged value for use in ->l/r.
 * NULL is not preserved. <tag> must be either CB_TYPE_LEAF or CB_TYPE_NODE.
 */
static inline cba_tree_t __cba_dotag(struct cba_node *node, size_t tag)
{
	return (cba_tree_t)((char *)node + tag);
}

/* Converts a cba_tree_t to its equivalent untagged pointer. NULL is preserved.
 * <tag> must be either CB_TYPE_LEAF or CB_TYPE_NODE.
 */
static inline struct cba_node *__cba_untag(cba_tree_t tree, size_t tag)
{
	return (struct cba_node *)((char *)tree - tag);
}

//
//struct cba_node *cba_insert(struct cba_node **root, struct cba_node *data);
//struct cba_node *cba_lookup(struct cba_node **root, void *data);
//struct cba_node *cba_lookup_le(struct cba_node **root, void *data);
//struct cba_node *cba_lookup_ge(struct cba_node **root, void *data);
//void *cba_dump_tree(struct cba_node *node, unsigned long pxor, void *last,
//                    int level,
//                    void (*node_dump)(struct cba_node *node, int level),
//                    void (*leaf_dump)(struct cba_node *node, int level));
//
#endif /* _CBATREE_H */
