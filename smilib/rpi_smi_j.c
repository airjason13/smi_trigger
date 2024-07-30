#include "rpi_smi_j.h"

//rpi4 register base address
#define RPI4_REG_PHY_ADDR 	0xfe000000
	
#define RPI4_GPIO_OFFSET	0x00200000
#define RPI4_DMA_OFFSET		0x00007000
#define RPI4_SMI_OFFSET		0x00600000
#define RPI4_CLK_OFFSET		0x00101000

#define SHOW_DEBUG_MEM	false//true

#define APA104_SMI_TIMING 	10, 15, 30, 15
//#define AOS_SMI_TIMING 	    8, 10, 28, 10
//#define AOS_SMI_TIMING 	    10, 10, 28, 10 //333ns 666ns
//#define AOS_SMI_TIMING 	    10, 10, 25, 10 //290ns 600ns
//#define AOS_SMI_TIMING 	    10, 10, 23, 10 //290ns 560ns
#define AOS_SMI_TIMING 	    10, 10, 21, 10 //292ns 542ns

#define LED_D0_PIN	8
#define LED_NCHANS	16//8
#define LED_NBITS_24	24
#define LED_NBITS_48	48
#define LED_PREBITS	4
#define LED_POSTBITS	4
#define BIT_NPULSES	3	//how many pulses in one bit
#define CHAN_MAXLEDS	1000
#define CHASE_MSEC	100
#define REQUEST_THRESH	2
#define DMA_CHAN	5//6//10	//DMA channel to use?? 7~15 DMA CHAN is DMA lite(64k only)

int chan_ledcount= 960;
int test_rgb_data[CHAN_MAXLEDS][LED_NCHANS];
unsigned long test_ul_rgb_data[CHAN_MAXLEDS][LED_NCHANS];


//move to header
/*#define BITS_PER_PIXEL_24    0
#define BITS_PER_PIXEL_48    1

#define ICLED_SMI_TIMING_TYPE_APA104    0
#define ICLED_SMI_TIMING_TYPE_AOS       1*/

int i_icled_bits_per_pixel = BITS_PER_PIXEL_48;
int i_icled_smi_timing_type = ICLED_SMI_TIMING_TYPE_AOS;


// DMA channels and data requests
#define DMA_CHAN_A		10
#define DMA_CHAN_B		11
#define DMA_PWM_DREQ		5
#define DMA_SPI_TX_DREQ		6
#define DMA_SPI_RX_DREQ		7
#define DMA_BASE		(PHY_REG_BASE + 0x007000)
#define DMA_CS			0x00
#define DMA_CONBLK_AD		0x04
#define DMA_TI			0x08
#define DMA_SRCE_AD		0x0c
#define DMA_DEST_AD		0x10
#define DMA_TXFR_LEN		0x14
#define DMA_STRIDE		0x18
#define DMA_NEXTCONBK		0x1C
#define DMA_DEBUG		0x20
#define DMA_REG(ch, r)		((r)==DMA_ENABLE ? DMA_ENABLE : (ch)*0x100+(r))
#define DMA_ENABLE		0xff0

//DMA register values
/*#define DMA_WAIT_RESP		(1 << 3)
#define DMA_CB_DEST_INC		(1 << 4)
#define DMA_DEST_DREQ		(1 << 6)
#define DMA_CB_SRCE_INC		(1 << 8)*/


int on_rgbs[16] = { 0xff0000, 0x00ff00, 0x0000ff, 0xffffff,
		    0xff4040, 0x40ff40, 0x4040ff, 0x404040, 
		    0xff0000, 0x00ff00, 0x0000ff, 0xffffff,
		    0xff4040, 0x40ff40, 0x4040ff, 0x404040
			};
int off_rgbs[16];

#define CLK_PASSWD	0x5a000000

#define BUS_REG_BASE	0x7e000000

#define PAGE_SIZE	0x1000

#define REQUEST_THRESH	2


#define GPIO_IN		0	
#define GPIO_ALT1	5	

#define PAGE_ROUNDUP(n)	((n)%PAGE_SIZE==0 ? (n) : ((n)+PAGE_SIZE)&~(PAGE_SIZE-1))

#define REG8(m, x)	((volatile unsigned char *)((unsigned int)(m.virt)+(unsigned int)(x)))
//#define REG32(m, x)	((volatile unsigned int *)((unsigned int)(m.virt)+(unsigned int)(x)))
#define REG32(m, x)	((volatile unsigned int *)(m.virt+x))

//typedef struct {
//	int fd, h, size;
//	void *bus, *virt, *phys;
//} MEM_MAP;


MEM_MAP gpio_regs, dma_regs, clk_regs, pwm_regs, smi_regs, kclk_regs, vc_mem;



#define LED_DLEN_24BITS		(LED_NBITS_24 * BIT_NPULSES)  //24*3
#define LED_DLEN_48BITS		(LED_NBITS_48 * BIT_NPULSES)  //48*3

#if LED_NCHANS > 8
#define TXDATA_T		unsigned short
#else
#define TXDATA_T		unsigned char
#endif

#define LED_TX_OSET_24BIT(n) 		(LED_PREBITS + (LED_DLEN_24BITS * (n)))  // 4 + (24*3)*n 
#define LED_TX_OSET_48BIT(n) 		(LED_PREBITS + (LED_DLEN_48BITS * (n)))  // 4 + (48*3)*n 

