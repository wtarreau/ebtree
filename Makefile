OBJS = ebtree.o eb32tree.o eb64tree.o ebpttree.o

all: libebtree.a

libebtree.a: $(OBJS)
	$(AR) rv $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	-rm -fv libebtree.a $(OBJS) *~ *.rej core
