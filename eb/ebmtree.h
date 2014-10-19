/* Mapping of generic ebtree code to relative medium pointer code ("ebm").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBMTREE_H
#define _EBMTREE_H

#undef EB_SIZE
#define EB_SIZE 32

/* redefine entries for mapping */
#undef ebx_root
#define ebx_root ebm_root
#undef ebx_troot_t
#define ebx_troot_t ebm_troot_t
#undef ebx_node
#define ebx_node ebm_node
#undef ebx_link_t
#define ebx_link_t ebm_link_t

#undef ebx_is_empty
#define ebx_is_empty ebm_is_empty
#undef ebx_is_dup
#define ebx_is_dup ebm_is_dup

#undef ebx_untag
#define ebx_untag ebm_untag
#undef ebx_dotag
#define ebx_dotag ebm_dotag
#undef ebx_clrtag
#define ebx_clrtag ebm_clrtag
#undef ebx_gettag
#define ebx_gettag ebm_gettag

#undef ebx_root_to_node
#define ebx_root_to_node ebm_root_to_node
#undef ebx_setlink
#define ebx_setlink ebm_setlink
#undef ebx_getroot
#define ebx_getroot ebm_getroot
#undef ebx_link_is_null
#define ebx_link_is_null ebm_link_is_null

#undef ebx_first
#define ebx_first ebm_first
#undef ebx_last
#define ebx_last ebm_last
#undef ebx_next
#define ebx_next ebm_next
#undef ebx_next_dup
#define ebx_next_dup ebm_next_dup
#undef ebx_next_unique
#define ebx_next_unique ebm_next_unique
#undef ebx_prev
#define ebx_prev ebm_prev
#undef ebx_prev_dup
#define ebx_prev_dup ebm_prev_dup
#undef ebx_prev_unique
#define ebx_prev_unique ebm_prev_unique
#undef ebx_walk_down
#define ebx_walk_down ebm_walk_down

#undef __ebx_delete
#define __ebx_delete __ebm_delete
#undef __ebx_insert_dup
#define __ebx_insert_dup __ebm_insert_dup

/* these ones are exported functions */
#undef ebx_delete
#define ebx_delete ebm_delete
#undef ebx_insert_dup
#define ebx_insert_dup ebm_insert_dup

#include "ebxtree.h"

/* and now macros that we define based on generic ones */
#define ebm_entry(ptr, type, member) ebx_entry(ptr, type, member)

#endif