//#define TX_BUFF_LEN_24BIT(n) 		(LED_TX_OSET_24BIT(n) + LED_POSTBITS)   // 4 + (24*3)*n + 4
#define TX_BUFF_LEN_24BIT(n) 		(LED_TX_OSET_24BIT(n) + 24 + LED_POSTBITS)   // 4 + (24*3)*n + (24) + 4  // the extra 24 if the current gain parameter of anapex_m
#define TX_BUFF_OFFSET_GAIN(n)		(LED_TX_OSET_24BIT(n))
#define TX_BUFF_LEN_48BIT(n) 		(LED_TX_OSET_48BIT(n) + LED_POSTBITS)   // 4 + (48*3)*n + 4

#define TX_BUFF_SIZE_8CHAN_24BIT(n)		(TX_BUFF_LEN_24BIT(n) * sizeof(unsigned short))  
#define TX_BUFF_SIZE_16CHAN_24BIT(n)		(TX_BUFF_LEN_24BIT(n) * sizeof(TXDATA_T))  

#define TX_BUFF_SIZE_8CHAN_48BIT(n)		(TX_BUFF_LEN_48BIT(n) * sizeof(unsigned short))  
#define TX_BUFF_SIZE_16CHAN_48BIT(n)		(TX_BUFF_LEN_48BIT(n) * sizeof(TXDATA_T))  

#define VC_MEM_SIZE_8CHAN_24BIT		    (PAGE_SIZE + TX_BUFF_SIZE_8CHAN_24BIT(CHAN_MAXLEDS))
#define VC_MEM_SIZE_8CHAN_48BIT		    (PAGE_SIZE + TX_BUFF_SIZE_8CHAN_48BIT(CHAN_MAXLEDS))
#define VC_MEM_SIZE_16CHAN_24BIT		(PAGE_SIZE + TX_BUFF_SIZE_16CHAN_24BIT(CHAN_MAXLEDS))
#define VC_MEM_SIZE_16CHAN_48BIT		(PAGE_SIZE + TX_BUFF_SIZE_16CHAN_48BIT(CHAN_MAXLEDS))

volatile SMI_CS_REG *smi_cs;
volatile SMI_L_REG *smi_l;
volatile SMI_A_REG *smi_a;
volatile SMI_D_REG *smi_d;
volatile SMI_DMC_REG *smi_dmc;
volatile SMI_DSR_REG *smi_dsr;
volatile SMI_DSW_REG *smi_dsw;
volatile SMI_DCS_REG *smi_dcs;
volatile SMI_DCA_REG *smi_dca;
volatile SMI_DCD_REG *smi_dcd;

#define TX_TEST		0//1

#if TX_TEST
#endif


TXDATA_T *txdata;
int tx_buf_w_index = 0;
int tx_buf_r_index = 0;
#define tx_buf_max_index  1

//anapex L 24bit icled current gain signal and reset 
int i_cg_signal_bytes = 13;
int i_reset_signal_bytes = 162;

TXDATA_T tx_buffer_24bit[tx_buf_max_index][TX_BUFF_LEN_24BIT(CHAN_MAXLEDS)];
TXDATA_T tx_buffer_48bit[tx_buf_max_index][TX_BUFF_LEN_48BIT(CHAN_MAXLEDS)];


TXDATA_T apa_m_gain[1][24];

int smi_start = 0;
pthread_mutex_t smi_lock;
pthread_mutex_t mem_lock;
int smi_quit = 0;





void rgb_txdata(int *rgbs, TXDATA_T *txd){
	int i, n, msk;
	for (n=0; n<LED_NBITS_24; n++){
		//msk = n==0? 0x8000: n==8 ? 0x800000 : n==16 ? 0x80 : msk>>1;
		//msk = n==0? 0x8000: n==8 ? 0x80 : n==16 ? 0x800000 : msk>>1;
        	msk = n==0 ? 0x8000 : n==8 ? 0x800000 : n==16 ? 0x80 : msk>>1;
        	//log_debug("msk : %d\n", msk);
		txd[0] = (TXDATA_T)0xffff;
		txd[1] = txd[2] = 0;
		for (i = 0; i < LED_NCHANS; i++){
			if(rgbs[i] & msk){
				txd[1] |= (1<<i);
			}
		}
		txd += BIT_NPULSES;
	}
}


void ul_rgb_txdata(unsigned long *ulrgbs, TXDATA_T *txd){
	unsigned long i, n, msk;
	for (n=0; n<LED_NBITS_48; n++){
		//msk = n==0? 0x800000000000: n==16 ? 0x80000000 : n==32 ? 0x8000 : msk>>1;
		//msk = n==0? 0x8000: n==16 ? 0x800000000000 : n==32 ? 0x80000000 : msk>>1;  //b->g->r
		msk = n==0? 0x80000000: n==16 ? 0x8000 : n==32 ? 0x800000000000 : msk>>1;  //b->g->r
		txd[0] = (TXDATA_T)0xffff;
		txd[1] = txd[2] = 0;
		for (i = 0; i < LED_NCHANS; i++){
			if(ulrgbs[i] & msk){
				txd[1] |= (1<<i);
			}
		}
		txd += BIT_NPULSES;
	}
}

