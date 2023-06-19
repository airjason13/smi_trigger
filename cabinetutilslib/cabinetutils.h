#ifndef _CABINETUTILS_H_
#define _CABINETUTILS_H_

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "utildbg.h"

#define cabinet_params_config_folder "/home/root/.cabinets_params"
#define cabinet_params_config_file_prefix "/home/root/.cabinets_params/.cabinet_params_config_"
#define brightness_config_file "/home/root/.cabinets_params/.brightness_config"
#define contrast_config_file "/home/root/.cabinets_params/.contrast_config"
#define gamma_config_file "/home/root/.cabinets_params/.gamma_config"

typedef struct cabinet_params{
	unsigned int port_id; 
    unsigned int cabinet_width;
    unsigned int cabinet_height;
    unsigned int start_x;
    unsigned int start_y;
    int layout_type;
    struct libusb_device_handle *pico_handle;
}cabinet_params_t;

void write_cabinet_params_config_file(int port_id, struct cabinet_params *params);

int cabinet_params_init(unsigned int port_id, struct cabinet_params * cab_params);

void write_brightness_config_file(int value);
void write_contrast_config_file(int value);
void write_gamma_config_file(float value);
int get_brightness_from_config(void);
int get_contrast_from_config(void);
float get_gamma_from_config(void);


#endif
