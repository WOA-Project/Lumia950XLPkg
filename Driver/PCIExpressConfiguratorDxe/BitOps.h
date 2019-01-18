#ifndef __BITOPS_H__
#define __BITOPS_H__

#define BITS_PER_LONG 64

/*
* Find the first set bit in a memory region.
*/
unsigned long find_first_bit(const unsigned long *addr, unsigned long size);

/**
* upper_32_bits - return bits 32-63 of a number
* @n: the number we're accessing
*
* A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
* the "right shift count >= width of type" warning when that quantity is
* 32-bits.
*/
#define upper_32_bits(n) ((UINT32)(((n) >> 16) >> 16))

/**
* lower_32_bits - return bits 0-31 of a number
* @n: the number we're accessing
*/
#define lower_32_bits(n) ((UINT32)(n))

#define SZ_4K				0x00001000
#define SZ_1M				0x00100000
#define SZ_2M				0x00200000
#define SZ_4M				0x00400000
#define SZ_8M				0x00800000
#define SZ_16M				0x01000000
#define SZ_32M				0x02000000
#define SZ_64M				0x04000000
#define SZ_128M				0x08000000
#define SZ_256M				0x10000000
#define SZ_512M				0x20000000

#endif