void *map_segment_ul(void *addr, int size){
	int fd;
	void *mem;
	size = PAGE_ROUNDUP(size);

	if ((fd = open("/dev/mem", O_RDWR|O_SYNC|O_CLOEXEC)) < 0){
		log_fatal("Error: can't open /dev/mem!\n");
		return NULL;
	}
	mem = mmap(0, size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, (unsigned long)addr);
	log_debug("Map %p --> %p\n", (void *)addr, mem);
	//printf("*(unsigned int*)mem = %08x\n", *(unsigned int*)mem);
	if (mem == MAP_FAILED){
		log_fatal("Error, can't map memory!\n");
	}
	return mem;
}

void *map_segment(void *addr, int size){
	int fd;
	void *mem;
	size = PAGE_ROUNDUP(size);

	if ((fd = open("/dev/mem", O_RDWR|O_SYNC|O_CLOEXEC)) < 0){
		log_fatal("Error: can't open /dev/mem!\n");
		return NULL;
	}
	mem = mmap(0, size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, (unsigned int)addr);
	//printf("Map %p --> %p\n", (void *)addr, mem);
	//printf("*(unsigned int*)mem = %08x\n", *(unsigned int*)mem);
	if (mem == MAP_FAILED){
		log_fatal("Error, can't map memory!\n");
	}
	return mem;
}

void unmap_segment(void *mem, int size){
	if(mem){
		munmap(mem, PAGE_ROUNDUP(size));
	}
}

void *map_periph(MEM_MAP *mp, void *phys, int size){
	mp->phys = phys;
	mp->size = PAGE_ROUNDUP(size);
	mp->bus = (void *)((uint32_t)phys - RPI4_REG_PHY_ADDR + BUS_REG_BASE);
	mp->virt = map_segment(phys, mp->size);
    mp->real_virt = NULL;
	return (mp->virt);
}


void unmap_periph_mem(MEM_MAP *mp){
    unsigned int iret = 0;
	if(mp){
		if(mp->fd){
            if(mp->real_virt != NULL){
			    unmap_segment(mp->real_virt, mp->size);
            }else{
			    unmap_segment(mp->virt, mp->size);
            }
			iret = unlock_vc_mem(mp->fd, mp->h);
            log_debug("unlock vc mem iret = 0x%x\n", iret);
			iret = free_vc_mem(mp->fd, mp->h);
            log_debug("free vc mem iret = 0x%x\n", iret);
			close_mbox(mp->fd);
		}else{
            if(mp->real_virt != NULL){
			    unmap_segment(mp->real_virt, mp->size);
            }else{
			    unmap_segment(mp->virt, mp->size);
            }
			
		}

	}
}

unsigned int* get_periph_virt_addr_by_mmap(unsigned int periph_phy_addr){
	//1.get /dev/mem fd first
	int mem_fd = open("/dev/mem", O_RDWR);
	if(mem_fd < 0){
		log_fatal("Error:%d opening /dev/mem\n", mem_fd);
		return NULL;
	}
	unsigned int *periph_virt_addr = (unsigned int*)mmap(0, 4096, PROT_READ, MAP_SHARED, mem_fd, periph_phy_addr );
	close(mem_fd);
	//printf("periph_virt_addr = %p\n", periph_virt_addr);
	if(periph_virt_addr == MAP_FAILED){
		log_fatal("Error on mmap failed!\n");
		return NULL;
	}
	return periph_virt_addr;
}



int test_mmap_gpio_regs(int argc, char* argv[]){
	unsigned int gpio_phy_addr = RPI4_REG_PHY_ADDR + RPI4_GPIO_OFFSET;
	unsigned int *gpio_virt_addr = get_periph_virt_addr_by_mmap(gpio_phy_addr);

	if(SHOW_DEBUG_MEM){
		for(int i = 0; i < 6; i ++){
			log_debug("@%x:%08x\n", gpio_phy_addr+i*4, gpio_virt_addr[i]);
		}
	}

	return 0;
}


int map_devices(void){
	unsigned int gpio_phy_addr = RPI4_REG_PHY_ADDR + RPI4_GPIO_OFFSET;	
	map_periph(&gpio_regs, (void *)gpio_phy_addr, PAGE_SIZE);
	unsigned int dma_phy_addr = RPI4_REG_PHY_ADDR + RPI4_DMA_OFFSET;	
	map_periph(&dma_regs, (void *)dma_phy_addr, PAGE_SIZE);
	unsigned int clk_phy_addr = RPI4_REG_PHY_ADDR + RPI4_CLK_OFFSET;	
	map_periph(&kclk_regs, (void *)clk_phy_addr, PAGE_SIZE);
	unsigned int smi_phy_addr = RPI4_REG_PHY_ADDR + RPI4_SMI_OFFSET;	
	map_periph(&smi_regs, (void *)smi_phy_addr, PAGE_SIZE);


	
	return 0;
}

void enable_dma(int chan){
	*REG32(dma_regs, DMA_ENABLE) |= (1 << chan);
	*REG32(dma_regs, DMA_REG(chan, DMA_CS)) = 1 << 31;
}


void gpio_mode(int pin, int mode){
	volatile unsigned int *reg = REG32(gpio_regs, 0) + pin /10, shift = (pin % 10) *3;
	*reg = (*reg & ~(7 << shift)) | (mode << shift);
}

