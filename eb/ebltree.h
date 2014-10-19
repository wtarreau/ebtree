/* Mapping of generic ebtree code to relative long pointer code ("ebl").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBLTREE_H
#define _EBLTREE_H

#undef EB_SIZE
#define EB_SIZE 64

/* redefine entries for mapping */
#undef ebx_root
#define ebx_root ebl_root
#undef ebx_troot_t
#define ebx_troot_t ebl_troot_t
#undef ebx_node
#define ebx_node ebl_node
#undef ebx_link_t
#define ebx_link_t ebl_link_t

#undef ebx_is_empty
#define ebx_is_empty ebl_is_empty
#undef ebx_is_dup
#define ebx_is_dup ebl_is_dup

#undef ebx_untag
#define ebx_untag ebl_untag
#undef ebx_dotag
#define ebx_dotag ebl_dotag
#undef ebx_clrtag
#define ebx_clrtag ebl_clrtag
#undef ebx_gettag
#define ebx_gettag ebl_gettag

#undef ebx_root_to_node
#define ebx_root_to_node ebl_root_to_node
#undef ebx_setlink
#define ebx_setlink ebl_setlink
#undef ebx_getroot
#define ebx_getroot ebl_getroot
#undef ebx_link_is_null
#define ebx_link_is_null ebl_link_is_null

#undef ebx_first
#define ebx_first ebl_first
#undef ebx_last
#define ebx_last ebl_last
#undef ebx_next
#define ebx_next ebl_next
#undef ebx_next_dup
#define ebx_next_dup ebl_next_dup
#undef ebx_next_unique
#define ebx_next_unique ebl_next_unique
#undef ebx_prev
#define ebx_prev ebl_prev
#undef ebx_prev_dup
#define ebx_prev_dup ebl_prev_dup
#undef ebx_prev_unique
#define ebx_prev_unique ebl_prev_unique
#undef ebx_walk_down
#define ebx_walk_down ebl_walk_down

#undef __ebx_delete
#define __ebx_delete __ebl_delete
#undef __ebx_insert_dup
#define __ebx_insert_dup __ebl_insert_dup

/* these ones are exported functions */
#undef ebx_delete
#define ebx_delete ebl_delete
#undef ebx_insert_dup
#define ebx_insert_dup ebl_insert_dup

#include "ebxtree.h"

/* and now macros that we define based on generic ones */
#define ebl_entry(ptr, type, member) ebx_entry(ptr, type, member)

#endif
