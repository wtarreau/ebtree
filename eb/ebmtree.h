/* Mapping of generic ebtree code to relative medium pointer code ("ebm").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBMTREE_H
#define _EBMTREE_H

#include "ebcommon.h"

/* EB_SIZE = 32 : we're using 32-bit offsets for relative pointers. */
#undef EB_SIZE
#define EB_SIZE 32

/* The root of a tree is an ebm_root initialized with both pointers NULL.
 * During its life, only the left pointer will change. The right one will
 * always remain NULL, which is the way we detect it.
 */
#define EBM_ROOT						\
	(struct ebm_root) {				\
		.b = {[0] = 0, [1] = 0 },		\
	}

#define EBM_ROOT_UNIQUE					\
	(struct ebm_root) {				\
		.b = {[0] = 0, [1] = (ebm_link_t)1 },	\
	}

#define EBM_TREE_HEAD(name)				\
	struct ebm_root name = EBM_ROOT

/* map a few macros to their original names */
#define ebm_entry(ptr, type, member) ebx_entry(ptr, type, member)
#define ebm32_entry(ptr, type, member) ebx32_entry(ptr, type, member)
#define ebm64_entry(ptr, type, member) ebx64_entry(ptr, type, member)
#define ebmpt_entry(ptr, type, member) ebxpt_entry(ptr, type, member)
#define ebmmb_entry(ptr, type, member) ebxmb_entry(ptr, type, member)

/* we're using 32-bit signed offsets for the links */
typedef s32 ebm_link_t;

/* remap ebxtree.h symbols and types to ebm- equivalent */
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

/* remap ebxtree.c symbols and types to ebm- equivalent */
#undef ebx_delete
#define ebx_delete ebm_delete
#undef ebx_insert_dup
#define ebx_insert_dup ebm_insert_dup

/* remap ebx32tree.h symbols and types to ebm- equivalent */
#undef ebx32_node
#define ebx32_node ebm32_node
#undef ebx32_first
#define ebx32_first ebm32_first
#undef ebx32_last
#define ebx32_last ebm32_last
#undef ebx32_next
#define ebx32_next ebm32_next
#undef ebx32_prev
#define ebx32_prev ebm32_prev
#undef ebx32_next_dup
#define ebx32_next_dup ebm32_next_dup
#undef ebx32_prev_dup
#define ebx32_prev_dup ebm32_prev_dup
#undef ebx32_next_unique
#define ebx32_next_unique ebm32_next_unique
#undef ebx32_prev_unique
#define ebx32_prev_unique ebm32_prev_unique
#undef __ebx32_delete
#define __ebx32_delete __ebm32_delete
#undef __ebx32_lookup
#define __ebx32_lookup __ebm32_lookup
#undef __ebx32i_lookup
#define __ebx32i_lookup __ebm32i_lookup
#undef __ebx32_insert
#define __ebx32_insert __ebm32_insert
#undef __ebx32i_insert
#define __ebx32i_insert __ebm32i_insert

/* remap ebx32tree.c symbols and types to ebm- equivalent */
#undef ebx32_delete
#define ebx32_delete ebm32_delete
#undef ebx32_lookup
#define ebx32_lookup ebm32_lookup
#undef ebx32i_lookup
#define ebx32i_lookup ebm32i_lookup
#undef ebx32_insert
#define ebx32_insert ebm32_insert
#undef ebx32i_insert
#define ebx32i_insert ebm32i_insert
#undef ebx32_lookup_le
#define ebx32_lookup_le ebm32_lookup_le
#undef ebx32_lookup_ge
#define ebx32_lookup_ge ebm32_lookup_ge

/* remap ebx64tree.h symbols and types to ebm- equivalent */
#undef ebx64_node
#define ebx64_node ebm64_node
#undef ebx64_first
#define ebx64_first ebm64_first
#undef ebx64_last
#define ebx64_last ebm64_last
#undef ebx64_next
#define ebx64_next ebm64_next
#undef ebx64_prev
#define ebx64_prev ebm64_prev
#undef ebx64_next_dup
#define ebx64_next_dup ebm64_next_dup
#undef ebx64_prev_dup
#define ebx64_prev_dup ebm64_prev_dup
#undef ebx64_next_unique
#define ebx64_next_unique ebm64_next_unique
#undef ebx64_prev_unique
#define ebx64_prev_unique ebm64_prev_unique
#undef __ebx64_delete
#define __ebx64_delete __ebm64_delete
#undef __ebx64_lookup
#define __ebx64_lookup __ebm64_lookup
#undef __ebx64i_lookup
#define __ebx64i_lookup __ebm64i_lookup
#undef __ebx64_insert
#define __ebx64_insert __ebm64_insert
#undef __ebx64i_insert
#define __ebx64i_insert __ebm64i_insert

/* remap ebx64tree.c symbols and types to ebm- equivalent */
#undef ebx64_delete
#define ebx64_delete ebm64_delete
#undef ebx64_lookup
#define ebx64_lookup ebm64_lookup
#undef ebx64i_lookup
#define ebx64i_lookup ebm64i_lookup
#undef ebx64_insert
#define ebx64_insert ebm64_insert
#undef ebx64i_insert
#define ebx64i_insert ebm64i_insert
#undef ebx64_lookup_le
#define ebx64_lookup_le ebm64_lookup_le
#undef ebx64_lookup_ge
#define ebx64_lookup_ge ebm64_lookup_ge

