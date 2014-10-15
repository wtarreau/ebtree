/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBMBTREE_H
#define _EBMBTREE_H

/* redefine entries for mapping */
#undef ebxmb_node
#define ebxmb_node ebmb_node

#undef ebxmb_first
#define ebxmb_first ebmb_first
#undef ebxmb_last
#define ebxmb_last ebmb_last
#undef ebxmb_next
#define ebxmb_next ebmb_next
#undef ebxmb_prev
#define ebxmb_prev ebmb_prev
#undef ebxmb_next_dup
#define ebxmb_next_dup ebmb_next_dup
#undef ebxmb_prev_dup
#define ebxmb_prev_dup ebmb_prev_dup
#undef ebxmb_next_unique
#define ebxmb_next_unique ebmb_next_unique
#undef ebxmb_prev_unique
#define ebxmb_prev_unique ebmb_prev_unique

#undef __ebxmb_delete
#define __ebxmb_delete __ebmb_delete
#undef __ebxmb_lookup_longest
#define __ebxmb_lookup_longest __ebmb_lookup_longest
#undef __ebxmb_lookup
#define __ebxmb_lookup __ebmb_lookup
#undef __ebxmb_insert
#define __ebxmb_insert __ebmb_insert
#undef __ebxmb_lookup_prefix
#define __ebxmb_lookup_prefix __ebmb_lookup_prefix
#undef __ebxmb_insert_prefix
#define __ebxmb_insert_prefix __ebmb_insert_prefix

/* exported functions below */
#undef ebxmb_delete
#define ebxmb_delete ebmb_delete
#undef ebxmb_lookup_longest
#define ebxmb_lookup_longest ebmb_lookup_longest
#undef ebxmb_lookup
#define ebxmb_lookup ebmb_lookup
#undef ebxmb_insert
#define ebxmb_insert ebmb_insert
#undef ebxmb_lookup_prefix
#define ebxmb_lookup_prefix ebmb_lookup_prefix
#undef ebxmb_insert_prefix
#define ebxmb_insert_prefix ebmb_insert_prefix

#include "ebtree.h"
#include "ebxmbtree.h"

/* and now macros that we define based on generic ones */
#define ebmb_entry(ptr, type, member) ebxmb_entry(ptr, type, member)

#endif