//
void init_smi(int width, int ns, int setup, int strobe, int hold){
	int i, divi = ns/2;

	smi_cs = (SMI_CS_REG *)REG32(smi_regs, SMI_CS);
	smi_l  = (SMI_L_REG *)REG32(smi_regs, SMI_L);
	smi_a  = (SMI_A_REG *)REG32(smi_regs, SMI_A);
	smi_d  = (SMI_D_REG *)REG32(smi_regs, SMI_D);
	smi_dmc  = (SMI_DMC_REG *)REG32(smi_regs, SMI_DMC);
	smi_dsr  = (SMI_DSR_REG *)REG32(smi_regs, SMI_DSR0);
	smi_dsw  = (SMI_DSW_REG *)REG32(smi_regs, SMI_DSW0);
	smi_dcs  = (SMI_DCS_REG *)REG32(smi_regs, SMI_DCS);
	smi_dca  = (SMI_DCA_REG *)REG32(smi_regs, SMI_DCA);
	smi_dcd  = (SMI_DCD_REG *)REG32(smi_regs, SMI_DCD);
	//printf("smi_cs->value = %8x\n", smi_cs->value);
	smi_cs->value = smi_l->value = smi_a->value = 0;
	smi_dsr->value = smi_dsw->value = smi_dcs->value = smi_dca->value = 0;
	//printf("smi_l->value = %8x\n", smi_l->value);	
	if (*REG32(kclk_regs, CLK_SMI_DIV) != divi << 12){
		*REG32(kclk_regs, CLK_SMI_CTL) = CLK_PASSWD | (1 << 5);
		usleep(10);
		while (*REG32(kclk_regs, CLK_SMI_CTL) & (1 << 7));
		usleep(10);
		*REG32(kclk_regs, CLK_SMI_DIV) = CLK_PASSWD | (divi << 12);
		usleep(10);
		*REG32(kclk_regs, CLK_SMI_CTL) = CLK_PASSWD | 6 | (1 << 4);
		usleep(10);
		while((*REG32(kclk_regs, CLK_SMI_CTL) & (1 << 7)) == 0);
		usleep(10);
	}
	if (smi_cs->seterr){
		smi_cs->seterr = 1;
	}
	smi_dsr->rsetup = smi_dsw->wsetup = setup;
	smi_dsr->rstrobe = smi_dsw->wstrobe = strobe;
	smi_dsr->rhold = smi_dsw->whold = hold;
	smi_dmc->panicr = smi_dmc->panicw = 8;
	smi_dmc->reqr = smi_dmc->reqw = REQUEST_THRESH;
	smi_dsr->rwidth = smi_dsw->wwidth = width;
}

void set_gpio_smi_mode(void){
	//set gpio to alt1 mode	
	for(int i = 0; i < LED_NCHANS; i++){
		gpio_mode(LED_D0_PIN+i, GPIO_ALT1);
	}
	usleep(1000);
}

void setup_smi_dma(MEM_MAP *mp, int nsamp){
	DMA_CB *cbs=mp->virt;
	txdata = (TXDATA_T *)(cbs + 1);
	smi_dmc->dmaen = 1;
	smi_cs->enable = 1;
	smi_cs->clear = 1;
	smi_cs->pxldat = 1;
	smi_l->len = nsamp*sizeof(TXDATA_T);
	//smi_l->len = test_len;//nsamp*sizeof(TXDATA_T);
    
	log_debug("nsamp = %d\n", nsamp);
	log_debug("sizeof(TXDATA_T) = %d\n", sizeof(TXDATA_T));
	log_debug("smi_l->len = 0x%08x\n", smi_l->len);
	smi_cs->write = 1;
	enable_dma(DMA_CHAN);
    // DMA_WAIT_RESP will cause the signal hang 
	//cbs[0].ti = DMA_DEST_DREQ | (DMA_SMI_DREQ << 16) | DMA_CB_SRCE_INC | DMA_WAIT_RESP;
	cbs[0].ti = DMA_DEST_DREQ | (DMA_SMI_DREQ << 16) | DMA_CB_SRCE_INC ;
	cbs[0].tfr_len = nsamp * sizeof(TXDATA_T) ;
	cbs[0].srce_ad = MEM_BUS_ADDR(mp, txdata);
	cbs[0].dest_ad = REG_BUS_ADDR(smi_regs, SMI_D);
}

void start_dma(MEM_MAP *mp, int chan, DMA_CB *cbp, unsigned int csval){
	*REG32(dma_regs, DMA_REG(chan, DMA_CONBLK_AD)) = MEM_BUS_ADDR(mp, cbp);
	*REG32(dma_regs, DMA_REG(chan, DMA_CS)) = 2;
	*REG32(dma_regs, DMA_REG(chan, DMA_DEBUG)) = 7;
	*REG32(dma_regs, DMA_REG(chan, DMA_CS)) = 1 | csval;
}

void stop_dma(int chan){
	if (dma_regs.virt){
		//printf("stop_dmai\n");
		*REG32(dma_regs, DMA_REG(chan, DMA_CS)) = 1 << 31;
	}
}

