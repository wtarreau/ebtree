/* Mapping of generic ebtree code to relative short pointer code ("ebs").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBSTREE_H
#define _EBSTREE_H

/* EB_SIZE = 16 : we're using 16-bit offsets for relative pointers. */
#undef EB_SIZE
#define EB_SIZE 16

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

/* map a few macros to their original names */
#define ebs_entry(ptr, type, member) ebx_entry(ptr, type, member)
#define ebs32_entry(ptr, type, member) ebx32_entry(ptr, type, member)
#define ebs64_entry(ptr, type, member) ebx64_entry(ptr, type, member)
#define ebspt_entry(ptr, type, member) ebxpt_entry(ptr, type, member)
#define ebsmb_entry(ptr, type, member) ebxmb_entry(ptr, type, member)


/* remap ebxtree.h symbols and types to ebs- equivalent */
#undef ebx_root
#define ebx_root ebs_root
#undef ebx_troot_t
#define ebx_troot_t ebs_troot_t
#undef ebx_node
#define ebx_node ebs_node
#undef ebx_link_t
#define ebx_link_t ebs_link_t
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

/* remap ebxtree.c symbols and types to ebs- equivalent */
#undef ebx_delete
#define ebx_delete ebs_delete
#undef ebx_insert_dup
#define ebx_insert_dup ebs_insert_dup

/* remap ebx32tree.h symbols and types to ebs- equivalent */
#undef ebx32_node
#define ebx32_node ebs32_node
#undef ebx32_first
#define ebx32_first ebs32_first
#undef ebx32_last
#define ebx32_last ebs32_last
#undef ebx32_next
#define ebx32_next ebs32_next
#undef ebx32_prev
#define ebx32_prev ebs32_prev
#undef ebx32_next_dup
#define ebx32_next_dup ebs32_next_dup
#undef ebx32_prev_dup
#define ebx32_prev_dup ebs32_prev_dup
#undef ebx32_next_unique
#define ebx32_next_unique ebs32_next_unique
#undef ebx32_prev_unique
#define ebx32_prev_unique ebs32_prev_unique
#undef __ebx32_delete
#define __ebx32_delete __ebs32_delete
#undef __ebx32_lookup
#define __ebx32_lookup __ebs32_lookup
#undef __ebx32i_lookup
#define __ebx32i_lookup __ebs32i_lookup
#undef __ebx32_insert
#define __ebx32_insert __ebs32_insert
#undef __ebx32i_insert
#define __ebx32i_insert __ebs32i_insert

/* remap ebx32tree.c symbols and types to ebs- equivalent */
#undef ebx32_delete
#define ebx32_delete ebs32_delete
#undef ebx32_lookup
#define ebx32_lookup ebs32_lookup
#undef ebx32i_lookup
#define ebx32i_lookup ebs32i_lookup
#undef ebx32_insert
#define ebx32_insert ebs32_insert
#undef ebx32i_insert
#define ebx32i_insert ebs32i_insert
#undef ebx32_lookup_le
#define ebx32_lookup_le ebs32_lookup_le
#undef ebx32_lookup_ge
#define ebx32_lookup_ge ebs32_lookup_ge

/* remap ebx64tree.h symbols and types to ebs- equivalent */
#undef ebx64_node
#define ebx64_node ebs64_node
#undef ebx64_first
#define ebx64_first ebs64_first
#undef ebx64_last
#define ebx64_last ebs64_last
#undef ebx64_next
#define ebx64_next ebs64_next
#undef ebx64_prev
#define ebx64_prev ebs64_prev
#undef ebx64_next_dup
#define ebx64_next_dup ebs64_next_dup
#undef ebx64_prev_dup
#define ebx64_prev_dup ebs64_prev_dup
#undef ebx64_next_unique
#define ebx64_next_unique ebs64_next_unique
#undef ebx64_prev_unique
#define ebx64_prev_unique ebs64_prev_unique
#undef __ebx64_delete
#define __ebx64_delete __ebs64_delete
#undef __ebx64_lookup
#define __ebx64_lookup __ebs64_lookup
#undef __ebx64i_lookup
#define __ebx64i_lookup __ebs64i_lookup
#undef __ebx64_insert
#define __ebx64_insert __ebs64_insert
#undef __ebx64i_insert
#define __ebx64i_insert __ebs64i_insert

