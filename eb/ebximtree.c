/*
 * Elastic Binary Trees - exported functions for Indirect Multi-Byte data nodes.
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

/* Consult ebimtree.h for more details about those functions */

/* Find the first occurence of a key of <len> bytes in the tree <root>.
 * If none can be found, return NULL.
 */
REGPRM3 struct ebxpt_node *
ebxim_lookup(struct ebx_root *root, const void *x, unsigned int len)
{
	return __ebxim_lookup(root, x, len);
}

/* Insert ebxpt_node <new> into subtree starting at node root <root>.
 * Only new->key needs be set with the key. The ebxpt_node is returned.
 * If root->b[EB_SIDE_RGHT]==1, the tree may only contain unique keys. The
 * len is specified in bytes.
 */
REGPRM3 struct ebxpt_node *
ebxim_insert(struct ebx_root *root, struct ebxpt_node *new, unsigned int len)
{
	return __ebxim_insert(root, new, len);
}