/* remap ebxpttree.h symbols and types to ebm- equivalent */
#undef ebxpt_node
#define ebxpt_node ebmpt_node
#undef ebxpt_first
#define ebxpt_first ebmpt_first
#undef ebxpt_last
#define ebxpt_last ebmpt_last
#undef ebxpt_next
#define ebxpt_next ebmpt_next
#undef ebxpt_prev
#define ebxpt_prev ebmpt_prev
#undef ebxpt_next_dup
#define ebxpt_next_dup ebmpt_next_dup
#undef ebxpt_prev_dup
#define ebxpt_prev_dup ebmpt_prev_dup
#undef ebxpt_next_unique
#define ebxpt_next_unique ebmpt_next_unique
#undef ebxpt_prev_unique
#define ebxpt_prev_unique ebmpt_prev_unique
#undef __ebxpt_delete
#define __ebxpt_delete __ebmpt_delete
#undef __ebxpt_lookup
#define __ebxpt_lookup __ebmpt_lookup
#undef __ebxpt_insert
#define __ebxpt_insert __ebmpt_insert
#undef ebxpt_delete
#define ebxpt_delete ebmpt_delete
#undef ebxpt_lookup
#define ebxpt_lookup ebmpt_lookup
#undef ebxpt_insert
#define ebxpt_insert ebmpt_insert
#undef ebxpt_lookup_le
#define ebxpt_lookup_le ebmpt_lookup_le
#undef ebxpt_lookup_ge
#define ebxpt_lookup_ge ebmpt_lookup_ge

/* remap ebxmbtree.h symbols and types to ebm- equivalent */
#undef ebxmb_node
#define ebxmb_node ebmmb_node
#undef ebxmb_first
#define ebxmb_first ebmmb_first
#undef ebxmb_last
#define ebxmb_last ebmmb_last
#undef ebxmb_next
#define ebxmb_next ebmmb_next
#undef ebxmb_prev
#define ebxmb_prev ebmmb_prev
#undef ebxmb_next_dup
#define ebxmb_next_dup ebmmb_next_dup
#undef ebxmb_prev_dup
#define ebxmb_prev_dup ebmmb_prev_dup
#undef ebxmb_next_unique
#define ebxmb_next_unique ebmmb_next_unique
#undef ebxmb_prev_unique
#define ebxmb_prev_unique ebmmb_prev_unique
#undef __ebxmb_delete
#define __ebxmb_delete __ebmmb_delete
#undef __ebxmb_lookup_longest
#define __ebxmb_lookup_longest __ebmmb_lookup_longest
#undef __ebxmb_lookup
#define __ebxmb_lookup __ebmmb_lookup
#undef __ebxmb_insert
#define __ebxmb_insert __ebmmb_insert
#undef __ebxmb_lookup_prefix
#define __ebxmb_lookup_prefix __ebmmb_lookup_prefix
#undef __ebxmb_insert_prefix
#define __ebxmb_insert_prefix __ebmmb_insert_prefix

/* remap ebxmbtree.c symbols and types to ebm- equivalent */
#undef ebxmb_delete
#define ebxmb_delete ebmmb_delete
#undef ebxmb_lookup_longest
#define ebxmb_lookup_longest ebmmb_lookup_longest
#undef ebxmb_lookup
#define ebxmb_lookup ebmmb_lookup
#undef ebxmb_insert
#define ebxmb_insert ebmmb_insert
#undef ebxmb_lookup_prefix
#define ebxmb_lookup_prefix ebmmb_lookup_prefix
#undef ebxmb_insert_prefix
#define ebxmb_insert_prefix ebmmb_insert_prefix

/* remap ebxsttree.h symbols and types to ebm- equivalent */
#undef __ebxst_lookup
#define __ebxst_lookup __ebmst_lookup
#undef __ebxst_insert
#define __ebxst_insert __ebmst_insert

/* remap ebxsttree.c symbols and types to ebm- equivalent */
#undef ebxst_lookup
#define ebxst_lookup ebmst_lookup
#undef ebxst_lookup_len
#define ebxst_lookup_len ebmst_lookup_len
#undef ebxst_insert
#define ebxst_insert ebmst_insert

/* remap ebximtree.h symbols and types to ebm- equivalent */
#undef __ebxim_lookup
#define __ebxim_lookup __ebmim_lookup
#undef __ebxim_insert
#define __ebxim_insert __ebmim_insert

/* remap ebximtree.c symbols and types to ebm- equivalent */
#undef ebxim_lookup
#define ebxim_lookup ebmim_lookup
#undef ebxim_insert
#define ebxim_insert ebmim_insert

/* remap ebxistree.h symbols and types to ebm- equivalent */
#undef __ebxis_lookup
#define __ebxis_lookup __ebmis_lookup
#undef __ebxis_insert
#define __ebxis_insert __ebmis_insert

/* remap ebxistree.c symbols and types to ebm- equivalent */
#undef ebxis_lookup
#define ebxis_lookup ebmis_lookup
#undef ebxis_lookup_len
#define ebxis_lookup_len ebmis_lookup_len
#undef ebxis_insert
#define ebxis_insert ebmis_insert

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
