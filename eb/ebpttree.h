/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * It maps eb* names to eba* so that legacy applications can continue to use
 * eb* names.
 */

#ifndef _EBPTTREE_H
#define _EBPTTREE_H

#include "ebtree.h"
#include "ebapttree.h"

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

#endif
