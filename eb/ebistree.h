/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBISTREE_H
#define _EBISTREE_H

/* redefine entries for mapping */
#undef __ebxis_lookup
#define __ebxis_lookup __ebis_lookup
#undef __ebxis_insert
#define __ebxis_insert __ebis_insert

#include "ebtree.h"
#include "ebmbtree.h"
#include "ebxistree.h"

#endif
