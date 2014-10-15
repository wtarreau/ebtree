/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBTREE_H
#define _EBTREE_H

/* redefine entries for mapping */
#undef ebx_root
#define ebx_root eb_root
#undef ebx_troot_t
#define ebx_troot_t eb_troot_t
#undef ebx_node
#define ebx_node eb_node
#undef ebx_link_t
#define ebx_link_t eb_link_t

#undef ebx_is_empty
#define ebx_is_empty eb_is_empty
#undef ebx_is_dup
#define ebx_is_dup eb_is_dup

#undef ebx_untag
#define ebx_untag eb_untag
#undef ebx_dotag
#define ebx_dotag eb_dotag
#undef ebx_clrtag
#define ebx_clrtag eb_clrtag
#undef ebx_gettag
#define ebx_gettag eb_gettag

#undef ebx_first
#define ebx_first eb_first
#undef ebx_last
#define ebx_last eb_last
#undef ebx_next
#define ebx_next eb_next
#undef ebx_next_dup
#define ebx_next_dup eb_next_dup
#undef ebx_next_unique
#define ebx_next_unique eb_next_unique
#undef ebx_prev
#define ebx_prev eb_prev
#undef ebx_prev_dup
#define ebx_prev_dup eb_prev_dup
#undef ebx_prev_unique
#define ebx_prev_unique eb_prev_unique
#undef ebx_walk_down
#define ebx_walk_down eb_walk_down

#undef __ebx_delete
#define __ebx_delete __eb_delete
#undef __ebx_insert_dup
#define __ebx_insert_dup __eb_insert_dup

/* these ones are exported functions */
#undef ebx_delete
#define ebx_delete eb_delete
#undef ebx_insert_dup
#define ebx_insert_dup eb_insert_dup

#include "ebxtree.h"

/* and now macros that we define based on generic ones */
#define eb_entry(ptr, type, member) ebx_entry(ptr, type, member)

#endif
