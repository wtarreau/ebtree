/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBMBTREE_H
#define _EBMBTREE_H

/* redefine entries for mapping */
#undef ebxmb_node
#define ebxmb_node ebmb_node

#include "ebtree.h"
#include "ebxmbtree.h"

#endif
