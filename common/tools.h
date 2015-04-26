#ifndef _EBTREE_TOOLS_H
#define _EBTREE_TOOLS_H

#include "../common/compiler.h"

/*****************************************************************************\
 * a few types we need below. Some of these may sometimes already be defined *
\*****************************************************************************/

typedef unsigned short u16;
typedef   signed short s16;

typedef unsigned int u32;
typedef   signed int s32;

typedef unsigned long long u64;
typedef   signed long long s64;


/*************************************************\
 * a few constants and macros we need everywhere *
\*************************************************/

/* Linux-like "container_of". It returns a pointer to the structure of type
 * <type> which has its member <name> stored at address <ptr>.
 */
#ifndef container_of
#define container_of(ptr, type, name) ((type *)(((char *)(ptr)) - ((long)&((type *)0)->name)))
#endif

/* returns a pointer to the structure of type <type> which has its member <name>
 * stored at address <ptr>, unless <ptr> is 0, in which case 0 is returned.
 */
#ifndef container_of_safe
#define container_of_safe(ptr, type, name) \
	({ void *__p = (ptr); \
		__p ? (type *)(__p - ((long)&((type *)0)->name)) : (type *)0; \
	})
#endif

/******************************\
 * bit manipulation functions *
\******************************/

#if defined(__i386__) || defined(__x86_64__)

/* returns 1 to 32 for 1<<0 to 1<<31. Undefined for 0 */
static inline int flsnz32(unsigned int x)
{
	int r;
	__asm__("bsrl %1,%0\n"
	        : "=r" (r) : "rm" (x));
	return r + 1;
}

static inline int flsnz8(unsigned char x)
{
	int r;
	__asm__("movzbl %%al, %%eax\n"
		"bsrl %%eax,%0\n"
	        : "=r" (r) : "a" (x));
	return r + 1;
}

#else

/* returns 1 to 32 for 1<<0 to 1<<31. Undefined for 0. */
#define flsnz32(___a) ({ \
	register int ___x, ___bits = 0; \
	___x = (___a); \
	if (___x & 0xffff0000) { ___x &= 0xffff0000; ___bits += 16;} \
	if (___x & 0xff00ff00) { ___x &= 0xff00ff00; ___bits +=  8;} \
	if (___x & 0xf0f0f0f0) { ___x &= 0xf0f0f0f0; ___bits +=  4;} \
	if (___x & 0xcccccccc) { ___x &= 0xcccccccc; ___bits +=  2;} \
	if (___x & 0xaaaaaaaa) { ___x &= 0xaaaaaaaa; ___bits +=  1;} \
	___bits + 1; \
	})

static inline int flsnz8(unsigned int x)
{
	int ret = 0;
	if (x >> 4) { x >>= 4; ret += 4; }
	return ret + ((0xFFFFAA50U >> (x << 1)) & 3) + 1;
}


#endif

#if defined(__x86_64__)
static inline int flsnz64(unsigned long long x)
{
	unsigned long long r;
	__asm__("bsrq %1,%0\n"
	        : "=r" (r) : "rm" (x));
	return r + 1;
}

#else

/* returns 1 to 64 for 1<<0 to 1<<63 */
static inline int flsnz64(unsigned long long x)
{
	unsigned int h;
	unsigned int bits = 32;

	h = x >> 32;
	if (!h) {
		h = x;
		bits = 0;
	}
	return flsnz32(h) + bits;
}
#endif

#define flsnz(x) ((sizeof(x) > 4) ? flsnz64(x) : (sizeof(x) > 1) ? flsnz32(x) : flsnz8(x))

/* Compare blocks <a> and <b> byte-to-byte, from bit <ignore> to bit <len-1>.
 * Return the number of equal bits between strings, assuming that the first
 * <ignore> bits are already identical. It is possible to return slightly more
 * than <len> bits if <len> does not stop on a byte boundary and we find exact
 * bytes. Note that parts or all of <ignore> bits may be rechecked. It is only
 * passed here as a hint to speed up the check.
 */
