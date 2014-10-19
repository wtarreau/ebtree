/* Mapping of generic ebtree code to relative long pointer code ("ebl").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBLIMTREE_H
#define _EBLIMTREE_H

/* redefine entries for mapping */
#undef __ebxim_lookup
#define __ebxim_lookup __eblim_lookup
#undef __ebxim_insert
#define __ebxim_insert __eblim_insert

/* exported functions below */
#undef ebxim_lookup
#define ebxim_lookup eblim_lookup
#undef ebxim_insert
#define ebxim_insert eblim_insert

#include "ebltree.h"
#include "eblmbtree.h"
#include "ebximtree.h"

#endif
