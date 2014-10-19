/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * It maps eb* names to eba* so that legacy applications can continue to use
 * eb* names.
 */

#ifndef _EBTREE_H
#define _EBTREE_H

#include "ebatree.h"

#undef eb_root
#define eb_root eba_root
#undef eb_troot_t
#define eb_troot_t eba_troot_t
#undef eb_node
#define eb_node eba_node
#undef eb_link_t
#define eb_link_t eba_link_t

#define EB_ROOT            EBA_ROOT
#define EB_ROOT_UNIQUE     EBA_ROOT_UNIQUE
#define EB_TREE_HEAD(name) EBA_TREE_HEAD(name)

#undef eb_is_empty
#define eb_is_empty eba_is_empty
#undef eb_is_dup
#define eb_is_dup eba_is_dup

#undef eb_untag
#define eb_untag eba_untag
#undef eb_dotag
#define eb_dotag eba_dotag
#undef eb_clrtag
#define eb_clrtag eba_clrtag
#undef eb_gettag
#define eb_gettag eba_gettag

#undef eb_first
#define eb_first eba_first
#undef eb_last
#define eb_last eba_last
#undef eb_next
#define eb_next eba_next
#undef eb_next_dup
#define eb_next_dup eba_next_dup
#undef eb_next_unique
#define eb_next_unique eba_next_unique
#undef eb_prev
#define eb_prev eba_prev
#undef eb_prev_dup
#define eb_prev_dup eba_prev_dup
#undef eb_prev_unique
#define eb_prev_unique eba_prev_unique
#undef eb_walk_down
#define eb_walk_down eba_walk_down

#undef __eb_delete
#define __eb_delete __eba_delete
#undef __eb_insert_dup
#define __eb_insert_dup __eba_insert_dup

#undef eb_delete
#define eb_delete eba_delete
#undef eb_insert_dup
#define eb_insert_dup eba_insert_dup

#define eb_entry(ptr, type, member) eba_entry(ptr, type, member)

#define eb32_node eba32_node

#define eb32_first eba32_first
#define eb32_last eba32_last
#define eb32_next eba32_next
#define eb32_prev eba32_prev
#define eb32_next_dup eba32_next_dup
#define eb32_prev_dup eba32_prev_dup
#define eb32_next_unique eba32_next_unique
#define eb32_prev_unique eba32_prev_unique

#define __eb32_delete __eba32_delete
#define __eb32_lookup __eba32_lookup
#define __eb32i_lookup __eba32i_lookup
#define __eb32_insert __eba32_insert
#define __eb32i_insert __eba32i_insert

#define eb32_delete eba32_delete
#define eb32_lookup eba32_lookup
#define eb32i_lookup eba32i_lookup
#define eb32_insert eba32_insert
#define eb32i_insert eba32i_insert
#define eb32_lookup_le eba32_lookup_le
#define eb32_lookup_ge eba32_lookup_ge

#define eb32_entry(ptr, type, member) eba32_entry(ptr, type, member)

#define eb64_node eba64_node

#define eb64_first eba64_first
#define eb64_last eba64_last
#define eb64_next eba64_next
#define eb64_prev eba64_prev
#define eb64_next_dup eba64_next_dup
#define eb64_prev_dup eba64_prev_dup
#define eb64_next_unique eba64_next_unique
#define eb64_prev_unique eba64_prev_unique

#define __eb64_delete __eba64_delete
#define __eb64_lookup __eba64_lookup
#define __eb64i_lookup __eba64i_lookup
#define __eb64_insert __eba64_insert
#define __eb64i_insert __eba64i_insert

#define eb64_delete eba64_delete
#define eb64_lookup eba64_lookup
#define eb64i_lookup eba64i_lookup
#define eb64_insert eba64_insert
#define eb64i_insert eba64i_insert
#define eb64_lookup_le eba64_lookup_le
#define eb64_lookup_ge eba64_lookup_ge

#define eb64_entry(ptr, type, member) eba64_entry(ptr, type, member)

#define ebpt_node ebapt_node

#define ebpt_first ebapt_first
#define ebpt_last ebapt_last
#define ebpt_next ebapt_next
#define ebpt_prev ebapt_prev
#define ebpt_next_dup ebapt_next_dup
#define ebpt_prev_dup ebapt_prev_dup
#define ebpt_next_unique ebapt_next_unique
#define ebpt_prev_unique ebapt_prev_unique

#define __ebpt_delete __ebapt_delete
#define __ebpt_lookup __ebapt_lookup
#define __ebpt_insert __ebapt_insert

#define ebpt_delete ebapt_delete
#define ebpt_lookup ebapt_lookup
#define ebpt_insert ebapt_insert
#define ebpt_lookup_le ebapt_lookup_le
#define ebpt_lookup_ge ebapt_lookup_ge

#define ebpt_entry(ptr, type, member) ebapt_entry(ptr, type, member)

#define ebmb_node ebamb_node

#define ebmb_first ebamb_first
#define ebmb_last ebamb_last
#define ebmb_next ebamb_next
#define ebmb_prev ebamb_prev
#define ebmb_next_dup ebamb_next_dup
#define ebmb_prev_dup ebamb_prev_dup
#define ebmb_next_unique ebamb_next_unique
#define ebmb_prev_unique ebamb_prev_unique

#define __ebmb_delete __ebamb_delete
#define __ebmb_lookup_longest __ebamb_lookup_longest
#define __ebmb_lookup __ebamb_lookup
#define __ebmb_insert __ebamb_insert
#define __ebmb_lookup_prefix __ebamb_lookup_prefix
#define __ebmb_insert_prefix __ebamb_insert_prefix

#define ebmb_delete ebamb_delete
#define ebmb_lookup_longest ebamb_lookup_longest
#define ebmb_lookup ebamb_lookup
#define ebmb_insert ebamb_insert
#define ebmb_lookup_prefix ebamb_lookup_prefix
#define ebmb_insert_prefix ebamb_insert_prefix

#define ebmb_entry(ptr, type, member) ebamb_entry(ptr, type, member)

#define __ebim_lookup __ebaim_lookup
#define __ebim_insert __ebaim_insert

#define ebim_lookup ebaim_lookup
#define ebim_insert ebaim_insert

#define __ebis_lookup __ebais_lookup
#define __ebis_insert __ebais_insert

#define ebis_lookup ebais_lookup
#define ebis_lookup_len ebais_lookup_len
#define ebis_insert ebais_insert

#define __ebst_lookup __ebast_lookup
#define __ebst_insert __ebast_insert

#define ebst_lookup ebast_lookup
#define ebst_lookup_len ebast_lookup_len
#define ebst_insert ebast_insert

#endif
