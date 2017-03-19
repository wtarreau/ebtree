/* Mapping of generic ebtree code to reentrant absolute pointer code ("ebar").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBARTREE_H
#define _EBARTREE_H

#include "ebcommon.h"

/* The root of a tree is an ebar_root initialized with left pointer NULL
 * and the right one having bit 1 set. During its life, only the left
 * pointer will change, always keeping bit 1 cleared. The right one will
 * always remain set, which is the way we detect it.
 */
#define EBAR_ROOT					\
	(struct ebar_root) {				\
		.b = {[0] = 0, [1] = (ebar_link_t)2 },	\
	}

#define EBAR_ROOT_UNIQUE					\
	(struct ebar_root) {				\
		.b = {[0] = 0, [1] = (ebar_link_t)3 },	\
	}

#define EBAR_TREE_HEAD(name)				\
	struct ebar_root name = EBAR_ROOT

/* Assigns a pointer to a link */
#define __ebar_setlink(dest, troot) do { *(dest) = (troot); } while (0)

/* Returns the pointer from a link */
#define __ebar_getroot(a) (*(a))

/* an absolute pointer is NULL only when exactly NULL (no tag) */
#define __ebar_link_is_null(a) (a == NULL)

/* an absolute pointer designates the ROOT if its right branch is NULL. */
#define __ebar_is_root(a) ((size_t)((a)->b[EB_RGHT]) & 2)

/* we're using absolute pointers for the links */
typedef void *ebar_link_t;

/* remap ebxtree.h symbols and types to ebar- equivalent */
#undef ebx_root
#define ebx_root ebar_root
#undef ebx_troot_t
#define ebx_troot_t ebar_troot_t
#undef ebx_node
#define ebx_node ebar_node
#undef ebx_link_t
#define ebx_link_t ebar_link_t
#undef ebx_is_empty
#define ebx_is_empty ebar_is_empty
#undef  __ebx_is_dup
#define __ebx_is_dup __ebar_is_dup
#undef  __ebx_untag
#define __ebx_untag __ebar_untag
#undef  __ebx_dotag
#define __ebx_dotag __ebar_dotag
#undef  __ebx_clrtag
#define __ebx_clrtag __ebar_clrtag
#undef  __ebx_gettag
#define __ebx_gettag __ebar_gettag
#undef  __ebx_root_to_node
#define __ebx_root_to_node __ebar_root_to_node
#undef  __ebx_setlink
#define __ebx_setlink __ebar_setlink
#undef  __ebx_getroot
#define __ebx_getroot __ebar_getroot
#undef  __ebx_link_is_null
#define __ebx_link_is_null __ebar_link_is_null
#undef  __ebx_is_root
#define __ebx_is_root __ebar_is_root
#undef ebx_first
#define ebx_first ebar_first
#undef ebx_last
#define ebx_last ebar_last
#undef ebx_next
#define ebx_next ebar_next
#undef ebx_next_dup
#define ebx_next_dup ebar_next_dup
#undef ebx_next_unique
#define ebx_next_unique ebar_next_unique
#undef ebx_prev
#define ebx_prev ebar_prev
#undef ebx_prev_dup
#define ebx_prev_dup ebar_prev_dup
#undef ebx_prev_unique
#define ebx_prev_unique ebar_prev_unique
#undef  __ebx_walk_down
#define __ebx_walk_down __ebar_walk_down
#undef __ebx_delete
#define __ebx_delete __ebar_delete
#undef __ebx_insert_dup
#define __ebx_insert_dup __ebar_insert_dup

/* remap ebxtree.c symbols and types to ebar- equivalent */
#undef ebx_delete
#define ebx_delete ebar_delete
#undef ebx_insert_dup
#define ebx_insert_dup ebar_insert_dup

