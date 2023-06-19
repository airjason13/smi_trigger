#include "rpi_mem_j.h"


int open_mbox(void){
	int fd;
	if((fd = open("/dev/vcio", 0)) < 0){
		printf("Error: can't open VC mailbox\n");
	}
	return (fd);
}

void close_mbox(int fd){
	if (fd >= 0){
		close(fd);
	}
}

void disp_vc_msg(VC_MSG *msgp){
	int i;
	log_debug("VC msg len=%X, req=%X, tag=%X, dlen=%x, data",
			       msgp->len, msgp->req, msgp->tag, msgp->blen, msgp->dlen);
	for(i = 0; i < msgp->blen/4;i ++){
		printf("%08X ", msgp->uints[i]);
	}
	printf("\n");

}


unsigned int msg_mbox(int fd, VC_MSG *msgp){
	unsigned int ret = 0, i;

	for (i = msgp->dlen/4; i <= msgp->blen/4; i+=4){
		msgp->uints[i++] = 0;
	}
	msgp->len = (msgp->blen +6) *4;
	msgp->req = 0;
	if(ioctl(fd, _IOWR(100, 0, void*), msgp) < 0){
		log_debug("VC IOCTL failed\n");
	}else if ((msgp->req&0x80000000) == 0){
		log_debug("VC IOCTL error\n");
	}else if (msgp->req == 0x80000001){
	    log_debug("VC IOCTL partial error\n");
	}else
		ret = msgp->uints[0];

#if 1
	disp_vc_msg(msgp);
#endif
	return (ret);
}

unsigned int alloc_vc_mem(int fd, unsigned int size, VC_ALLOC_FLAGS flags){
	unsigned int u32_ret = 0;
	VC_MSG msg={.tag=0x3000c, .blen=12, .dlen=12,
		.uints={PAGE_ROUNDUP(size), PAGE_SIZE, flags}};
	//u32_ret = msg_mbox(fd, &msg);
	//printf("alloc_vc_mem ret = 0x%x\n", msg_mbox(fd, &msg));
	return msg_mbox(fd, &msg);
}

void *lock_vc_mem(int fd, int h){
	VC_MSG msg={.tag=0x3000d, .blen=4, .dlen=4, .uints={h}};
	return (h ? (void *)msg_mbox(fd, &msg) : 0);
}

unsigned int unlock_vc_mem(int fd, int h){
	VC_MSG msg={.tag=0x3000e, .blen=4, .dlen=4, .uints={h}};
	return (h ? msg_mbox(fd, &msg) : 0);
}

unsigned int free_vc_mem(int fd, int h){
	VC_MSG msg={.tag=0x3000f, .blen=4, .dlen=4, .uints={h}};
	return (h ? msg_mbox(fd, &msg) : 0);

}

void *map_uncached_mem(MEM_MAP *mp, int size){
	void *ret;
	mp->size = PAGE_ROUNDUP(size);
    log_debug("mp->size = 0x%08x\n", mp->size);
	mp->fd = open_mbox();
#if 0
	ret = (mp->h = alloc_vc_mem(mp->fd, mp->size, DMA_MEM_FLAGS)) > 0 &&
		(mp->bus = lock_vc_mem(mp->fd, mp->h)) != 0 && 
		(mp->virt = map_segment(BUS_PHYS_ADDR(mp->bus), mp->size)) != 0
		? mp->virt : 0;
#else
    log_debug("ready to alloc vc mem!\n");
	mp->h = alloc_vc_mem(mp->fd, mp->size, DMA_MEM_FLAGS);
    if(mp->h <= 0){
        log_fatal("alloc vc mem failed!\n");
        return NULL;
    }
    mp->bus = lock_vc_mem(mp->fd, mp->h);
    if(mp->bus == 0){
        log_fatal("lock vc mem failed!\n");
        return NULL;
    }
	mp->virt = map_segment(BUS_PHYS_ADDR(mp->bus), mp->size);
    if(mp->virt == 0){
        log_fatal("mmap failed!\n");
        return 0;
    }
    
    
    
    
#endif
	mp->phys = BUS_PHYS_ADDR(mp->bus);
    log_debug("map_uncached_mem!\n");
	log_debug("1. mp->bus = %p\n", mp->bus);
	log_debug("2. mp->virt = %p\n", mp->virt);
#if 0
    mp->virt = (void*)((unsigned int)(mp->virt)&(~0xf0000000));
#else// work but not stable

    mp->real_virt = mp->virt;
	mp->virt = (unsigned long)mp->virt & 0x7fffffffff;
#endif
	//printf("mp->virt value : 0x%08x\n", *(unsigned long*)mp->virt);
	log_debug("VC mem handle : %u, phys %p, virt %p\n", mp->h, mp->bus, mp->virt);
	return (ret);
}
