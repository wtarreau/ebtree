/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EB32TREE_H
#define _EB32TREE_H

/* redefine entries for mapping */
#undef ebx32_node
#define ebx32_node eb32_node

#undef ebx32_first
#define ebx32_first eb32_first
#undef ebx32_last
#define ebx32_last eb32_last
#undef ebx32_next
#define ebx32_next eb32_next
#undef ebx32_prev
#define ebx32_prev eb32_prev
#undef ebx32_next_dup
#define ebx32_next_dup eb32_next_dup
#undef ebx32_prev_dup
#define ebx32_prev_dup eb32_prev_dup
#undef ebx32_next_unique
#define ebx32_next_unique eb32_next_unique
#undef ebx32_prev_unique
#define ebx32_prev_unique eb32_prev_unique

#undef __ebx32_delete
#define __ebx32_delete __eb32_delete
#undef __ebx32_lookup
#define __ebx32_lookup __eb32_lookup
#undef __ebx32i_lookup
#define __ebx32i_lookup __eb32i_lookup
#undef __ebx32_insert
#define __ebx32_insert __eb32_insert
#undef __ebx32i_insert
#define __ebx32i_insert __eb32i_insert

#include "ebtree.h"
#include "ebx32tree.h"

/* and now macros that we define based on generic ones */
#define eb32_entry(ptr, type, member) ebx32_entry(ptr, type, member)

#endif
