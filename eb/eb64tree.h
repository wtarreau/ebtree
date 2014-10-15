/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EB64TREE_H
#define _EB64TREE_H

/* redefine entries for mapping */
#undef ebx64_node
#define ebx64_node eb64_node

#undef ebx64_first
#define ebx64_first eb64_first
#undef ebx64_last
#define ebx64_last eb64_last
#undef ebx64_next
#define ebx64_next eb64_next
#undef ebx64_prev
#define ebx64_prev eb64_prev
#undef ebx64_next_dup
#define ebx64_next_dup eb64_next_dup
#undef ebx64_prev_dup
#define ebx64_prev_dup eb64_prev_dup
#undef ebx64_next_unique
#define ebx64_next_unique eb64_next_unique
#undef ebx64_prev_unique
#define ebx64_prev_unique eb64_prev_unique

#undef __ebx64_delete
#define __ebx64_delete __eb64_delete
#undef __ebx64_lookup
#define __ebx64_lookup __eb64_lookup
#undef __ebx64i_lookup
#define __ebx64i_lookup __eb64i_lookup
#undef __ebx64_insert
#define __ebx64_insert __eb64_insert
#undef __ebx64i_insert
#define __ebx64i_insert __eb64i_insert

#include "ebtree.h"
#include "ebx64tree.h"

/* and now macros that we define based on generic ones */
#define eb64_entry(ptr, type, member) ebx64_entry(ptr, type, member)

#endif
