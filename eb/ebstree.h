/* Mapping of generic ebtree code to relative short pointer code ("ebs").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBSTREE_H
#define _EBSTREE_H

#undef EB_SIZE
#define EB_SIZE 16

/* redefine entries for mapping */
#undef ebx_root
#define ebx_root ebs_root
#undef ebx_troot_t
#define ebx_troot_t ebs_troot_t
#undef ebx_node
#define ebx_node ebs_node
#undef ebx_link_t
#define ebx_link_t ebs_link_t

/* The root of a tree is an ebs_root initialized with both pointers NULL.
 * During its life, only the left pointer will change. The right one will
 * always remain NULL, which is the way we detect it.
 */
#define EBS_ROOT						\
	(struct ebs_root) {				\
		.b = {[0] = 0, [1] = 0 },		\
	}

#define EBS_ROOT_UNIQUE					\
	(struct ebs_root) {				\
		.b = {[0] = 0, [1] = (ebs_link_t)1 },	\
	}

#define EBS_TREE_HEAD(name)				\
	struct ebs_root name = EBS_ROOT

#undef ebx_is_empty
#define ebx_is_empty ebs_is_empty
#undef ebx_is_dup
#define ebx_is_dup ebs_is_dup

#undef ebx_untag
#define ebx_untag ebs_untag
#undef ebx_dotag
#define ebx_dotag ebs_dotag
#undef ebx_clrtag
#define ebx_clrtag ebs_clrtag
#undef ebx_gettag
#define ebx_gettag ebs_gettag

#undef ebx_root_to_node
#define ebx_root_to_node ebs_root_to_node
#undef ebx_setlink
#define ebx_setlink ebs_setlink
#undef ebx_getroot
#define ebx_getroot ebs_getroot
#undef ebx_link_is_null
#define ebx_link_is_null ebs_link_is_null

#undef ebx_first
#define ebx_first ebs_first
#undef ebx_last
#define ebx_last ebs_last
#undef ebx_next
#define ebx_next ebs_next
#undef ebx_next_dup
#define ebx_next_dup ebs_next_dup
#undef ebx_next_unique
#define ebx_next_unique ebs_next_unique
#undef ebx_prev
#define ebx_prev ebs_prev
#undef ebx_prev_dup
#define ebx_prev_dup ebs_prev_dup
#undef ebx_prev_unique
#define ebx_prev_unique ebs_prev_unique
#undef ebx_walk_down
#define ebx_walk_down ebs_walk_down

#undef __ebx_delete
#define __ebx_delete __ebs_delete
#undef __ebx_insert_dup
#define __ebx_insert_dup __ebs_insert_dup

/* these ones are exported functions */
#undef ebx_delete
#define ebx_delete ebs_delete
#undef ebx_insert_dup
#define ebx_insert_dup ebs_insert_dup

#include "ebxtree.h"

/* and now macros that we define based on generic ones */
#define ebs_entry(ptr, type, member) ebx_entry(ptr, type, member)

#endif
