/* Mapping of generic ebtree code to relative short pointer code ("ebs").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBSSTTREE_H
#define _EBSSTTREE_H

/* redefine entries for mapping */
#undef __ebxst_lookup
#define __ebxst_lookup __ebsst_lookup
#undef __ebxst_insert
#define __ebxst_insert __ebsst_insert

/* exported functions below */
#undef ebxst_lookup
#define ebxst_lookup ebsst_lookup
#undef ebxst_insert
#define ebxst_insert ebsst_insert

#include "ebstree.h"
#include "ebsmbtree.h"
#include "ebxsttree.h"

#endif
