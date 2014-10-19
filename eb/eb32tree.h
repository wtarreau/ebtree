/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * It maps eb* names to eba* so that legacy applications can continue to use
 * eb* names.
 */

#ifndef _EB32TREE_H
#define _EB32TREE_H

#include "ebtree.h"
#include "eba32tree.h"

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

#endif