void rpi_start_dma(MEM_MAP *mp){
	DMA_CB *cbs = mp->virt;
	start_dma(mp, DMA_CHAN, &cbs[0], 0);
}


void start_smi(MEM_MAP *mp){ 
	//DMA_CB *cbs = mp->virt;
	//start_dma(mp, DMA_CHAN, &cbs[0], 0);
	
    usleep(10);
	smi_cs->start = 1;
}

void stop_smi(){
	stop_dma(DMA_CHAN);
	usleep(10);
	smi_cs->start = 0;
	usleep(10);
}

unsigned int dma_transfer_len(int chan){
	return (*REG32(dma_regs, DMA_REG(chan, DMA_TXFR_LEN)));
}

unsigned int dma_active(int chan){
	return ((*REG32(dma_regs, DMA_REG(chan, DMA_CS))) & 1);
}

void swap_bytes(void *data, int len){
	unsigned short *wp = (unsigned short *)data;
	len = (len + 1)/2;
	while (len-- > 0){
		*wp = __builtin_bswap16(*wp);
		wp ++;
	}
}

void smi_terminate(int sig){
	int i;
	log_debug("closing\n");
    smi_quit = 1;
    pthread_mutex_lock(&smi_lock);
	if(gpio_regs.virt){
		for(i = 0; i < LED_NCHANS; i++){
			gpio_mode(LED_D0_PIN+i, GPIO_IN);
		}
	}
	log_debug("closing---1\n");
	if(smi_regs.virt){
		*REG32(smi_regs, SMI_CS) =0;
	}
	log_debug("closing---2\n");
	stop_dma(DMA_CHAN);
	log_debug("closing---3\n");
	unmap_periph_mem(&vc_mem);
	log_debug("closing---4\n");
	unmap_periph_mem(&smi_regs);
	log_debug("closing---5\n");
	unmap_periph_mem(&dma_regs);
	log_debug("closing---6\n");
	unmap_periph_mem(&gpio_regs);
	log_debug("closing---end\n");
    pthread_mutex_unlock(&smi_lock);
    pthread_mutex_destroy(&smi_lock);
    pthread_mutex_destroy(&mem_lock);
    exit(0);
}


int get_icled_timing_type(void){
    return i_icled_smi_timing_type;
}

int get_icled_bits_per_pixel(void){
    return i_icled_bits_per_pixel;
}

int init_rpi_smi(int led_timing_type, int bits_per_pixel, bool gain_mode){
    if(pthread_mutex_init(&smi_lock, NULL) != 0){
        log_fatal("\nmutex smi_lock init failed!\n");
        exit(0);
    }
    if(pthread_mutex_init(&mem_lock, NULL) != 0){
        log_fatal("\nmutex mem_lock init failed!\n");
        exit(0);
    }
	map_devices(); 
    i_icled_smi_timing_type = led_timing_type;
    i_icled_bits_per_pixel = bits_per_pixel;
	signal(SIGINT, smi_terminate);
    if(i_icled_smi_timing_type == ICLED_SMI_TIMING_TYPE_AOS){
	    init_smi(LED_NCHANS>8 ? SMI_16_BITS : SMI_8_BITS, AOS_SMI_TIMING);	
    }else if(i_icled_smi_timing_type == ICLED_SMI_TIMING_TYPE_APA104){
	    init_smi(LED_NCHANS>8 ? SMI_16_BITS : SMI_8_BITS, APA104_SMI_TIMING);	 
    }else if(i_icled_smi_timing_type == ICLED_SMI_TIMING_TYPE_APAM){
	    log_debug("timing same as apa104!\n");
	    init_smi(LED_NCHANS>8 ? SMI_16_BITS : SMI_8_BITS, APA104_SMI_TIMING);	 
    }
	set_gpio_smi_mode();
	usleep(1000);
    log_debug("set gpio mode ok!\n");
#if LED_NCHANS <= 8
    
    if(i_icled_bits_per_pixel == BITS_PER_PIXEL_48){
	    map_uncached_mem(&vc_mem, VC_MEM_SIZE_8CHAN_48BIT);
	    setup_smi_dma(&vc_mem, TX_BUFF_LEN_48BIT(chan_ledcount));
        log_debug("48bit 8chan init dma len = %d\n", TX_BUFF_LEN_48BIT(chan_ledcount));
    }else if(i_icled_bits_per_pixel == BITS_PER_PIXEL_24){
	    map_uncached_mem(&vc_mem, VC_MEM_SIZE_8CHAN_24BIT);
	    setup_smi_dma(&vc_mem, TX_BUFF_LEN_24BIT(chan_ledcount));
    }else{
	    map_uncached_mem(&vc_mem, VC_MEM_SIZE_8CHAN_24BIT);
	    setup_smi_dma(&vc_mem, TX_BUFF_LEN_24BIT(chan_ledcount));
    }
#else
    if(gain_mode){
	    if(i_icled_bits_per_pixel == BITS_PER_PIXEL_48){
	    	map_uncached_mem(&vc_mem, VC_MEM_SIZE_16CHAN_48BIT);
	   	setup_smi_dma(&vc_mem, TX_BUFF_LEN_48BIT(chan_ledcount) + 960);
        	printf("48bit 16chan init dma len = %d\n", TX_BUFF_LEN_48BIT(chan_ledcount));
    	    }else if(i_icled_bits_per_pixel == BITS_PER_PIXEL_24){
	    	map_uncached_mem(&vc_mem, VC_MEM_SIZE_16CHAN_24BIT);
	    	//setup_smi_dma(&vc_mem, TX_BUFF_LEN_24BIT(chan_ledcount) + 960);
	    	setup_smi_dma(&vc_mem, TX_BUFF_LEN_24BIT(chan_ledcount) + (24*16*3));
        	printf("24bit 16chan init dma len = %d\n", TX_BUFF_LEN_24BIT(chan_ledcount) + (24*16*3));
    	    }else{
	    	map_uncached_mem(&vc_mem, VC_MEM_SIZE_16CHAN_24BIT);
	    	setup_smi_dma(&vc_mem, TX_BUFF_LEN_24BIT(chan_ledcount) + 960);
    	    }
    }else{
	    if(i_icled_bits_per_pixel == BITS_PER_PIXEL_48){
	    	map_uncached_mem(&vc_mem, VC_MEM_SIZE_16CHAN_48BIT);
	   	    setup_smi_dma(&vc_mem, TX_BUFF_LEN_48BIT(chan_ledcount));
        	printf("48bit 16chan init dma len = %d\n", TX_BUFF_LEN_48BIT(chan_ledcount));
    	    }else if(i_icled_bits_per_pixel == BITS_PER_PIXEL_24){
	    	map_uncached_mem(&vc_mem, VC_MEM_SIZE_16CHAN_24BIT);
	    	setup_smi_dma(&vc_mem, TX_BUFF_LEN_24BIT(chan_ledcount));
    	    }else{
	    	map_uncached_mem(&vc_mem, VC_MEM_SIZE_16CHAN_24BIT);
	    	setup_smi_dma(&vc_mem, TX_BUFF_LEN_24BIT(chan_ledcount));
    	    }
    }
#endif
    log_debug("setup smi dma ok!\n");
    return 0;
}

