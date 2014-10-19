/* Mapping of generic ebtree code to relative long pointer code ("ebl").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBLMBTREE_H
#define _EBLMBTREE_H

/* redefine entries for mapping */
#undef ebxmb_node
#define ebxmb_node eblmb_node

#undef ebxmb_first
#define ebxmb_first eblmb_first
#undef ebxmb_last
#define ebxmb_last eblmb_last
#undef ebxmb_next
#define ebxmb_next eblmb_next
#undef ebxmb_prev
#define ebxmb_prev eblmb_prev
#undef ebxmb_next_dup
#define ebxmb_next_dup eblmb_next_dup
#undef ebxmb_prev_dup
#define ebxmb_prev_dup eblmb_prev_dup
#undef ebxmb_next_unique
#define ebxmb_next_unique eblmb_next_unique
#undef ebxmb_prev_unique
#define ebxmb_prev_unique eblmb_prev_unique

#undef __ebxmb_delete
#define __ebxmb_delete __eblmb_delete
#undef __ebxmb_lookup_longest
#define __ebxmb_lookup_longest __eblmb_lookup_longest
#undef __ebxmb_lookup
#define __ebxmb_lookup __eblmb_lookup
#undef __ebxmb_insert
#define __ebxmb_insert __eblmb_insert
#undef __ebxmb_lookup_prefix
#define __ebxmb_lookup_prefix __eblmb_lookup_prefix
#undef __ebxmb_insert_prefix
#define __ebxmb_insert_prefix __eblmb_insert_prefix

/* exported functions below */
#undef ebxmb_delete
#define ebxmb_delete eblmb_delete
#undef ebxmb_lookup_longest
#define ebxmb_lookup_longest eblmb_lookup_longest
#undef ebxmb_lookup
#define ebxmb_lookup eblmb_lookup
#undef ebxmb_insert
#define ebxmb_insert eblmb_insert
#undef ebxmb_lookup_prefix
#define ebxmb_lookup_prefix eblmb_lookup_prefix
#undef ebxmb_insert_prefix
#define ebxmb_insert_prefix eblmb_insert_prefix

#include "ebltree.h"
#include "ebxmbtree.h"

/* and now macros that we define based on generic ones */
#define eblmb_entry(ptr, type, member) ebxmb_entry(ptr, type, member)

#endif
