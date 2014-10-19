/* Mapping of generic ebtree code to relative long pointer code ("ebl").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBL64TREE_H
#define _EBL64TREE_H

/* redefine entries for mapping */
#undef ebx64_node
#define ebx64_node ebl64_node

#undef ebx64_first
#define ebx64_first ebl64_first
#undef ebx64_last
#define ebx64_last ebl64_last
#undef ebx64_next
#define ebx64_next ebl64_next
#undef ebx64_prev
#define ebx64_prev ebl64_prev
#undef ebx64_next_dup
#define ebx64_next_dup ebl64_next_dup
#undef ebx64_prev_dup
#define ebx64_prev_dup ebl64_prev_dup
#undef ebx64_next_unique
#define ebx64_next_unique ebl64_next_unique
#undef ebx64_prev_unique
#define ebx64_prev_unique ebl64_prev_unique

#undef __ebx64_delete
#define __ebx64_delete __ebl64_delete
#undef __ebx64_lookup
#define __ebx64_lookup __ebl64_lookup
#undef __ebx64i_lookup
#define __ebx64i_lookup __ebl64i_lookup
#undef __ebx64_insert
#define __ebx64_insert __ebl64_insert
#undef __ebx64i_insert
#define __ebx64i_insert __ebl64i_insert

/* exported functions below */
#undef ebx64_delete
#define ebx64_delete ebl64_delete
#undef ebx64_lookup
#define ebx64_lookup ebl64_lookup
#undef ebx64i_lookup
#define ebx64i_lookup ebl64i_lookup
#undef ebx64_insert
#define ebx64_insert ebl64_insert
#undef ebx64i_insert
#define ebx64i_insert ebl64i_insert
#undef ebx64_lookup_le
#define ebx64_lookup_le ebl64_lookup_le
#undef ebx64_lookup_ge
#define ebx64_lookup_ge ebl64_lookup_ge

#include "ebltree.h"
#include "ebx64tree.h"

/* and now macros that we define based on generic ones */
#define ebl64_entry(ptr, type, member) ebx64_entry(ptr, type, member)

#endif