/***************************************************************
param : int led_count
return : always zero
purpose : set how many leds in one chan
***************************************************************/
int rpi_set_smi_chan_led_count(int led_count){
	chan_ledcount = led_count;
    return 0;   
}


int rpi_set_smi_buffer_24bit(unsigned int uirgbdata[1000][16]){

    if(i_icled_bits_per_pixel != BITS_PER_PIXEL_24){
        return -EINVAL;
    }
    pthread_mutex_lock(&mem_lock);
    if(i_icled_bits_per_pixel == BITS_PER_PIXEL_24){
	for(int n = 0; n < chan_ledcount; n++){
		rgb_txdata(uirgbdata[n], &tx_buffer_24bit[tx_buf_w_index][LED_TX_OSET_24BIT(n)]);
	}
        tx_buf_r_index = tx_buf_w_index;
        tx_buf_w_index += 1;
        if(tx_buf_w_index >= tx_buf_max_index){
        	tx_buf_w_index = 0;
        }
    }
    pthread_mutex_unlock(&mem_lock);
    return 0;
}

int rpi_set_smi_buffer_48bit(unsigned long ulrgbdata[1000][16]){

    if(i_icled_bits_per_pixel != BITS_PER_PIXEL_48){
        return -EINVAL;
    }
    pthread_mutex_lock(&mem_lock);
    if(i_icled_bits_per_pixel == BITS_PER_PIXEL_48){
	    for(int n = 0; n < chan_ledcount; n++){
		    ul_rgb_txdata(ulrgbdata[n], &tx_buffer_48bit[tx_buf_w_index][LED_TX_OSET_48BIT(n)]);
	    }
        tx_buf_r_index = tx_buf_w_index;
        tx_buf_w_index += 1;
        if(tx_buf_w_index >= tx_buf_max_index){
            tx_buf_w_index = 0;
        }
    }
    pthread_mutex_unlock(&mem_lock);
    return 0;
}


int set_test_buffer_48bit(unsigned long color){
    
    if(i_icled_bits_per_pixel != BITS_PER_PIXEL_48){
        return -EINVAL;
    }
    /*printf("Atxdata = %p\n", txdata);
    DMA_CB *cbs=vc_mem.virt;
    txdata = (TXDATA_T *)(cbs + 1);*/
	for(int m = 0; m < LED_NCHANS; m ++){
		for(int q = 0; q < CHAN_MAXLEDS; q ++){
			test_ul_rgb_data[q][m] = color;
		}
	}
    if(i_icled_bits_per_pixel == BITS_PER_PIXEL_48){
	    for(int n = 0; n < chan_ledcount; n++){
		    ul_rgb_txdata(test_ul_rgb_data[n], &tx_buffer_48bit[tx_buf_w_index][LED_TX_OSET_48BIT(n)]);
	    }
        /*printf("txdata = %p\n", txdata);
        printf("tx_buffer_48bit  = %p\n", tx_buffer_48bit);
        printf("TX_BUFF_SIZE_16CHAN_48BIT(chan_ledcount)  = %p\n", 
                    TX_BUFF_SIZE_16CHAN_48BIT(chan_ledcount));*/
        tx_buf_w_index += 1;
        if(tx_buf_w_index >= tx_buf_max_index){
            tx_buf_w_index = 0;
        }
	    memcpy(txdata, tx_buffer_48bit, TX_BUFF_SIZE_16CHAN_48BIT(chan_ledcount));
    }
    return 0;
}


