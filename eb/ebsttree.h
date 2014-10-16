/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBSTTREE_H
#define _EBSTTREE_H

/* redefine entries for mapping */
#undef __ebxst_lookup
#define __ebxst_lookup __ebst_lookup
#undef __ebxst_insert
#define __ebxst_insert __ebst_insert

/* exported functions below */
#undef ebxst_lookup
#define ebxst_lookup ebst_lookup
#undef ebxst_insert
#define ebxst_insert ebst_insert

#include "ebtree.h"
#include "ebmbtree.h"
#include "ebxsttree.h"

#endif
