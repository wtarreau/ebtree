/* Mapping of generic ebtree code to relative short pointer code ("ebs").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBS32TREE_H
#define _EBS32TREE_H

/* redefine entries for mapping */
#undef ebx32_node
#define ebx32_node ebs32_node

#undef ebx32_first
#define ebx32_first ebs32_first
#undef ebx32_last
#define ebx32_last ebs32_last
#undef ebx32_next
#define ebx32_next ebs32_next
#undef ebx32_prev
#define ebx32_prev ebs32_prev
#undef ebx32_next_dup
#define ebx32_next_dup ebs32_next_dup
#undef ebx32_prev_dup
#define ebx32_prev_dup ebs32_prev_dup
#undef ebx32_next_unique
#define ebx32_next_unique ebs32_next_unique
#undef ebx32_prev_unique
#define ebx32_prev_unique ebs32_prev_unique

#undef __ebx32_delete
#define __ebx32_delete __ebs32_delete
#undef __ebx32_lookup
#define __ebx32_lookup __ebs32_lookup
#undef __ebx32i_lookup
#define __ebx32i_lookup __ebs32i_lookup
#undef __ebx32_insert
#define __ebx32_insert __ebs32_insert
#undef __ebx32i_insert
#define __ebx32i_insert __ebs32i_insert

/* exported functions below */
#undef ebx32_delete
#define ebx32_delete ebs32_delete
#undef ebx32_lookup
#define ebx32_lookup ebs32_lookup
#undef ebx32i_lookup
#define ebx32i_lookup ebs32i_lookup
#undef ebx32_insert
#define ebx32_insert ebs32_insert
#undef ebx32i_insert
#define ebx32i_insert ebs32i_insert
#undef ebx32_lookup_le
#define ebx32_lookup_le ebs32_lookup_le
#undef ebx32_lookup_ge
#define ebx32_lookup_ge ebs32_lookup_ge

#include "ebstree.h"
#include "ebx32tree.h"

/* and now macros that we define based on generic ones */
#define ebs32_entry(ptr, type, member) ebx32_entry(ptr, type, member)

#endif
