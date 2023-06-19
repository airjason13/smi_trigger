#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "utildbg.h"

#define REG_BUS_ADDR(m, x) 	((unsigned int)(m.bus) + (unsigned int)(x))
#define MEM_BUS_ADDR(mp, a) 	((unsigned int)a-(unsigned int)mp->virt+(unsigned int)mp->bus)
//#define MEM_BUS_ADDR(mp, a) 	(a-mp->virt+mp->bus)
//#define BUS_PHYS_ADDR(a) 	((void*)((unsigned int)(a)&~0xC0000000))
#define BUS_PHYS_ADDR(a) 	((void*)((unsigned int)(a)&~0xC0000000))
//#define BUS_PHYS_ADDR(a) 	((void*)((unsigned int)(a)&~0x00000000C0000000))

#define PAGE_SIZE	4096
#define PAGE_ROUNDUP(n) ((n)%PAGE_SIZE==0 ? (n) : ((n)+PAGE_SIZE)&~(PAGE_SIZE-1))

//DMA register values
#define DMA_WAIT_RESP		(1 << 3)
#define DMA_CB_DEST_INC		(1 << 4)
#define DMA_DEST_DREQ		(1 << 6)
#define DMA_CB_SRCE_INC		(1 << 8)
#define DMA_SRCE_DREQ		(1 << 10))
#define DMA_PRIORITY(n)		((n) << 16)


//DMA control block (must be 32 byte aligned)
typedef struct	{
	unsigned int ti, 
		     srce_ad, 
		     dest_ad,
		     tfr_len, 
		     stride,
		     next_cb,
		     debug,
		     unused;
} DMA_CB __attribute__ ((aligned(32)));


typedef struct {
	unsigned int len,
		     req,
		     tag, 
		     blen,
		     dlen;
	unsigned int uints[32 -5];
}VC_MSG __attribute__ ((aligned(16)));


typedef enum {
	MEM_FLAG_DISCARDABLE		= 1<<0,
	MEM_FLAG_NORMAL			= 0<<2,
	MEM_FLAG_DIRECT			= 1<<2,
	MEM_FLAG_COHERENT		= 2<<2,
	MEM_FLAG_ZERO			= 1<<4,
	MEM_FLAG_NO_INIT		= 1<<5,
	MEM_FLAG_HINT_PERMALOCK		= 1<<6,
	MEM_LFAG_L1_NONALLOCATING	=(MEM_FLAG_DIRECT | MEM_FLAG_COHERENT)
} VC_ALLOC_FLAGS;

#define DMA_MEM_FLAGS	(MEM_FLAG_DIRECT|MEM_FLAG_ZERO)

typedef struct {
	int fd, h, size;
	void *bus, *virt, *phys, *real_virt;
	//void *bus, *phys, *real_virt;
    //unsigned long *virt;
	//void *bus, *phys;
	//unsigned int *virt;
}MEM_MAP;

void *map_uncached_mem(MEM_MAP *mp, int size);
