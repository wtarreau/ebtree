/* Mapping of generic ebtree code to relative medium pointer code ("ebm").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBMPTTREE_H
#define _EBMPTTREE_H

/* redefine entries for mapping */
#undef ebxpt_node
#define ebxpt_node ebmpt_node

#undef ebxpt_first
#define ebxpt_first ebmpt_first
#undef ebxpt_last
#define ebxpt_last ebmpt_last
#undef ebxpt_next
#define ebxpt_next ebmpt_next
#undef ebxpt_prev
#define ebxpt_prev ebmpt_prev
#undef ebxpt_next_dup
#define ebxpt_next_dup ebmpt_next_dup
#undef ebxpt_prev_dup
#define ebxpt_prev_dup ebmpt_prev_dup
#undef ebxpt_next_unique
#define ebxpt_next_unique ebmpt_next_unique
#undef ebxpt_prev_unique
#define ebxpt_prev_unique ebmpt_prev_unique

#undef __ebxpt_delete
#define __ebxpt_delete __ebmpt_delete
#undef __ebxpt_lookup
#define __ebxpt_lookup __ebmpt_lookup
#undef __ebxpt_insert
#define __ebxpt_insert __ebmpt_insert

#undef ebxpt_delete
#define ebxpt_delete ebmpt_delete
#undef ebxpt_lookup
#define ebxpt_lookup ebmpt_lookup
#undef ebxpt_insert
#define ebxpt_insert ebmpt_insert
#undef ebxpt_lookup_le
#define ebxpt_lookup_le ebmpt_lookup_le
#undef ebxpt_lookup_ge
#define ebxpt_lookup_ge ebmpt_lookup_ge

#include "ebmtree.h"
#include "ebxpttree.h"

/* and now macros that we define based on generic ones */
#define ebmpt_entry(ptr, type, member) ebxpt_entry(ptr, type, member)

#endif
