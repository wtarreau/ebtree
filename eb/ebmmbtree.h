/* Mapping of generic ebtree code to relative medium pointer code ("ebm").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBMMBTREE_H
#define _EBMMBTREE_H

/* redefine entries for mapping */
#undef ebxmb_node
#define ebxmb_node ebmmb_node

#undef ebxmb_first
#define ebxmb_first ebmmb_first
#undef ebxmb_last
#define ebxmb_last ebmmb_last
#undef ebxmb_next
#define ebxmb_next ebmmb_next
#undef ebxmb_prev
#define ebxmb_prev ebmmb_prev
#undef ebxmb_next_dup
#define ebxmb_next_dup ebmmb_next_dup
#undef ebxmb_prev_dup
#define ebxmb_prev_dup ebmmb_prev_dup
#undef ebxmb_next_unique
#define ebxmb_next_unique ebmmb_next_unique
#undef ebxmb_prev_unique
#define ebxmb_prev_unique ebmmb_prev_unique

#undef __ebxmb_delete
#define __ebxmb_delete __ebmmb_delete
#undef __ebxmb_lookup_longest
#define __ebxmb_lookup_longest __ebmmb_lookup_longest
#undef __ebxmb_lookup
#define __ebxmb_lookup __ebmmb_lookup
#undef __ebxmb_insert
#define __ebxmb_insert __ebmmb_insert
#undef __ebxmb_lookup_prefix
#define __ebxmb_lookup_prefix __ebmmb_lookup_prefix
#undef __ebxmb_insert_prefix
#define __ebxmb_insert_prefix __ebmmb_insert_prefix

/* exported functions below */
#undef ebxmb_delete
#define ebxmb_delete ebmmb_delete
#undef ebxmb_lookup_longest
#define ebxmb_lookup_longest ebmmb_lookup_longest
#undef ebxmb_lookup
#define ebxmb_lookup ebmmb_lookup
#undef ebxmb_insert
#define ebxmb_insert ebmmb_insert
#undef ebxmb_lookup_prefix
#define ebxmb_lookup_prefix ebmmb_lookup_prefix
#undef ebxmb_insert_prefix
#define ebxmb_insert_prefix ebmmb_insert_prefix

#include "ebmtree.h"
#include "ebxmbtree.h"

/* and now macros that we define based on generic ones */
#define ebmmb_entry(ptr, type, member) ebxmb_entry(ptr, type, member)

#endif
