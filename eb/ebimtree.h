/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * It maps eb* names to eba* so that legacy applications can continue to use
 * eb* names.
 */

#ifndef _EBIMTREE_H
#define _EBIMTREE_H

#include "ebtree.h"
#include "ebmbtree.h"
#include "ebaimtree.h"

#define __ebim_lookup __ebaim_lookup
#define __ebim_insert __ebaim_insert

#define ebim_lookup ebaim_lookup
#define ebim_insert ebaim_insert

#endif
