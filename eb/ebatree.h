/* Mapping of generic ebtree code to absolute pointer code ("eba").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBATREE_H
#define _EBATREE_H

#undef EB_SIZE
#define EB_SIZE 0

/* redefine entries for mapping */
#undef ebx_root
#define ebx_root eba_root
#undef ebx_troot_t
#define ebx_troot_t eba_troot_t
#undef ebx_node
#define ebx_node eba_node
#undef ebx_link_t
#define ebx_link_t eba_link_t

/* The root of a tree is an eba_root initialized with both pointers NULL.
 * During its life, only the left pointer will change. The right one will
 * always remain NULL, which is the way we detect it.
 */
#define EBA_ROOT						\
	(struct eba_root) {				\
		.b = {[0] = 0, [1] = 0 },		\
	}

#define EBA_ROOT_UNIQUE					\
	(struct eba_root) {				\
		.b = {[0] = 0, [1] = (eba_link_t)1 },	\
	}

#define EBA_TREE_HEAD(name)				\
	struct eba_root name = EBA_ROOT

#undef ebx_is_empty
#define ebx_is_empty eba_is_empty
#undef ebx_is_dup
#define ebx_is_dup eba_is_dup

#undef ebx_untag
#define ebx_untag eba_untag
#undef ebx_dotag
#define ebx_dotag eba_dotag
#undef ebx_clrtag
#define ebx_clrtag eba_clrtag
#undef ebx_gettag
#define ebx_gettag eba_gettag

#undef ebx_root_to_node
#define ebx_root_to_node eba_root_to_node
#undef ebx_setlink
#define ebx_setlink eba_setlink
#undef ebx_getroot
#define ebx_getroot eba_getroot
#undef ebx_link_is_null
#define ebx_link_is_null eba_link_is_null

#undef ebx_first
#define ebx_first eba_first
#undef ebx_last
#define ebx_last eba_last
#undef ebx_next
#define ebx_next eba_next
#undef ebx_next_dup
#define ebx_next_dup eba_next_dup
#undef ebx_next_unique
#define ebx_next_unique eba_next_unique
#undef ebx_prev
#define ebx_prev eba_prev
#undef ebx_prev_dup
#define ebx_prev_dup eba_prev_dup
#undef ebx_prev_unique
#define ebx_prev_unique eba_prev_unique
#undef ebx_walk_down
#define ebx_walk_down eba_walk_down

#undef __ebx_delete
#define __ebx_delete __eba_delete
#undef __ebx_insert_dup
#define __ebx_insert_dup __eba_insert_dup

/* these ones are exported functions */
#undef ebx_delete
#define ebx_delete eba_delete
#undef ebx_insert_dup
#define ebx_insert_dup eba_insert_dup

#include "ebxtree.h"

/* and now macros that we define based on generic ones */
#define eba_entry(ptr, type, member) ebx_entry(ptr, type, member)

#endif
