/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * It maps eb* names to eba* so that legacy applications can continue to use
 * eb* names.
 */

#ifndef _EB64TREE_H
#define _EB64TREE_H

#include "ebtree.h"
#include "eba64tree.h"

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

#endif