int set_test_buffer_24bit(int color){
#if 0
    if(i_icled_bits_per_pixel != BITS_PER_PIXEL_48){
        return -EINVAL;
    }
	for(int m = 0; m < LED_NCHANS; m ++){
		for(int q = 0; q < CHAN_MAXLEDS; q ++){
			rgb_data[q][m] = color;
		}
	}
	for(int n = 0; n < chan_ledcount; n++){
		rgb_txdata(ul_rgb_data[n], &tx_buffer_24bit[LED_TX_OSET_24BIT(n)]);
	}
#if LED_NCHAN <= 8
	swap_bytes(tx_buffer_24bit, TX_BUFF_SIZE_8CHAN_24BIT(chan_ledcount));
#endif
	memcpy(txdata, tx_buffer_24bit, TX_BUFF_SIZE_16CHAN_24BIT(chan_ledcount));
#endif
    return 0;
}

/***************************************************************
param : void
return : always zero
purpose : trigger the smi and dma
***************************************************************/
#define TEST_RGB    0
int rpi_test_smi_rgb_buffer(void){
    //stop_smi();
#if TEST_RGB
    int color_shift = 0;
    int k = 0x00000000ffff;
    
    if(i_icled_bits_per_pixel == BITS_PER_PIXEL_48){
	    for(int m = 0; m < LED_NCHANS; m ++){
		    for(int q = 0; q < CHAN_MAXLEDS; q ++){
			    ul_rgb_data[q][m] = k << color_shift;
		    }
	    }
	    for(int n = 0; n < chan_ledcount; n++){
		    ul_rgb_txdata(ul_rgb_data[n], &tx_buffer_48BIT[LED_TX_OSET(n)]);
	    }
    }else if(i_icled_bits_per_pixel == BITS_PER_PIXEL_24){
	    for(int m = 0; m < LED_NCHANS; m ++){
		    for(int q = 0; q < CHAN_MAXLEDS; q ++){
			    rgb_data[q][m] = k << color_shift;
		    }
	    }
	    for(int n = 0; n < chan_ledcount; n++){
		    rgb_txdata(rgb_data[n], &tx_buffer_24BIT[LED_TX_OSET(n)]);
	    }
    }else{
	    for(int m = 0; m < LED_NCHANS; m ++){
		    for(int q = 0; q < CHAN_MAXLEDS; q ++){
			    rgb_data[q][m] = k << color_shift;
		    }
	    }
	    for(int n = 0; n < chan_ledcount; n++){
		    rgb_txdata(ul_rgb_data[n], &tx_buffer_48BIT[LED_TX_OSET(n)]);
	    } 
    }
#endif
}

