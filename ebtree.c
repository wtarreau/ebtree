
#include "ebtree.h"

int eb_delete(struct eb_node *node) {
    return __eb_delete(node);
}

struct eb32_node *eb32_insert(struct eb32_node *root, struct eb32_node *new) {
    return __eb32_insert(root, new);
}

struct eb32_node *eb32_lookup(struct eb32_node *root, unsigned long x) {
    return __eb32_lookup(root, x);
}

struct eb64_node *eb64_insert(struct eb64_node *root, struct eb64_node *new) {
    return __eb64_insert(root, new);
}
