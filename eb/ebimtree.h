/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBIMTREE_H
#define _EBIMTREE_H

/* redefine entries for mapping */
#undef __ebxim_lookup
#define __ebxim_lookup __ebim_lookup
#undef __ebxim_insert
#define __ebxim_insert __ebim_insert

/* exported functions below */
#undef ebxim_lookup
#define ebxim_lookup ebim_lookup
#undef ebxim_insert
#define ebxim_insert ebim_insert

#include "ebtree.h"
#include "ebmbtree.h"
#include "ebximtree.h"

#endif
