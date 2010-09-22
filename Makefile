OBJS = ebtree.o eb32tree.o eb64tree.o ebmbtree.o ebsttree.o ebimtree.o ebistree.o
CFLAGS = -O3
EXAMPLES = $(basename $(wildcard examples/*.c))

all: libebtree.a

examples: ${EXAMPLES}

libebtree.a: $(OBJS)
	$(AR) rv $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

examples/%: examples/%.c libebtree.a
	$(CC) $(CFLAGS) -I. -o $@ $< -L. -lebtree

test: test32 test64 testst

test%: test%.c libebtree.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lebtree

clean:
	-rm -fv libebtree.a $(OBJS) *~ *.rej core test32 test64 testst ${EXAMPLES}

.PHONY: examples tests
