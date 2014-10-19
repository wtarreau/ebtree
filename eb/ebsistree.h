/* Mapping of generic ebtree code to relative short pointer code ("ebs").
 * Common function names are #defined before including internal files so that
 * their declaration causes the new name to be used instead. This results in
 * new names to appear in .o/.a.
 */

#ifndef _EBSISTREE_H
#define _EBSISTREE_H

/* redefine entries for mapping */
#undef __ebxis_lookup
#define __ebxis_lookup __ebsis_lookup
#undef __ebxis_insert
#define __ebxis_insert __ebsis_insert

/* exported functions below */
#undef ebxis_lookup
#define ebxis_lookup ebsis_lookup
#undef ebxis_insert
#define ebxis_insert ebsis_insert

#include "ebstree.h"
#include "ebsmbtree.h"
#include "ebxistree.h"

#endif
