/* Mapping of generic ebtree code to relative short pointer code ("ebs").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBS64TREE_H
#define _EBS64TREE_H

/* redefine entries for mapping */
#undef ebx64_node
#define ebx64_node ebs64_node

#undef ebx64_first
#define ebx64_first ebs64_first
#undef ebx64_last
#define ebx64_last ebs64_last
#undef ebx64_next
#define ebx64_next ebs64_next
#undef ebx64_prev
#define ebx64_prev ebs64_prev
#undef ebx64_next_dup
#define ebx64_next_dup ebs64_next_dup
#undef ebx64_prev_dup
#define ebx64_prev_dup ebs64_prev_dup
#undef ebx64_next_unique
#define ebx64_next_unique ebs64_next_unique
#undef ebx64_prev_unique
#define ebx64_prev_unique ebs64_prev_unique

#undef __ebx64_delete
#define __ebx64_delete __ebs64_delete
#undef __ebx64_lookup
#define __ebx64_lookup __ebs64_lookup
#undef __ebx64i_lookup
#define __ebx64i_lookup __ebs64i_lookup
#undef __ebx64_insert
#define __ebx64_insert __ebs64_insert
#undef __ebx64i_insert
#define __ebx64i_insert __ebs64i_insert

/* exported functions below */
#undef ebx64_delete
#define ebx64_delete ebs64_delete
#undef ebx64_lookup
#define ebx64_lookup ebs64_lookup
#undef ebx64i_lookup
#define ebx64i_lookup ebs64i_lookup
#undef ebx64_insert
#define ebx64_insert ebs64_insert
#undef ebx64i_insert
#define ebx64i_insert ebs64i_insert
#undef ebx64_lookup_le
#define ebx64_lookup_le ebs64_lookup_le
#undef ebx64_lookup_ge
#define ebx64_lookup_ge ebs64_lookup_ge

#include "ebstree.h"
#include "ebx64tree.h"

/* and now macros that we define based on generic ones */
#define ebs64_entry(ptr, type, member) ebx64_entry(ptr, type, member)

#endif