/* remap ebx64tree.c symbols and types to ebs- equivalent */
#undef ebx64_delete
#define ebx64_delete ebs64_delete
#undef ebx64_lookup
#define ebx64_lookup ebs64_lookup
#undef ebx64i_lookup
#define ebx64i_lookup ebs64i_lookup
#undef ebx64_insert
#define ebx64_insert ebs64_insert
#undef ebx64i_insert
#define ebx64i_insert ebs64i_insert
#undef ebx64_lookup_le
#define ebx64_lookup_le ebs64_lookup_le
#undef ebx64_lookup_ge
#define ebx64_lookup_ge ebs64_lookup_ge

/* remap ebxpttree.h symbols and types to ebs- equivalent */
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

/* remap ebxmbtree.h symbols and types to ebs- equivalent */
#undef ebxmb_node
#define ebxmb_node ebsmb_node
#undef ebxmb_first
#define ebxmb_first ebsmb_first
#undef ebxmb_last
#define ebxmb_last ebsmb_last
#undef ebxmb_next
#define ebxmb_next ebsmb_next
#undef ebxmb_prev
#define ebxmb_prev ebsmb_prev
#undef ebxmb_next_dup
#define ebxmb_next_dup ebsmb_next_dup
#undef ebxmb_prev_dup
#define ebxmb_prev_dup ebsmb_prev_dup
#undef ebxmb_next_unique
#define ebxmb_next_unique ebsmb_next_unique
#undef ebxmb_prev_unique
#define ebxmb_prev_unique ebsmb_prev_unique
#undef __ebxmb_delete
#define __ebxmb_delete __ebsmb_delete
#undef __ebxmb_lookup_longest
#define __ebxmb_lookup_longest __ebsmb_lookup_longest
#undef __ebxmb_lookup
#define __ebxmb_lookup __ebsmb_lookup
#undef __ebxmb_insert
#define __ebxmb_insert __ebsmb_insert
#undef __ebxmb_lookup_prefix
#define __ebxmb_lookup_prefix __ebsmb_lookup_prefix
#undef __ebxmb_insert_prefix
#define __ebxmb_insert_prefix __ebsmb_insert_prefix

/* remap ebxmbtree.c symbols and types to ebs- equivalent */
#undef ebxmb_delete
#define ebxmb_delete ebsmb_delete
#undef ebxmb_lookup_longest
#define ebxmb_lookup_longest ebsmb_lookup_longest
#undef ebxmb_lookup
#define ebxmb_lookup ebsmb_lookup
#undef ebxmb_insert
#define ebxmb_insert ebsmb_insert
#undef ebxmb_lookup_prefix
#define ebxmb_lookup_prefix ebsmb_lookup_prefix
#undef ebxmb_insert_prefix
#define ebxmb_insert_prefix ebsmb_insert_prefix

/* remap ebxsttree.h symbols and types to ebs- equivalent */
#undef __ebxst_lookup
#define __ebxst_lookup __ebsst_lookup
#undef __ebxst_insert
#define __ebxst_insert __ebsst_insert

/* remap ebxsttree.c symbols and types to ebs- equivalent */
#undef ebxst_lookup
#define ebxst_lookup ebsst_lookup
#undef ebxst_insert
#define ebxst_insert ebsst_insert

/* remap ebximtree.h symbols and types to ebs- equivalent */
#undef __ebxim_lookup
#define __ebxim_lookup __ebsim_lookup
#undef __ebxim_insert
#define __ebxim_insert __ebsim_insert

/* remap ebximtree.c symbols and types to ebs- equivalent */
#undef ebxim_lookup
#define ebxim_lookup ebsim_lookup
#undef ebxim_insert
#define ebxim_insert ebsim_insert

/* remap ebxistree.h symbols and types to ebs- equivalent */
#undef __ebxis_lookup
#define __ebxis_lookup __ebsis_lookup
#undef __ebxis_insert
#define __ebxis_insert __ebsis_insert

/* remap ebxistree.c symbols and types to ebs- equivalent */
#undef ebxis_lookup
#define ebxis_lookup ebsis_lookup
#undef ebxis_insert
#define ebxis_insert ebsis_insert

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
