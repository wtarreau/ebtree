/* Mapping of generic ebtree code to relative medium pointer code ("ebm").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBMISTREE_H
#define _EBMISTREE_H

/* redefine entries for mapping */
#undef __ebxis_lookup
#define __ebxis_lookup __ebmis_lookup
#undef __ebxis_insert
#define __ebxis_insert __ebmis_insert

/* exported functions below */
#undef ebxis_lookup
#define ebxis_lookup ebmis_lookup
#undef ebxis_insert
#define ebxis_insert ebmis_insert

#include "ebmtree.h"
#include "ebmmbtree.h"
#include "ebxistree.h"

#endif
