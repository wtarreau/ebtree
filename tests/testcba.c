#include <sys/time.h>

#include <inttypes.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cbatree.h"

void *cba_dump_tree_u32(struct cba_node *node, u32 pxor, void *last,
			int level,
			void (*node_dump)(struct cba_node *node, int level),
			void (*leaf_dump)(struct cba_node *node, int level));

struct cba_node *cba_insert_u32(struct cba_node **root, struct cba_node *node);
struct cba_node *cba_lookup_u32(struct cba_node **root, u32 key);


struct cba_node *cba_root = NULL;

struct key {
	struct cba_node node;
	uint32_t key;
};

static void dump_node(struct cba_node *node, int level)
{
	struct key *key = container_of(node, struct key, node);
	u32 pxor, lxor, rxor;

	/* xor of the keys of the two lower branches */
	pxor = container_of(__cba_clrtag(node->l), struct key, node)->key ^
		container_of(__cba_clrtag(node->r), struct key, node)->key;

	printf("  \"%lx_n\" [label=\"%lx\\nlev=%d\\nkey=%u\" fillcolor=\"lightskyblue1\"];\n",
	       (long)node, (long)node, level, key->key);

	/* xor of the keys of the left branch's lower branches */
	lxor = container_of(__cba_clrtag(((struct cba_node*)__cba_clrtag(node->l))->l), struct key, node)->key ^
		container_of(__cba_clrtag(((struct cba_node*)__cba_clrtag(node->l))->r), struct key, node)->key;

	printf("  \"%lx_n\" -> \"%lx_%c\" [taillabel=\"L\"];\n",
	       (long)node, (long)__cba_clrtag(node->l),
	       (((long)node->l & 1) || (lxor < pxor && ((struct cba_node*)node->l)->l != ((struct cba_node*)node->l)->r)) ? 'n' : 'l');

	/* xor of the keys of the right branch's lower branches */
	rxor = container_of(__cba_clrtag(((struct cba_node*)__cba_clrtag(node->r))->l), struct key, node)->key ^
		container_of(__cba_clrtag(((struct cba_node*)__cba_clrtag(node->r))->r), struct key, node)->key;

	printf("  \"%lx_n\" -> \"%lx_%c\" [taillabel=\"R\"];\n",
	       (long)node, (long)__cba_clrtag(node->r),
	       (((long)node->r & 1) || (rxor < pxor && ((struct cba_node*)node->r)->l != ((struct cba_node*)node->r)->r)) ? 'n' : 'l');
}

static void dump_leaf(struct cba_node *node, int level)
{
	struct key *key = container_of(node, struct key, node);

	printf("  \"%lx_l\" [label=\"%lx\\nlev=%d\\nkey=%u\\n\" fillcolor=\"yellow\"];\n",
	       (long)node, (long)node, level, key->key);
}

struct cba_node *add_value(struct cba_node **root, uint32_t value)
{
	struct key *key;

	key = calloc(1, sizeof(*key));
	key->key = value;
	return cba_insert_u32(root, &key->node);
}

int main(int argc, char **argv)
{
	const struct cba_node *old;
	char *orig_argv = argv[1];
	char *p;
	uint32_t v;


	while (argc > 1) {
		v = atoi(argv[1]);
		old = cba_lookup_u32(&cba_root, v);
		if (old)
			fprintf(stderr, "Note: value %u already present at %p\n", v, old);
		add_value(&cba_root, v);
		argv++;
		argc--;
	}

	/* rebuild args as a single string */
	for (p = orig_argv; p != *argv; *p++ = ' ')
		p += strlen(p);

	printf("digraph cba_tree_u32 {\n"
	       "  fontname=\"fixed\";\n"
	       "  fontsize=8\n"
	       "  label=\"%s\"\n"
	       "", orig_argv);

	printf("  node [fontname=\"fixed\" fontsize=8 shape=\"box\" style=\"filled\" color=\"black\" fillcolor=\"white\"];\n"
	       "  edge [fontname=\"fixed\" fontsize=8 style=\"solid\" color=\"magenta\" dir=\"forward\"];\n"
	       "  \"%lx_n\" [label=\"root\\n%lx\"]\n", (long)&cba_root, (long)&cba_root);

	cba_dump_tree_u32(cba_root, 0, NULL, 0, dump_node, dump_leaf);

	printf("}\n");
	return 0;
}
