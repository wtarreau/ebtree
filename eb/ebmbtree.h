/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * It maps eb* names to eba* so that legacy applications can continue to use
 * eb* names.
 */

#ifndef _EBMBTREE_H
#define _EBMBTREE_H

#include "ebtree.h"

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

#endif
