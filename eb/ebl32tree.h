/* Mapping of generic ebtree code to relative long pointer code ("ebl").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBL32TREE_H
#define _EBL32TREE_H

/* redefine entries for mapping */
#undef ebx32_node
#define ebx32_node ebl32_node

#undef ebx32_first
#define ebx32_first ebl32_first
#undef ebx32_last
#define ebx32_last ebl32_last
#undef ebx32_next
#define ebx32_next ebl32_next
#undef ebx32_prev
#define ebx32_prev ebl32_prev
#undef ebx32_next_dup
#define ebx32_next_dup ebl32_next_dup
#undef ebx32_prev_dup
#define ebx32_prev_dup ebl32_prev_dup
#undef ebx32_next_unique
#define ebx32_next_unique ebl32_next_unique
#undef ebx32_prev_unique
#define ebx32_prev_unique ebl32_prev_unique

#undef __ebx32_delete
#define __ebx32_delete __ebl32_delete
#undef __ebx32_lookup
#define __ebx32_lookup __ebl32_lookup
#undef __ebx32i_lookup
#define __ebx32i_lookup __ebl32i_lookup
#undef __ebx32_insert
#define __ebx32_insert __ebl32_insert
#undef __ebx32i_insert
#define __ebx32i_insert __ebl32i_insert

/* exported functions below */
#undef ebx32_delete
#define ebx32_delete ebl32_delete
#undef ebx32_lookup
#define ebx32_lookup ebl32_lookup
#undef ebx32i_lookup
#define ebx32i_lookup ebl32i_lookup
#undef ebx32_insert
#define ebx32_insert ebl32_insert
#undef ebx32i_insert
#define ebx32i_insert ebl32i_insert
#undef ebx32_lookup_le
#define ebx32_lookup_le ebl32_lookup_le
#undef ebx32_lookup_ge
#define ebx32_lookup_ge ebl32_lookup_ge

#include "ebltree.h"
#include "ebx32tree.h"

/* and now macros that we define based on generic ones */
#define ebl32_entry(ptr, type, member) ebx32_entry(ptr, type, member)

#endif