/* remap ebx32tree.h symbols and types to ebar- equivalent */
#undef ebx32_node
#define ebx32_node ebar32_node
#undef ebx32_first
#define ebx32_first ebar32_first
#undef ebx32_last
#define ebx32_last ebar32_last
#undef ebx32_next
#define ebx32_next ebar32_next
#undef ebx32_prev
#define ebx32_prev ebar32_prev
#undef ebx32_next_dup
#define ebx32_next_dup ebar32_next_dup
#undef ebx32_prev_dup
#define ebx32_prev_dup ebar32_prev_dup
#undef ebx32_next_unique
#define ebx32_next_unique ebar32_next_unique
#undef ebx32_prev_unique
#define ebx32_prev_unique ebar32_prev_unique
#undef __ebx32_delete
#define __ebx32_delete __ebar32_delete
#undef __ebx32_lookup
#define __ebx32_lookup __ebar32_lookup
#undef __ebx32i_lookup
#define __ebx32i_lookup __ebar32i_lookup
#undef __ebx32_insert
#define __ebx32_insert __ebar32_insert
#undef __ebx32i_insert
#define __ebx32i_insert __ebar32i_insert

/* remap ebx32tree.c symbols and types to ebar- equivalent */
#undef ebx32_delete
#define ebx32_delete ebar32_delete
#undef ebx32_lookup
#define ebx32_lookup ebar32_lookup
#undef ebx32i_lookup
#define ebx32i_lookup ebar32i_lookup
#undef ebx32_insert
#define ebx32_insert ebar32_insert
#undef ebx32i_insert
#define ebx32i_insert ebar32i_insert
#undef ebx32_lookup_le
#define ebx32_lookup_le ebar32_lookup_le
#undef ebx32_lookup_ge
#define ebx32_lookup_ge ebar32_lookup_ge

/* remap ebx64tree.h symbols and types to ebar- equivalent */
#undef ebx64_node
#define ebx64_node ebar64_node
#undef ebx64_first
#define ebx64_first ebar64_first
#undef ebx64_last
#define ebx64_last ebar64_last
#undef ebx64_next
#define ebx64_next ebar64_next
#undef ebx64_prev
#define ebx64_prev ebar64_prev
#undef ebx64_next_dup
#define ebx64_next_dup ebar64_next_dup
#undef ebx64_prev_dup
#define ebx64_prev_dup ebar64_prev_dup
#undef ebx64_next_unique
#define ebx64_next_unique ebar64_next_unique
#undef ebx64_prev_unique
#define ebx64_prev_unique ebar64_prev_unique
#undef __ebx64_delete
#define __ebx64_delete __ebar64_delete
#undef __ebx64_lookup
#define __ebx64_lookup __ebar64_lookup
#undef __ebx64i_lookup
#define __ebx64i_lookup __ebar64i_lookup
#undef __ebx64_insert
#define __ebx64_insert __ebar64_insert
#undef __ebx64i_insert
#define __ebx64i_insert __ebar64i_insert

/* remap ebx64tree.c symbols and types to ebar- equivalent */
#undef ebx64_delete
#define ebx64_delete ebar64_delete
#undef ebx64_lookup
#define ebx64_lookup ebar64_lookup
#undef ebx64i_lookup
#define ebx64i_lookup ebar64i_lookup
#undef ebx64_insert
#define ebx64_insert ebar64_insert
#undef ebx64i_insert
#define ebx64i_insert ebar64i_insert
#undef ebx64_lookup_le
#define ebx64_lookup_le ebar64_lookup_le
#undef ebx64_lookup_ge
#define ebx64_lookup_ge ebar64_lookup_ge

/* remap ebxpttree.h symbols and types to ebar- equivalent */
#undef ebxpt_node
#define ebxpt_node ebarpt_node
#undef ebxpt_first
#define ebxpt_first ebarpt_first
#undef ebxpt_last
#define ebxpt_last ebarpt_last
#undef ebxpt_next
#define ebxpt_next ebarpt_next
#undef ebxpt_prev
#define ebxpt_prev ebarpt_prev
#undef ebxpt_next_dup
#define ebxpt_next_dup ebarpt_next_dup
#undef ebxpt_prev_dup
#define ebxpt_prev_dup ebarpt_prev_dup
#undef ebxpt_next_unique
#define ebxpt_next_unique ebarpt_next_unique
#undef ebxpt_prev_unique
#define ebxpt_prev_unique ebarpt_prev_unique
#undef __ebxpt_delete
#define __ebxpt_delete __ebarpt_delete
#undef __ebxpt_lookup
#define __ebxpt_lookup __ebarpt_lookup
#undef __ebxpt_insert
#define __ebxpt_insert __ebarpt_insert
#undef ebxpt_delete
#define ebxpt_delete ebarpt_delete
#undef ebxpt_lookup
#define ebxpt_lookup ebarpt_lookup
#undef ebxpt_insert
#define ebxpt_insert ebarpt_insert
#undef ebxpt_lookup_le
#define ebxpt_lookup_le ebarpt_lookup_le
#undef ebxpt_lookup_ge
#define ebxpt_lookup_ge ebarpt_lookup_ge

