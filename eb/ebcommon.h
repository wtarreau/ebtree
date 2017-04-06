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

/* parent pointer tags used to find the branch side they're attached to. As a
 * special case, a node attached below the root has tag EB_SIDE_ROOT instead
 * of EB_SIDE_LEFT, since the root is not really a matter of left/right and we
 * need to know when we reach it. Note that it's mandatory here that the values
 * are assigned this way since they are also used as array indices.
 */
#define EB_SIDE_MASK     3
#define EB_SIDE_LEFT     0
#define EB_SIDE_RGHT     1
#define EB_SIDE_ROOT     (2 | EB_SIDE_LEFT)

/* branch pointer tags used to find the downstream node type */
#define EB_TYPE_MASK     1
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
