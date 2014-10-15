/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBTREE_H
#define _EBTREE_H

/* redefine entries for mapping */
#undef ebx_root
#define ebx_root eb_root
#undef ebx_troot_t
#define ebx_troot_t eb_troot_t
#undef ebx_node
#define ebx_node eb_node

#include "ebxtree.h"

#endif
