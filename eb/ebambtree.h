/* Mapping of generic ebtree code to absolute pointer code ("eba").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBAMBTREE_H
#define _EBAMBTREE_H

/* redefine entries for mapping */
#undef ebxmb_node
#define ebxmb_node ebamb_node

#undef ebxmb_first
#define ebxmb_first ebamb_first
#undef ebxmb_last
#define ebxmb_last ebamb_last
#undef ebxmb_next
#define ebxmb_next ebamb_next
#undef ebxmb_prev
#define ebxmb_prev ebamb_prev
#undef ebxmb_next_dup
#define ebxmb_next_dup ebamb_next_dup
#undef ebxmb_prev_dup
#define ebxmb_prev_dup ebamb_prev_dup
#undef ebxmb_next_unique
#define ebxmb_next_unique ebamb_next_unique
#undef ebxmb_prev_unique
#define ebxmb_prev_unique ebamb_prev_unique

#undef __ebxmb_delete
#define __ebxmb_delete __ebamb_delete
#undef __ebxmb_lookup_longest
#define __ebxmb_lookup_longest __ebamb_lookup_longest
#undef __ebxmb_lookup
#define __ebxmb_lookup __ebamb_lookup
#undef __ebxmb_insert
#define __ebxmb_insert __ebamb_insert
#undef __ebxmb_lookup_prefix
#define __ebxmb_lookup_prefix __ebamb_lookup_prefix
#undef __ebxmb_insert_prefix
#define __ebxmb_insert_prefix __ebamb_insert_prefix

/* exported functions below */
#undef ebxmb_delete
#define ebxmb_delete ebamb_delete
#undef ebxmb_lookup_longest
#define ebxmb_lookup_longest ebamb_lookup_longest
#undef ebxmb_lookup
#define ebxmb_lookup ebamb_lookup
#undef ebxmb_insert
#define ebxmb_insert ebamb_insert
#undef ebxmb_lookup_prefix
#define ebxmb_lookup_prefix ebamb_lookup_prefix
#undef ebxmb_insert_prefix
#define ebxmb_insert_prefix ebamb_insert_prefix

#include "ebatree.h"
#include "ebxmbtree.h"

/* and now macros that we define based on generic ones */
#define ebamb_entry(ptr, type, member) ebxmb_entry(ptr, type, member)

#endif
