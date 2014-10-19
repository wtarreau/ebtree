/* Mapping of generic ebtree code to absolute pointer code ("eba").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBAPTTREE_H
#define _EBAPTTREE_H

/* redefine entries for mapping */
#undef ebxpt_node
#define ebxpt_node ebapt_node

#undef ebxpt_first
#define ebxpt_first ebapt_first
#undef ebxpt_last
#define ebxpt_last ebapt_last
#undef ebxpt_next
#define ebxpt_next ebapt_next
#undef ebxpt_prev
#define ebxpt_prev ebapt_prev
#undef ebxpt_next_dup
#define ebxpt_next_dup ebapt_next_dup
#undef ebxpt_prev_dup
#define ebxpt_prev_dup ebapt_prev_dup
#undef ebxpt_next_unique
#define ebxpt_next_unique ebapt_next_unique
#undef ebxpt_prev_unique
#define ebxpt_prev_unique ebapt_prev_unique

#undef __ebxpt_delete
#define __ebxpt_delete __ebapt_delete
#undef __ebxpt_lookup
#define __ebxpt_lookup __ebapt_lookup
#undef __ebxpt_insert
#define __ebxpt_insert __ebapt_insert

#undef ebxpt_delete
#define ebxpt_delete ebapt_delete
#undef ebxpt_lookup
#define ebxpt_lookup ebapt_lookup
#undef ebxpt_insert
#define ebxpt_insert ebapt_insert
#undef ebxpt_lookup_le
#define ebxpt_lookup_le ebapt_lookup_le
#undef ebxpt_lookup_ge
#define ebxpt_lookup_ge ebapt_lookup_ge

#include "ebatree.h"
#include "ebxpttree.h"

/* and now macros that we define based on generic ones */
#define ebapt_entry(ptr, type, member) ebxpt_entry(ptr, type, member)

#endif
