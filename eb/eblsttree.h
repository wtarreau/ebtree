/* Mapping of generic ebtree code to relative long pointer code ("ebl").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBLSTTREE_H
#define _EBLSTTREE_H

/* redefine entries for mapping */
#undef __ebxst_lookup
#define __ebxst_lookup __eblst_lookup
#undef __ebxst_insert
#define __ebxst_insert __eblst_insert

/* exported functions below */
#undef ebxst_lookup
#define ebxst_lookup eblst_lookup
#undef ebxst_insert
#define ebxst_insert eblst_insert

#include "ebltree.h"
#include "eblmbtree.h"
#include "ebxsttree.h"

#endif
