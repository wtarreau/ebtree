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

#ifndef _CBAATREE_H
#define _CBAATREE_H

#include "../common/tools.h"

/* absolute pointer version, large size */
struct cbaa_node {
	struct cbaa_node *l;
	struct cbaa_node *r;
};

struct cbaa_node *cbaa_insert(struct cbaa_node **root, struct cbaa_node *data);
struct cbaa_node *cbaa_lookup(struct cbaa_node **root, void *data);
struct cbaa_node *cbaa_lookup_le(struct cbaa_node **root, void *data);
struct cbaa_node *cbaa_lookup_ge(struct cbaa_node **root, void *data);
void *cbaa_dump_tree(struct cbaa_node *node, unsigned long pxor, void *last,
                    int level,
                    void (*node_dump)(struct cbaa_node *node, int level),
                    void (*leaf_dump)(struct cbaa_node *node, int level));

#endif /* _CBAATREE_H */
