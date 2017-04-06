/* Mapping of generic ebtree code to absolute pointer code ("eba").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBATREE_H
#define _EBATREE_H

#include "ebcommon.h"

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

/* Assigns a pointer to a link */
#define __eba_setlink(dest, troot) do { *(dest) = (troot); } while (0)

/* Returns the pointer from a link */
#define __eba_getroot(a) (*(a))

/* an absolute pointer is NULL only when exactly NULL (no tag) */
#define __eba_link_is_null(a) (a == NULL)

/* an absolute pointer designates the ROOT if its right branch is NULL. */
#define __eba_is_root(a) ((void *)((a)->b[EB_SIDE_RGHT]) <= (void *)1)

/* we're using absolute pointers for the links */
typedef void *eba_link_t;

/* remap ebxtree.h symbols and types to eba- equivalent */
#undef ebx_root
#define ebx_root eba_root
#undef ebx_troot_t
#define ebx_troot_t eba_troot_t
#undef ebx_node
#define ebx_node eba_node
#undef ebx_link_t
#define ebx_link_t eba_link_t
#undef ebx_is_empty
#define ebx_is_empty eba_is_empty
#undef  __ebx_is_dup
#define __ebx_is_dup __eba_is_dup
#undef  __ebx_untag
#define __ebx_untag __eba_untag
#undef  __ebx_dotag
#define __ebx_dotag __eba_dotag
#undef  __ebx_gettag
#define __ebx_gettag __eba_gettag
#undef  __ebx_root_to_node
#define __ebx_root_to_node __eba_root_to_node
#undef  __ebx_setlink
#define __ebx_setlink __eba_setlink
#undef  __ebx_getroot
#define __ebx_getroot __eba_getroot
#undef  __ebx_link_is_null
#define __ebx_link_is_null __eba_link_is_null
#undef  __ebx_is_root
#define __ebx_is_root __eba_is_root
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
#undef  __ebx_walk_down
#define __ebx_walk_down __eba_walk_down
#undef __ebx_delete
#define __ebx_delete __eba_delete
#undef __ebx_insert_dup
#define __ebx_insert_dup __eba_insert_dup

/* remap ebxtree.c symbols and types to eba- equivalent */
#undef ebx_delete
#define ebx_delete eba_delete
#undef ebx_insert_dup
#define ebx_insert_dup eba_insert_dup

/* remap ebx32tree.h symbols and types to eba- equivalent */
#undef ebx32_node
#define ebx32_node eba32_node
#undef ebx32_first
#define ebx32_first eba32_first
#undef ebx32_last
#define ebx32_last eba32_last
#undef ebx32_next
#define ebx32_next eba32_next
#undef ebx32_prev
#define ebx32_prev eba32_prev
#undef ebx32_next_dup
#define ebx32_next_dup eba32_next_dup
#undef ebx32_prev_dup
#define ebx32_prev_dup eba32_prev_dup
#undef ebx32_next_unique
#define ebx32_next_unique eba32_next_unique
#undef ebx32_prev_unique
#define ebx32_prev_unique eba32_prev_unique
#undef __ebx32_delete
#define __ebx32_delete __eba32_delete
#undef __ebx32_lookup
#define __ebx32_lookup __eba32_lookup
#undef __ebx32i_lookup
#define __ebx32i_lookup __eba32i_lookup
#undef __ebx32_insert
#define __ebx32_insert __eba32_insert
#undef __ebx32i_insert
#define __ebx32i_insert __eba32i_insert

/* remap ebx32tree.c symbols and types to eba- equivalent */
#undef ebx32_delete
#define ebx32_delete eba32_delete
#undef ebx32_lookup
#define ebx32_lookup eba32_lookup
#undef ebx32i_lookup
#define ebx32i_lookup eba32i_lookup
#undef ebx32_insert
#define ebx32_insert eba32_insert
#undef ebx32i_insert
#define ebx32i_insert eba32i_insert
#undef ebx32_lookup_le
#define ebx32_lookup_le eba32_lookup_le
#undef ebx32_lookup_ge
#define ebx32_lookup_ge eba32_lookup_ge

/* remap ebx64tree.h symbols and types to eba- equivalent */
#undef ebx64_node
#define ebx64_node eba64_node
#undef ebx64_first
#define ebx64_first eba64_first
#undef ebx64_last
#define ebx64_last eba64_last
#undef ebx64_next
#define ebx64_next eba64_next
#undef ebx64_prev
#define ebx64_prev eba64_prev
#undef ebx64_next_dup
#define ebx64_next_dup eba64_next_dup
#undef ebx64_prev_dup
#define ebx64_prev_dup eba64_prev_dup
#undef ebx64_next_unique
#define ebx64_next_unique eba64_next_unique
#undef ebx64_prev_unique
#define ebx64_prev_unique eba64_prev_unique
#undef __ebx64_delete
#define __ebx64_delete __eba64_delete
#undef __ebx64_lookup
#define __ebx64_lookup __eba64_lookup
#undef __ebx64i_lookup
#define __ebx64i_lookup __eba64i_lookup
#undef __ebx64_insert
#define __ebx64_insert __eba64_insert
#undef __ebx64i_insert
#define __ebx64i_insert __eba64i_insert

