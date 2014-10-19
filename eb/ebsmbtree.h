/* Mapping of generic ebtree code to relative short pointer code ("ebs").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBSMBTREE_H
#define _EBSMBTREE_H

/* redefine entries for mapping */
#undef ebxmb_node
#define ebxmb_node ebsmb_node

#undef ebxmb_first
#define ebxmb_first ebsmb_first
#undef ebxmb_last
#define ebxmb_last ebsmb_last
#undef ebxmb_next
#define ebxmb_next ebsmb_next
#undef ebxmb_prev
#define ebxmb_prev ebsmb_prev
#undef ebxmb_next_dup
#define ebxmb_next_dup ebsmb_next_dup
#undef ebxmb_prev_dup
#define ebxmb_prev_dup ebsmb_prev_dup
#undef ebxmb_next_unique
#define ebxmb_next_unique ebsmb_next_unique
#undef ebxmb_prev_unique
#define ebxmb_prev_unique ebsmb_prev_unique

#undef __ebxmb_delete
#define __ebxmb_delete __ebsmb_delete
#undef __ebxmb_lookup_longest
#define __ebxmb_lookup_longest __ebsmb_lookup_longest
#undef __ebxmb_lookup
#define __ebxmb_lookup __ebsmb_lookup
#undef __ebxmb_insert
#define __ebxmb_insert __ebsmb_insert
#undef __ebxmb_lookup_prefix
#define __ebxmb_lookup_prefix __ebsmb_lookup_prefix
#undef __ebxmb_insert_prefix
#define __ebxmb_insert_prefix __ebsmb_insert_prefix

/* exported functions below */
#undef ebxmb_delete
#define ebxmb_delete ebsmb_delete
#undef ebxmb_lookup_longest
#define ebxmb_lookup_longest ebsmb_lookup_longest
#undef ebxmb_lookup
#define ebxmb_lookup ebsmb_lookup
#undef ebxmb_insert
#define ebxmb_insert ebsmb_insert
#undef ebxmb_lookup_prefix
#define ebxmb_lookup_prefix ebsmb_lookup_prefix
#undef ebxmb_insert_prefix
#define ebxmb_insert_prefix ebsmb_insert_prefix

#include "ebstree.h"
#include "ebxmbtree.h"

/* and now macros that we define based on generic ones */
#define ebsmb_entry(ptr, type, member) ebxmb_entry(ptr, type, member)

#endif
