/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBPTTREE_H
#define _EBPTTREE_H

/* redefine entries for mapping */
#undef ebxpt_node
#define ebxpt_node ebpt_node

#undef ebxpt_first
#define ebxpt_first ebpt_first
#undef ebxpt_last
#define ebxpt_last ebpt_last
#undef ebxpt_next
#define ebxpt_next ebpt_next
#undef ebxpt_prev
#define ebxpt_prev ebpt_prev
#undef ebxpt_next_dup
#define ebxpt_next_dup ebpt_next_dup
#undef ebxpt_prev_dup
#define ebxpt_prev_dup ebpt_prev_dup
#undef ebxpt_next_unique
#define ebxpt_next_unique ebpt_next_unique
#undef ebxpt_prev_unique
#define ebxpt_prev_unique ebpt_prev_unique

#undef __ebxpt_delete
#define __ebxpt_delete __ebpt_delete
#undef __ebxpt_lookup
#define __ebxpt_lookup __ebpt_lookup
#undef __ebxpt_insert
#define __ebxpt_insert __ebpt_insert

#undef ebxpt_delete
#define ebxpt_delete ebpt_delete
#undef ebxpt_lookup
#define ebxpt_lookup ebpt_lookup
#undef ebxpt_insert
#define ebxpt_insert ebpt_insert
#undef ebxpt_lookup_le
#define ebxpt_lookup_le ebpt_lookup_le
#undef ebxpt_lookup_ge
#define ebxpt_lookup_ge ebpt_lookup_ge

#include "ebtree.h"
#include "ebxpttree.h"

/* and now macros that we define based on generic ones */
#define ebpt_entry(ptr, type, member) ebxpt_entry(ptr, type, member)

#endif
