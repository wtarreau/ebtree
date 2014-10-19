/* Mapping of generic ebtree code to absolute pointer code ("eba").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBASTTREE_H
#define _EBASTTREE_H

/* redefine entries for mapping */
#undef __ebxst_lookup
#define __ebxst_lookup __ebast_lookup
#undef __ebxst_insert
#define __ebxst_insert __ebast_insert

/* exported functions below */
#undef ebxst_lookup
#define ebxst_lookup ebast_lookup
#undef ebxst_insert
#define ebxst_insert ebast_insert

#include "ebatree.h"
#include "ebambtree.h"
#include "ebxsttree.h"

#endif
