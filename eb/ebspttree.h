/* Mapping of generic ebtree code to relative short pointer code ("ebs").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBSPTTREE_H
#define _EBSPTTREE_H

/* redefine entries for mapping */
#undef ebxpt_node
#define ebxpt_node ebspt_node

#undef ebxpt_first
#define ebxpt_first ebspt_first
#undef ebxpt_last
#define ebxpt_last ebspt_last
#undef ebxpt_next
#define ebxpt_next ebspt_next
#undef ebxpt_prev
#define ebxpt_prev ebspt_prev
#undef ebxpt_next_dup
#define ebxpt_next_dup ebspt_next_dup
#undef ebxpt_prev_dup
#define ebxpt_prev_dup ebspt_prev_dup
#undef ebxpt_next_unique
#define ebxpt_next_unique ebspt_next_unique
#undef ebxpt_prev_unique
#define ebxpt_prev_unique ebspt_prev_unique

#undef __ebxpt_delete
#define __ebxpt_delete __ebspt_delete
#undef __ebxpt_lookup
#define __ebxpt_lookup __ebspt_lookup
#undef __ebxpt_insert
#define __ebxpt_insert __ebspt_insert

#undef ebxpt_delete
#define ebxpt_delete ebspt_delete
#undef ebxpt_lookup
#define ebxpt_lookup ebspt_lookup
#undef ebxpt_insert
#define ebxpt_insert ebspt_insert
#undef ebxpt_lookup_le
#define ebxpt_lookup_le ebspt_lookup_le
#undef ebxpt_lookup_ge
#define ebxpt_lookup_ge ebspt_lookup_ge

#include "ebstree.h"
#include "ebxpttree.h"

/* and now macros that we define based on generic ones */
#define ebspt_entry(ptr, type, member) ebxpt_entry(ptr, type, member)

#endif