/* remap ebx64tree.c symbols and types to eba- equivalent */
#undef ebx64_delete
#define ebx64_delete eba64_delete
#undef ebx64_lookup
#define ebx64_lookup eba64_lookup
#undef ebx64i_lookup
#define ebx64i_lookup eba64i_lookup
#undef ebx64_insert
#define ebx64_insert eba64_insert
#undef ebx64i_insert
#define ebx64i_insert eba64i_insert
#undef ebx64_lookup_le
#define ebx64_lookup_le eba64_lookup_le
#undef ebx64_lookup_ge
#define ebx64_lookup_ge eba64_lookup_ge

/* remap ebxpttree.h symbols and types to eba- equivalent */
#undef ebxpt_node
#define ebxpt_node ebapt_node
#undef ebxpt_first
#define ebxpt_first ebapt_first
#undef ebxpt_last
#define ebxpt_last ebapt_last
#undef ebxpt_next
#define ebxpt_next ebapt_next
#undef ebxpt_prev
#define ebxpt_prev ebapt_prev
#undef ebxpt_next_dup
#define ebxpt_next_dup ebapt_next_dup
#undef ebxpt_prev_dup
#define ebxpt_prev_dup ebapt_prev_dup
#undef ebxpt_next_unique
#define ebxpt_next_unique ebapt_next_unique
#undef ebxpt_prev_unique
#define ebxpt_prev_unique ebapt_prev_unique
#undef __ebxpt_delete
#define __ebxpt_delete __ebapt_delete
#undef __ebxpt_lookup
#define __ebxpt_lookup __ebapt_lookup
#undef __ebxpt_insert
#define __ebxpt_insert __ebapt_insert
#undef ebxpt_delete
#define ebxpt_delete ebapt_delete
#undef ebxpt_lookup
#define ebxpt_lookup ebapt_lookup
#undef ebxpt_insert
#define ebxpt_insert ebapt_insert
#undef ebxpt_lookup_le
#define ebxpt_lookup_le ebapt_lookup_le
#undef ebxpt_lookup_ge
#define ebxpt_lookup_ge ebapt_lookup_ge

/* remap ebxmbtree.h symbols and types to eba- equivalent */
#undef ebxmb_node
#define ebxmb_node ebamb_node
#undef ebxmb_first
#define ebxmb_first ebamb_first
#undef ebxmb_last
#define ebxmb_last ebamb_last
#undef ebxmb_next
#define ebxmb_next ebamb_next
#undef ebxmb_prev
#define ebxmb_prev ebamb_prev
#undef ebxmb_next_dup
#define ebxmb_next_dup ebamb_next_dup
#undef ebxmb_prev_dup
#define ebxmb_prev_dup ebamb_prev_dup
#undef ebxmb_next_unique
#define ebxmb_next_unique ebamb_next_unique
#undef ebxmb_prev_unique
#define ebxmb_prev_unique ebamb_prev_unique
#undef __ebxmb_delete
#define __ebxmb_delete __ebamb_delete
#undef __ebxmb_lookup_longest
#define __ebxmb_lookup_longest __ebamb_lookup_longest
#undef __ebxmb_lookup
#define __ebxmb_lookup __ebamb_lookup
#undef __ebxmb_insert
#define __ebxmb_insert __ebamb_insert
#undef __ebxmb_lookup_prefix
#define __ebxmb_lookup_prefix __ebamb_lookup_prefix
#undef __ebxmb_insert_prefix
#define __ebxmb_insert_prefix __ebamb_insert_prefix

/* remap ebxmbtree.c symbols and types to eba- equivalent */
#undef ebxmb_delete
#define ebxmb_delete ebamb_delete
#undef ebxmb_lookup_longest
#define ebxmb_lookup_longest ebamb_lookup_longest
#undef ebxmb_lookup
#define ebxmb_lookup ebamb_lookup
#undef ebxmb_insert
#define ebxmb_insert ebamb_insert
#undef ebxmb_lookup_prefix
#define ebxmb_lookup_prefix ebamb_lookup_prefix
#undef ebxmb_insert_prefix
#define ebxmb_insert_prefix ebamb_insert_prefix

/* remap ebxsttree.h symbols and types to eba- equivalent */
#undef __ebxst_lookup
#define __ebxst_lookup __ebast_lookup
#undef __ebxst_insert
#define __ebxst_insert __ebast_insert

/* remap ebxsttree.c symbols and types to eba- equivalent */
#undef ebxst_lookup
#define ebxst_lookup ebast_lookup
#undef ebxst_lookup_len
#define ebxst_lookup_len ebast_lookup_len
#undef ebxst_insert
#define ebxst_insert ebast_insert

/* remap ebximtree.h symbols and types to eba- equivalent */
#undef __ebxim_lookup
#define __ebxim_lookup __ebaim_lookup
#undef __ebxim_insert
#define __ebxim_insert __ebaim_insert

/* remap ebximtree.c symbols and types to eba- equivalent */
#undef ebxim_lookup
#define ebxim_lookup ebaim_lookup
#undef ebxim_insert
#define ebxim_insert ebaim_insert

/* remap ebxistree.h symbols and types to eba- equivalent */
#undef __ebxis_lookup
#define __ebxis_lookup __ebais_lookup
#undef __ebxis_insert
#define __ebxis_insert __ebais_insert

/* remap ebxistree.c symbols and types to eba- equivalent */
#undef ebxis_lookup
#define ebxis_lookup ebais_lookup
#undef ebxis_lookup_len
#define ebxis_lookup_len ebais_lookup_len
#undef ebxis_insert
#define ebxis_insert ebais_insert

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
