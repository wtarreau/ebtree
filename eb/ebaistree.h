/* Mapping of generic ebtree code to absolute pointer code ("eba").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBAISTREE_H
#define _EBAISTREE_H

/* redefine entries for mapping */
#undef __ebxis_lookup
#define __ebxis_lookup __ebais_lookup
#undef __ebxis_insert
#define __ebxis_insert __ebais_insert

/* exported functions below */
#undef ebxis_lookup
#define ebxis_lookup ebais_lookup
#undef ebxis_insert
#define ebxis_insert ebais_insert

#include "ebatree.h"
#include "ebambtree.h"
#include "ebxistree.h"

#endif