//int rpi_start_smi(void){
int rpi_start_smi(int bpp, bool b_set_gain, int i_icled_timing){
    log_debug("");
    int target_count = 0;
    int i_n_cg_bytes = 0;/* number of curren gain signal bytes, include reset*/
    log_debug("size: %d\n", (CHAN_MAXLEDS));
    pthread_mutex_lock(&mem_lock);
    if(b_set_gain == false){
    	if(bpp == BITS_PER_PIXEL_48)
    		memcpy(txdata, tx_buffer_48bit[tx_buf_r_index], TX_BUFF_SIZE_16CHAN_48BIT(chan_ledcount));
    	else if(bpp == BITS_PER_PIXEL_24)
    		memcpy(txdata, tx_buffer_24bit[tx_buf_r_index], TX_BUFF_SIZE_16CHAN_24BIT(chan_ledcount));
    	else{
		log_debug("unknown bpp!\n");
    		pthread_mutex_unlock(&mem_lock);
		return -1;
    	}
    }else{
    	/* set current gain*/
	// ANAPEX L
	log_debug("set current gain data!\n");
	if(i_icled_timing == ICLED_SMI_TIMING_TYPE_APA104){
    		if(bpp == BITS_PER_PIXEL_24){
			for(int z = 0; z < 688;z ++){
				if(z < 50)
					*(txdata + z) = 0xffff;
				else
					*(txdata + z) = 0x0;

			}
    			memcpy(txdata + 688, tx_buffer_24bit[tx_buf_r_index], TX_BUFF_SIZE_16CHAN_24BIT(chan_ledcount));
			i_n_cg_bytes = 688;
		}else{
			log_debug("apa104 only 24bpp!\n");
    			pthread_mutex_unlock(&mem_lock);
			return -1;
    		}
	}else if(i_icled_timing == ICLED_SMI_TIMING_TYPE_APAM){
    		if(bpp == BITS_PER_PIXEL_24){
			// start of set the current gain parameter in the end of current gain data
			unsigned short cg_array[] = {	0xFFFF, 0xFFFF, 0x0000, //b'1
							0xFFFF, 0x0000, 0x0000, //b'0 
							0xFFFF, 0xFFFF, 0x0000, //b'1
							0xFFFF, 0xFFFF, 0x0000, //b'1 //0xB
							0xFFFF, 0x0000, 0x0000, //b'0 
							0xFFFF, 0xFFFF, 0x0000, //b'1
							0xFFFF, 0x0000, 0x0000, //b'0 
							0xFFFF, 0xFFFF, 0x0000, //b'1 //0x5
							};

			log_debug("sizeof(cg_rray): %d\n", sizeof(cg_array));
			for(int q = 0; q < sizeof(cg_array)/sizeof(unsigned short); q++){
				memcpy(&tx_buffer_24bit[tx_buf_r_index][TX_BUFF_OFFSET_GAIN(chan_ledcount) + q], &cg_array[q], 2);		
			}
			// end of set the current gain parameter in the end of current gain data
			
			log_debug("tx_buffer_24bit[tx_buf_r_index][TX_BUFF_OFFSET_GAIN(chan_ledcount)] = 0x%x", *(unsigned int*)&tx_buffer_24bit[tx_buf_r_index][TX_BUFF_OFFSET_GAIN(chan_ledcount)]);		
			log_debug("tx_buffer_24bit[tx_buf_r_index][TX_BUFF_OFFSET_GAIN(chan_ledcount) + 4] = 0x%x", *(unsigned int*)&tx_buffer_24bit[tx_buf_r_index][TX_BUFF_OFFSET_GAIN(chan_ledcount)] + 4);		
    			memcpy(txdata, tx_buffer_24bit[tx_buf_r_index], TX_BUFF_SIZE_16CHAN_24BIT(chan_ledcount) + 24*16);
			log_debug("TX_BUFF_SIZE_16CHAN_24BIT(chan_ledcount) = %d\n", 
					TX_BUFF_SIZE_16CHAN_24BIT(chan_ledcount));
			i_n_cg_bytes = 24; //24*16;
			log_debug("ICLED_SMI_TIMING_TYPE_APAM!\n");
		}else{
			log_debug("apa M only 24bpp!\n");
    			pthread_mutex_unlock(&mem_lock);
			return -1;
    		}

	}else{
		log_error("ICLED TYPE Current Gain not implemented!\n");
    		pthread_mutex_unlock(&mem_lock);
		return -1;	
	}
	log_debug("end of set current gain data!\n");
    }
    pthread_mutex_unlock(&mem_lock);
    pthread_mutex_lock(&smi_lock);
    if(smi_quit == 1){
        log_fatal("smi_quit!\n");
    	pthread_mutex_unlock(&smi_lock);
        return -1;
    }
    rpi_start_dma(&vc_mem);
    smi_l->len = 0;
        
    if(bpp == BITS_PER_PIXEL_48){
        //should be nsamp
    	//smi_l->len = TX_BUFF_LEN_48BIT(chan_ledcount) + 960;
    	smi_l->len = TX_BUFF_LEN_48BIT(chan_ledcount) + i_n_cg_bytes;
    	log_debug("smi_l->len = %d\n", TX_BUFF_LEN_48BIT(chan_ledcount)*sizeof(TXDATA_T) + i_n_cg_bytes);	
    }else if(bpp == BITS_PER_PIXEL_24){
        //should be nsamp
    	smi_l->len = TX_BUFF_LEN_24BIT(chan_ledcount) + i_n_cg_bytes;//TX_BUFF_LEN_24BIT(chan_ledcount)*sizeof(TXDATA_T) + i_n_cg_bytes;
    	//smi_l->len = TX_BUFF_LEN_24BIT(chan_ledcount)*sizeof(TXDATA_T) + i_n_cg_bytes;
    	//log_debug("smi_l->len = %d\n", TX_BUFF_LEN_24BIT(chan_ledcount)*sizeof(TXDATA_T) + i_n_cg_bytes);	
    	log_debug("smi_l->len = %d\n", smi_l->len);	
    }else{
	log_debug("unknown bpp!\n");
    	pthread_mutex_unlock(&smi_lock);
	return -1;
    }
    target_count = smi_l->len;
    start_smi(&vc_mem);
    int i = 0;    
    while(1){
        i ++;
        if(i > 100){
            log_fatal("break with smi hang!\n");
            break;
        }
        usleep(1000*10);
        unsigned int end_ret = *REG32(smi_regs, SMI_L);
        //log_debug("end_ret = %d\n", end_ret);
    	if(bpp == BITS_PER_PIXEL_48){
        	//if((end_ret % TX_BUFF_LEN_48BIT(chan_ledcount)*sizeof(TXDATA_T)) == 0){
        	if(smi_l->len >= target_count){
                    	end_ret = *REG32(smi_regs, SMI_L);
                    	log_debug("48bpp break end_ret = %d\n", end_ret);
            		usleep(1000*1);
            		break;
        	}
	    }else if(bpp == BITS_PER_PIXEL_24){
        	//if((end_ret % (TX_BUFF_LEN_24BIT(chan_ledcount)*sizeof(TXDATA_T)) + i_n_cg_bytes) == 0){
        	if(smi_l->len >= target_count){
                    	end_ret = *REG32(smi_regs, SMI_L);
                    	log_debug("24bpp break end_ret = %d\n", end_ret);
            		usleep(1000*1);
            		break;
        	}
	    }else{
		    log_debug("unknown bpp!\n");
    		pthread_mutex_unlock(&smi_lock);
		    return -1;
		
	    }
        //printf("smi working!\n");
    }
    stop_smi();
    smi_l->len = 0;
    log_debug("rpi_exit_smi!\n");
    pthread_mutex_unlock(&smi_lock);
    return 0;
} 


