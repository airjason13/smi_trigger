#ifndef _RPI_SMI_J_
#define _RPI_SMI_J_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "rpi_smi_defs.h"
#include "rpi_mem_j.h"
#include "utildbg.h"


#define BITS_PER_PIXEL_24    0
#define BITS_PER_PIXEL_48    1

#define ICLED_SMI_TIMING_TYPE_APA104    0
#define ICLED_SMI_TIMING_TYPE_AOS       1


int get_icled_timing_type(void);
int get_icled_bits_per_pixel(void);
int init_rpi_smi(int led_timing_type, int bits_per_pixel, bool gain_mode);
int rpi_set_smi_chan_led_count(int led_count);
int rpi_start_smi(int bpp, bool b_set_gain, int i_icled_timing);
void smi_terminate(int sig);
int rpi_set_smi_buffer_48bit(unsigned long ulrgbdata[1000][16]);
int set_test_buffer_48bit(unsigned long color);
int set_test_buffer_24bit(int color);
#endif
