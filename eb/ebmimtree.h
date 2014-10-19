/* Mapping of generic ebtree code to relative medium pointer code ("ebm").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBMIMTREE_H
#define _EBMIMTREE_H

/* redefine entries for mapping */
#undef __ebxim_lookup
#define __ebxim_lookup __ebmim_lookup
#undef __ebxim_insert
#define __ebxim_insert __ebmim_insert

/* exported functions below */
#undef ebxim_lookup
#define ebxim_lookup ebmim_lookup
#undef ebxim_insert
#define ebxim_insert ebmim_insert

#include "ebmtree.h"
#include "ebmmbtree.h"
#include "ebximtree.h"

#endif
