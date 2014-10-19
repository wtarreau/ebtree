/* Mapping of generic ebtree code to relative short pointer code ("ebs").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBSIMTREE_H
#define _EBSIMTREE_H

/* redefine entries for mapping */
#undef __ebxim_lookup
#define __ebxim_lookup __ebsim_lookup
#undef __ebxim_insert
#define __ebxim_insert __ebsim_insert

/* exported functions below */
#undef ebxim_lookup
#define ebxim_lookup ebsim_lookup
#undef ebxim_insert
#define ebxim_insert ebsim_insert

#include "ebstree.h"
#include "ebsmbtree.h"
#include "ebximtree.h"

#endif
