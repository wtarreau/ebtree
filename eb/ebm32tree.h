/* Mapping of generic ebtree code to relative medium pointer code ("ebm").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBM32TREE_H
#define _EBM32TREE_H

/* redefine entries for mapping */
#undef ebx32_node
#define ebx32_node ebm32_node

#undef ebx32_first
#define ebx32_first ebm32_first
#undef ebx32_last
#define ebx32_last ebm32_last
#undef ebx32_next
#define ebx32_next ebm32_next
#undef ebx32_prev
#define ebx32_prev ebm32_prev
#undef ebx32_next_dup
#define ebx32_next_dup ebm32_next_dup
#undef ebx32_prev_dup
#define ebx32_prev_dup ebm32_prev_dup
#undef ebx32_next_unique
#define ebx32_next_unique ebm32_next_unique
#undef ebx32_prev_unique
#define ebx32_prev_unique ebm32_prev_unique

#undef __ebx32_delete
#define __ebx32_delete __ebm32_delete
#undef __ebx32_lookup
#define __ebx32_lookup __ebm32_lookup
#undef __ebx32i_lookup
#define __ebx32i_lookup __ebm32i_lookup
#undef __ebx32_insert
#define __ebx32_insert __ebm32_insert
#undef __ebx32i_insert
#define __ebx32i_insert __ebm32i_insert

/* exported functions below */
#undef ebx32_delete
#define ebx32_delete ebm32_delete
#undef ebx32_lookup
#define ebx32_lookup ebm32_lookup
#undef ebx32i_lookup
#define ebx32i_lookup ebm32i_lookup
#undef ebx32_insert
#define ebx32_insert ebm32_insert
#undef ebx32i_insert
#define ebx32i_insert ebm32i_insert
#undef ebx32_lookup_le
#define ebx32_lookup_le ebm32_lookup_le
#undef ebx32_lookup_ge
#define ebx32_lookup_ge ebm32_lookup_ge

#include "ebmtree.h"
#include "ebx32tree.h"

/* and now macros that we define based on generic ones */
#define ebm32_entry(ptr, type, member) ebx32_entry(ptr, type, member)

#endif
