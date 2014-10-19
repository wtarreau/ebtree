/* Compatibility layer for legacy ebtree code in absolute pointer mode.
 * It maps eb* names to eba* so that legacy applications can continue to use
 * eb* names.
 */

#ifndef _EBISTREE_H
#define _EBISTREE_H

#include "ebtree.h"
#include "ebmbtree.h"
#include "ebaistree.h"

#define __ebis_lookup __ebais_lookup
#define __ebis_insert __ebais_insert

#define ebis_lookup ebais_lookup
#define ebis_insert ebais_insert

#endif