/* remap ebxmbtree.h symbols and types to ebar- equivalent */
#undef ebxmb_node
#define ebxmb_node ebarmb_node
#undef ebxmb_first
#define ebxmb_first ebarmb_first
#undef ebxmb_last
#define ebxmb_last ebarmb_last
#undef ebxmb_next
#define ebxmb_next ebarmb_next
#undef ebxmb_prev
#define ebxmb_prev ebarmb_prev
#undef ebxmb_next_dup
#define ebxmb_next_dup ebarmb_next_dup
#undef ebxmb_prev_dup
#define ebxmb_prev_dup ebarmb_prev_dup
#undef ebxmb_next_unique
#define ebxmb_next_unique ebarmb_next_unique
#undef ebxmb_prev_unique
#define ebxmb_prev_unique ebarmb_prev_unique
#undef __ebxmb_delete
#define __ebxmb_delete __ebarmb_delete
#undef __ebxmb_lookup_longest
#define __ebxmb_lookup_longest __ebarmb_lookup_longest
#undef __ebxmb_lookup
#define __ebxmb_lookup __ebarmb_lookup
#undef __ebxmb_insert
#define __ebxmb_insert __ebarmb_insert
#undef __ebxmb_lookup_prefix
#define __ebxmb_lookup_prefix __ebarmb_lookup_prefix
#undef __ebxmb_insert_prefix
#define __ebxmb_insert_prefix __ebarmb_insert_prefix

/* remap ebxmbtree.c symbols and types to ebar- equivalent */
#undef ebxmb_delete
#define ebxmb_delete ebarmb_delete
#undef ebxmb_lookup_longest
#define ebxmb_lookup_longest ebarmb_lookup_longest
#undef ebxmb_lookup
#define ebxmb_lookup ebarmb_lookup
#undef ebxmb_insert
#define ebxmb_insert ebarmb_insert
#undef ebxmb_lookup_prefix
#define ebxmb_lookup_prefix ebarmb_lookup_prefix
#undef ebxmb_insert_prefix
#define ebxmb_insert_prefix ebarmb_insert_prefix

/* remap ebxsttree.h symbols and types to ebar- equivalent */
#undef __ebxst_lookup
#define __ebxst_lookup __ebarst_lookup
#undef __ebxst_insert
#define __ebxst_insert __ebarst_insert

/* remap ebxsttree.c symbols and types to ebar- equivalent */
#undef ebxst_lookup
#define ebxst_lookup ebarst_lookup
#undef ebxst_lookup_len
#define ebxst_lookup_len ebarst_lookup_len
#undef ebxst_insert
#define ebxst_insert ebarst_insert

/* remap ebximtree.h symbols and types to ebar- equivalent */
#undef __ebxim_lookup
#define __ebxim_lookup __ebarim_lookup
#undef __ebxim_insert
#define __ebxim_insert __ebarim_insert

/* remap ebximtree.c symbols and types to ebar- equivalent */
#undef ebxim_lookup
#define ebxim_lookup ebarim_lookup
#undef ebxim_insert
#define ebxim_insert ebarim_insert

/* remap ebxistree.h symbols and types to ebar- equivalent */
#undef __ebxis_lookup
#define __ebxis_lookup __ebaris_lookup
#undef __ebxis_insert
#define __ebxis_insert __ebaris_insert

/* remap ebxistree.c symbols and types to ebar- equivalent */
#undef ebxis_lookup
#define ebxis_lookup ebaris_lookup
#undef ebxis_lookup_len
#define ebxis_lookup_len ebaris_lookup_len
#undef ebxis_insert
#define ebxis_insert ebaris_insert

/* now include all the generic files ; their symbols
 * will be defined with our names.
 */
#undef EB_TREE_RELATIVE
#include "ebxtree.h"
#include "ebx32tree.h"
#include "ebx64tree.h"
#include "ebxpttree.h"
#include "ebxmbtree.h"
#include "ebxsttree.h"
#include "ebximtree.h"
#include "ebxistree.h"

#endif
