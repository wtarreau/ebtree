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

#endif
