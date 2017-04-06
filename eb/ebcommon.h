#ifndef _EBCOMMON_H
#define _EBCOMMON_H

#include <stddef.h>
#include "../common/compiler.h"
#include "../common/tools.h"

/*************************************************\
 * a few constants and macros we need everywhere *
\*************************************************/

/* Number of bits per node, and number of leaves per node */
#define EB_NODE_BITS          1
#define EB_NODE_BRANCHES      (1 << EB_NODE_BITS)
#define EB_NODE_BRANCH_MASK   (EB_NODE_BRANCHES - 1)

/* Be careful not to tweak those values. The walking code is optimized for NULL
 * detection on the assumption that the following values are intact.
 */
#define EB_SIDE_LEFT     0
#define EB_SIDE_RGHT     1
#define EB_TYPE_LEAF     0
#define EB_TYPE_NODE     1

/* Tags to set in root->b[EB_SIDE_RGHT] :
 * - EB_NORMAL is a normal tree which stores duplicate keys.
 * - EB_UNIQUE is a tree which stores unique keys.
 */
#define EB_NORMAL   0
#define EB_UNIQUE   1

/* Return the structure of type <type> whose member <member> points to <ptr> */
#define eb_entry(ptr, type, member) container_of(ptr, type, member)

#endif /* _EBCOMMON_H */
