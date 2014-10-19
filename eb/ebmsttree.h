/* Mapping of generic ebtree code to relative medium pointer code ("ebm").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBMSTTREE_H
#define _EBMSTTREE_H

/* redefine entries for mapping */
#undef __ebxst_lookup
#define __ebxst_lookup __ebmst_lookup
#undef __ebxst_insert
#define __ebxst_insert __ebmst_insert

/* exported functions below */
#undef ebxst_lookup
#define ebxst_lookup ebmst_lookup
#undef ebxst_insert
#define ebxst_insert ebmst_insert

#include "ebmtree.h"
#include "ebmmbtree.h"
#include "ebxsttree.h"

#endif
