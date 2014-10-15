/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBPTTREE_H
#define _EBPTTREE_H

/* redefine entries for mapping */
#undef ebxpt_node
#define ebxpt_node ebpt_node

#endif
