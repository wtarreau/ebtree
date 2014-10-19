/* Mapping of generic ebtree code to relative long pointer code ("ebl").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBLPTTREE_H
#define _EBLPTTREE_H

/* redefine entries for mapping */
#undef ebxpt_node
#define ebxpt_node eblpt_node

#undef ebxpt_first
#define ebxpt_first eblpt_first
#undef ebxpt_last
#define ebxpt_last eblpt_last
#undef ebxpt_next
#define ebxpt_next eblpt_next
#undef ebxpt_prev
#define ebxpt_prev eblpt_prev
#undef ebxpt_next_dup
#define ebxpt_next_dup eblpt_next_dup
#undef ebxpt_prev_dup
#define ebxpt_prev_dup eblpt_prev_dup
#undef ebxpt_next_unique
#define ebxpt_next_unique eblpt_next_unique
#undef ebxpt_prev_unique
#define ebxpt_prev_unique eblpt_prev_unique

#undef __ebxpt_delete
#define __ebxpt_delete __eblpt_delete
#undef __ebxpt_lookup
#define __ebxpt_lookup __eblpt_lookup
#undef __ebxpt_insert
#define __ebxpt_insert __eblpt_insert

#undef ebxpt_delete
#define ebxpt_delete eblpt_delete
#undef ebxpt_lookup
#define ebxpt_lookup eblpt_lookup
#undef ebxpt_insert
#define ebxpt_insert eblpt_insert
#undef ebxpt_lookup_le
#define ebxpt_lookup_le eblpt_lookup_le
#undef ebxpt_lookup_ge
#define ebxpt_lookup_ge eblpt_lookup_ge

#include "ebltree.h"
#include "ebxpttree.h"

/* and now macros that we define based on generic ones */
#define eblpt_entry(ptr, type, member) ebxpt_entry(ptr, type, member)

#endif
