/* Mapping of generic ebtree code to relative long pointer code ("ebl").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBLTREE_H
#define _EBLTREE_H

#include "ebcommon.h"

/* EB_SIZE = 64 : we're using 64-bit offsets for relative pointers. */
#undef EB_SIZE
#define EB_SIZE 64

/* The root of a tree is an ebl_root initialized with both pointers NULL.
 * During its life, only the left pointer will change. The right one will
 * always remain NULL, which is the way we detect it.
 */
#define EBL_ROOT						\
	(struct ebl_root) {				\
		.b = {[0] = 0, [1] = 0 },		\
	}

#define EBL_ROOT_UNIQUE					\
	(struct ebl_root) {				\
		.b = {[0] = 0, [1] = (ebl_link_t)1 },	\
	}

#define EBL_TREE_HEAD(name)				\
	struct ebl_root name = EBL_ROOT

/* map a few macros to their original names */
#define ebl_entry(ptr, type, member) ebx_entry(ptr, type, member)
#define ebl32_entry(ptr, type, member) ebx32_entry(ptr, type, member)
#define ebl64_entry(ptr, type, member) ebx64_entry(ptr, type, member)
#define eblpt_entry(ptr, type, member) ebxpt_entry(ptr, type, member)
#define eblmb_entry(ptr, type, member) ebxmb_entry(ptr, type, member)

/* we're using 64-bit signed offsets for the links */
typedef s64 ebl_link_t;

/* remap ebxtree.h symbols and types to ebl- equivalent */
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

/* remap ebxtree.c symbols and types to ebl- equivalent */
#undef ebx_delete
#define ebx_delete ebl_delete
#undef ebx_insert_dup
#define ebx_insert_dup ebl_insert_dup

/* remap ebx32tree.h symbols and types to ebl- equivalent */
#undef ebx32_node
#define ebx32_node ebl32_node
#undef ebx32_first
#define ebx32_first ebl32_first
#undef ebx32_last
#define ebx32_last ebl32_last
#undef ebx32_next
#define ebx32_next ebl32_next
#undef ebx32_prev
#define ebx32_prev ebl32_prev
#undef ebx32_next_dup
#define ebx32_next_dup ebl32_next_dup
#undef ebx32_prev_dup
#define ebx32_prev_dup ebl32_prev_dup
#undef ebx32_next_unique
#define ebx32_next_unique ebl32_next_unique
#undef ebx32_prev_unique
#define ebx32_prev_unique ebl32_prev_unique
#undef __ebx32_delete
#define __ebx32_delete __ebl32_delete
#undef __ebx32_lookup
#define __ebx32_lookup __ebl32_lookup
#undef __ebx32i_lookup
#define __ebx32i_lookup __ebl32i_lookup
#undef __ebx32_insert
#define __ebx32_insert __ebl32_insert
#undef __ebx32i_insert
#define __ebx32i_insert __ebl32i_insert

/* remap ebx32tree.c symbols and types to ebl- equivalent */
#undef ebx32_delete
#define ebx32_delete ebl32_delete
#undef ebx32_lookup
#define ebx32_lookup ebl32_lookup
#undef ebx32i_lookup
#define ebx32i_lookup ebl32i_lookup
#undef ebx32_insert
#define ebx32_insert ebl32_insert
#undef ebx32i_insert
#define ebx32i_insert ebl32i_insert
#undef ebx32_lookup_le
#define ebx32_lookup_le ebl32_lookup_le
#undef ebx32_lookup_ge
#define ebx32_lookup_ge ebl32_lookup_ge

/* remap ebx64tree.h symbols and types to ebl- equivalent */
#undef ebx64_node
#define ebx64_node ebl64_node
#undef ebx64_first
#define ebx64_first ebl64_first
#undef ebx64_last
#define ebx64_last ebl64_last
#undef ebx64_next
#define ebx64_next ebl64_next
#undef ebx64_prev
#define ebx64_prev ebl64_prev
#undef ebx64_next_dup
#define ebx64_next_dup ebl64_next_dup
#undef ebx64_prev_dup
#define ebx64_prev_dup ebl64_prev_dup
#undef ebx64_next_unique
#define ebx64_next_unique ebl64_next_unique
#undef ebx64_prev_unique
#define ebx64_prev_unique ebl64_prev_unique
#undef __ebx64_delete
#define __ebx64_delete __ebl64_delete
#undef __ebx64_lookup
#define __ebx64_lookup __ebl64_lookup
#undef __ebx64i_lookup
#define __ebx64i_lookup __ebl64i_lookup
#undef __ebx64_insert
#define __ebx64_insert __ebl64_insert
#undef __ebx64i_insert
#define __ebx64i_insert __ebl64i_insert

/* remap ebx64tree.c symbols and types to ebl- equivalent */
#undef ebx64_delete
#define ebx64_delete ebl64_delete
#undef ebx64_lookup
#define ebx64_lookup ebl64_lookup
#undef ebx64i_lookup
#define ebx64i_lookup ebl64i_lookup
#undef ebx64_insert
#define ebx64_insert ebl64_insert
#undef ebx64i_insert
#define ebx64i_insert ebl64i_insert
#undef ebx64_lookup_le
#define ebx64_lookup_le ebl64_lookup_le
#undef ebx64_lookup_ge
#define ebx64_lookup_ge ebl64_lookup_ge

/* remap ebxpttree.h symbols and types to ebl- equivalent */
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

/* remap ebxmbtree.h symbols and types to ebl- equivalent */
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

/* remap ebxmbtree.c symbols and types to ebl- equivalent */
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

/* remap ebxsttree.h symbols and types to ebl- equivalent */
#undef __ebxst_lookup
#define __ebxst_lookup __eblst_lookup
#undef __ebxst_insert
#define __ebxst_insert __eblst_insert

/* remap ebxsttree.c symbols and types to ebl- equivalent */
#undef ebxst_lookup
#define ebxst_lookup eblst_lookup
#undef ebxst_lookup_len
#define ebxst_lookup_len eblst_lookup_len
#undef ebxst_insert
#define ebxst_insert eblst_insert

/* remap ebximtree.h symbols and types to ebl- equivalent */
#undef __ebxim_lookup
#define __ebxim_lookup __eblim_lookup
#undef __ebxim_insert
#define __ebxim_insert __eblim_insert

/* remap ebximtree.c symbols and types to ebl- equivalent */
#undef ebxim_lookup
#define ebxim_lookup eblim_lookup
#undef ebxim_insert
#define ebxim_insert eblim_insert

/* remap ebxistree.h symbols and types to ebl- equivalent */
#undef __ebxis_lookup
#define __ebxis_lookup __eblis_lookup
#undef __ebxis_insert
#define __ebxis_insert __eblis_insert

/* remap ebxistree.c symbols and types to ebl- equivalent */
#undef ebxis_lookup
#define ebxis_lookup eblis_lookup
#undef ebxis_lookup_len
#define ebxis_lookup_len eblis_lookup_len
#undef ebxis_insert
#define ebxis_insert eblis_insert

/* now include all the generic files ; their symbols
 * will be defined with our names.
 */
#include "ebxtree.h"
#include "ebx32tree.h"
#include "ebx64tree.h"
#include "ebxpttree.h"
#include "ebxmbtree.h"
#include "ebxsttree.h"
#include "ebximtree.h"
#include "ebxistree.h"

#endif