static forceinline int equal_bits(const unsigned char *a,
				  const unsigned char *b,
				  int ignore, int len)
{
	for (ignore >>= 3, a += ignore, b += ignore, ignore <<= 3;
	     ignore < len; ) {
		unsigned char c;

		a++; b++;
		ignore += 8;
		c = b[-1] ^ a[-1];

		if (c) {
			/* OK now we know that old and new differ at byte <ptr> and that <c> holds
			 * the bit differences. We have to find what bit is differing and report
			 * it as the number of identical bits. Note that low bit numbers are
			 * assigned to high positions in the byte, as we compare them as strings.
			 */
			ignore -= flsnz8(c);
			break;
		}
	}
	return ignore;
}

/* check that the two blocks <a> and <b> are equal on <len> bits. If it is known
 * they already are on some bytes, this number of equal bytes to be skipped may
 * be passed in <skip>. It returns 0 if they match, otherwise non-zero.
 */
static forceinline int check_bits(const unsigned char *a,
				  const unsigned char *b,
				  int skip,
				  int len)
{
	int bit, ret;

	/* This uncommon construction gives the best performance on x86 because
	 * it makes heavy use multiple-index addressing and parallel instructions,
	 * and it prevents gcc from reordering the loop since it is already
	 * properly oriented. Tested to be fine with 2.95 to 4.2.
	 */
	bit = ~len + (skip << 3) + 9; /* = (skip << 3) + (8 - len) */
	ret = a[skip] ^ b[skip];
	if (unlikely(bit >= 0))
		return ret >> bit;
	while (1) {
		skip++;
		if (ret)
			return ret;
		ret = a[skip] ^ b[skip];
		bit += 8;
		if (bit >= 0)
			return ret >> bit;
	}
}


/* Compare strings <a> and <b> byte-to-byte, from bit <ignore> to the last 0.
 * Return the number of equal bits between strings, assuming that the first
 * <ignore> bits are already identical. Note that parts or all of <ignore> bits
 * may be rechecked. It is only passed here as a hint to speed up the check.
 * The caller is responsible for not passing an <ignore> value larger than any
 * of the two strings. However, referencing any bit from the trailing zero is
 * permitted. Equal strings are reported as a negative number of bits, which
 * indicates the end was reached.
 */
static forceinline int string_equal_bits(const unsigned char *a,
					 const unsigned char *b,
					 int ignore)
{
	int beg;
	unsigned char c;

	beg = ignore >> 3;

	/* skip known and identical bits. We stop at the first different byte
	 * or at the first zero we encounter on either side.
	 */
	while (1) {
		unsigned char d;

		c = a[beg];
		d = b[beg];
		beg++;

		c ^= d;
		if (c)
			break;
		if (!d)
			return -1;
	}
	/* OK now we know that a and b differ at byte <beg>, or that both are zero.
	 * We have to find what bit is differing and report it as the number of
	 * identical bits. Note that low bit numbers are assigned to high positions
	 * in the byte, as we compare them as strings.
	 */
	return (beg << 3) - flsnz8(c);
}

static forceinline int cmp_bits(const unsigned char *a, const unsigned char *b, unsigned int pos)
{
	unsigned int ofs;
	unsigned char bit_a, bit_b;

	ofs = pos >> 3;
	pos = ~pos & 7;

	bit_a = (a[ofs] >> pos) & 1;
	bit_b = (b[ofs] >> pos) & 1;

	return bit_a - bit_b; /* -1: a<b; 0: a=b; 1: a>b */
}

static forceinline int get_bit(const unsigned char *a, unsigned int pos)
{
	unsigned int ofs;

	ofs = pos >> 3;
	pos = ~pos & 7;
	return (a[ofs] >> pos) & 1;
}

#endif /* _EBTREE_TOOLS_H */
