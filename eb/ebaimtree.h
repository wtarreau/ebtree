/* Mapping of generic ebtree code to absolute pointer code ("eba").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBAIMTREE_H
#define _EBAIMTREE_H

/* redefine entries for mapping */
#undef __ebxim_lookup
#define __ebxim_lookup __ebaim_lookup
#undef __ebxim_insert
#define __ebxim_insert __ebaim_insert

/* exported functions below */
#undef ebxim_lookup
#define ebxim_lookup ebaim_lookup
#undef ebxim_insert
#define ebxim_insert ebaim_insert

#include "ebatree.h"
#include "ebambtree.h"
#include "ebximtree.h"

#endif
