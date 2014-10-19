/* Mapping of generic ebtree code to relative long pointer code ("ebl").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBLISTREE_H
#define _EBLISTREE_H

/* redefine entries for mapping */
#undef __ebxis_lookup
#define __ebxis_lookup __eblis_lookup
#undef __ebxis_insert
#define __ebxis_insert __eblis_insert

/* exported functions below */
#undef ebxis_lookup
#define ebxis_lookup eblis_lookup
#undef ebxis_insert
#define ebxis_insert eblis_insert

#include "ebltree.h"
#include "eblmbtree.h"
#include "ebxistree.h"

#endif
