/*
 * Elastic Binary Trees - example of application to network list reduction
 *
 * Copyright (C) 2000-2015 Willy Tarreau - w@1wt.eu
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ebatree.h>

struct one_net {
	struct ebamb_node eb_node;
	struct in_addr addr; /* keep it after eb_node */
	/* any other information related to this network could be stored here */
};

struct eba_root tree = EBA_ROOT;  /* EBA_ROOT || EBA_ROOT_UNIQUE */

/* Insert an address into the tree, after checking that it does not match
 * another one. If it does, then only one is kept or they are merged in a
 * larger network and the function recurses over itself. The address is in
 * network byte order.
 */
void insert_net(unsigned addr, unsigned cidr)
{
	unsigned mask;
	struct ebamb_node *node;
	struct one_net *net;

	/* clear unexpected bits */
	mask = cidr ? ((signed)-0x80000000) >> (cidr - 1) : 0;
	mask = htonl(mask);
	addr &= mask;

	/* 1) check if the entry already exists or matches an existing one. If we
	 * get a match, we have to compare prefixes and keep the widest one.
	 */
	if ((node = ebamb_lookup_longest(&tree, &addr)) != NULL) {
		if (node->node.pfx <= cidr)
			return;
		__ebamb_delete(node);
		free(node);
	}

	/* 2) check if we can merge this network with the one just below or above */
	if (cidr) {
		unsigned mask2, addr2;
		mask2 = cidr ? ((unsigned)0x80000000U) >> (cidr - 1) : 0;
		mask2 = htonl(mask2);

		/* invert the bit corresponding to the mask */
		addr2 = addr ^ mask2;
		node = ebamb_lookup_prefix(&tree, &addr2, cidr);

		if (node) {
			/* we can merge both entries at cidr - 1 */
			__ebamb_delete(node);
			free(eb_entry(node, struct one_net, eb_node));
			addr &= addr2; /* clear varying bit */
			cidr--;
			/* recursively do the same above */
			insert_net(addr, cidr);
			return;
		}
	}

	net = (struct one_net *)calloc(1, sizeof(*net));
	net->addr.s_addr = addr;
	net->eb_node.node.pfx = cidr;
	ebamb_insert_prefix(&tree, &net->eb_node, sizeof(net->addr.s_addr));

	/* 3) it is possible that this node covers other ones. All other ones
	 * will always be located just after this one, so let's walk right as
	 * long as we find some matches and kill them.
	 */
	node = ebamb_next(&net->eb_node);
	while (node) {
		net = eb_entry(node, struct one_net, eb_node);
		if ((addr & mask) != (net->addr.s_addr & mask))
			break;
		node = ebamb_next(&net->eb_node);
		__ebamb_delete(&net->eb_node);
	}

}

void read_nets_from_stdin()
{
	struct in_addr addr;
	char str[256];
	char *slash;
	int bits;

	while (fgets(str, sizeof(str), stdin) != NULL) {
		bits = 32;
		slash = strchr(str, '/');
		if (slash) {
			*(slash++) = 0;
			if (strchr(slash, '.') == NULL) {
				bits = atoi(slash);
			} else {
				inet_aton(slash, &addr);
				bits = htonl(addr.s_addr);
				if (bits)
					bits = 32 - flsnz(~bits);
			}
		}
		inet_aton(str, &addr);
		insert_net(addr.s_addr, bits);
	}
}

void dump_nets()
{
	struct ebamb_node *node = ebamb_first(&tree);

	while (node) {
		printf("%d.%d.%d.%d/%d\n",
		       node->key[0], node->key[1], node->key[2], node->key[3], node->node.pfx);
		node = ebamb_next(node);
		if (!node)
			break;
	}
}

int main(int argc, char **argv)
{
	if (argc > 1) {
		fprintf(stderr,
			"Enter networks one per line in the form <net>[/<mask>]\n"
			"The output will contain the smallest reduction of these nets.\n"
			);
		exit(1);
	}
	read_nets_from_stdin();
	dump_nets();
	return 0;
}
