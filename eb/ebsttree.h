/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * It maps eb* names to eba* so that legacy applications can continue to use
 * eb* names.
 */

#ifndef _EBSTTREE_H
#define _EBSTTREE_H

#include "ebtree.h"
#include "ebmbtree.h"
#include "ebasttree.h"

#define __ebst_lookup __ebast_lookup
#define __ebst_insert __ebast_insert

#define ebst_lookup ebast_lookup
#define ebst_insert ebast_insert

#endif
