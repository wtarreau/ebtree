/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EB64TREE_H
#define _EB64TREE_H

/* redefine entries for mapping */
#undef ebx64_node
#define ebx64_node eb64_node

#include "ebtree.h"
#include "ebx64tree.h"

#endif
