/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EB32TREE_H
#define _EB32TREE_H

/* redefine entries for mapping */
#undef ebx32_node
#define ebx32_node eb32_node

#include "ebtree.h"
#include "ebx32tree.h"

#endif
