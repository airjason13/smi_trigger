#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "utildbg.h"
#include "jtimer.h"
#include "rpi_smi_j.h"

#define UNDEFINED		-1
#define TIMING_TYPE_AOS		"aos"
#define TIMING_TYPE_ANAPEX	"anapex"
#define BITS_48_STR		"48"
#define BITS_24_STR		"24"

/*#define COLOR_TYPE_RED		0
#define COLOR_TYPE_GREEN	1
#define COLOR_TYPE_BLUE		2*/

typedef enum{
	COLOR_RED = 1,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_WHITE,
};


unsigned long ul_rgb_data[1000][16];
unsigned int ui_rgb_data[1000][16];
#define BITS_8TO16_FACTOR	256

int set_current_gain_buffer(int i_bpp, int rgain, int ggain, int bgain)
{
	int m = 0;
	int n = 0;
	if(i_bpp == BITS_PER_PIXEL_24){
		unsigned int ui_rgb_gain = rgain << 16 | ggain << 8 | bgain << 0;
		for(m = 0; m < 15; m ++){
			for(n = 0; n < 999; n ++){
				ui_rgb_data[n][m] = ui_rgb_gain;
			}
		}			
	}else if(i_bpp == BITS_PER_PIXEL_48){
		log_error("not implemented yet!\n");
		return -1;
	}else{
		log_error("unknown bpp!\n");
		return -1;	
	}	
	return 0;
}

int set_test_pattern(int i_bpp, int color, int level)
{
	int m = 0;
	int n = 0;
	unsigned long ul_level = (unsigned long)level;
	unsigned long rgb_color_value = 0;
	if(i_bpp == BITS_PER_PIXEL_48){
		if((level < 0) || (level > 65535)){
			log_error("color level error!\n");
			return -1;
		}
		if(color == COLOR_RED){
			rgb_color_value = (ul_level*BITS_8TO16_FACTOR) << 32;
		}else if(color == COLOR_GREEN){
			rgb_color_value = (ul_level*BITS_8TO16_FACTOR) << 16;
		}else if(color == COLOR_BLUE){
			rgb_color_value = (ul_level*BITS_8TO16_FACTOR) << 0;
		}else if(color == COLOR_WHITE){
			rgb_color_value = (ul_level*BITS_8TO16_FACTOR) << 0 | (ul_level*BITS_8TO16_FACTOR) << 16 | (ul_level*BITS_8TO16_FACTOR) << 32;	
		}else{
			log_error("unknown color type %d!\n", color);
			return -1;
		}
		log_debug("rgb_color_value = %ld\n", rgb_color_value);
		for(m = 0; m < 15; m ++){
			for(n = 0; n < 999; n ++){
				ul_rgb_data[n][m] = rgb_color_value;
			}
		}
	}else if(i_bpp == BITS_PER_PIXEL_24){
		if(color == COLOR_RED){
			rgb_color_value = (level) << 16;
		}else if(color == COLOR_GREEN){
			rgb_color_value = (level) << 8;
		}else if(color == COLOR_BLUE){
			rgb_color_value = (level) << 0;
		}else if(color == COLOR_WHITE){
			rgb_color_value = (level) << 0 | (level) << 8 | (level) << 16;	
		}else{
			log_error("unknown color type %d!\n", color);
			return -1;
		}
		for(m = 0; m < 15; m ++){
			for(n = 0; n < 999; n ++){
				ui_rgb_data[n][m] = rgb_color_value;
			}
		}	
	}else{
		log_error("not supported!\n");
		return -1;
	}
	return 0;
}

int main(int argc, char* argv[]){
	int cmd_opt = 0;
	int i_icled_smi_timing_type = UNDEFINED;
	int i_bits_per_pixel = UNDEFINED;
	int i_color_type = UNDEFINED;
	int i_color_level = UNDEFINED;
	int i_red_current_gain = UNDEFINED;
	int i_green_current_gain = UNDEFINED;
	int i_blue_current_gain = UNDEFINED;
	bool b_current_gain_mode = false;
	while(1){
		cmd_opt = getopt(argc, argv, "ht:b:c:l:g:");
		if(cmd_opt == -1){
			break;
		}
		switch(cmd_opt){
			case 'h':
				printf("smi_trigger help\n");
				printf("-h : show these message!\n");
				printf("-t : icled timing type. aos/anapex\n");
				printf("-b : icled bits per pixel. 48/24\n");
				printf("-c : color type. r/g/b/w\n");
				printf("-l : color level. 0~65535\n");
				printf("-g : set current gain. rgain:ggain:bgain\r"
				       "     If you set this parameter, the excution will only set current gain\n");
				return 0;
			case 't':
				/*ICLed Timing, AOS or Anapex*/
				log_debug("cmdopt: t, optarg = %s", optarg);
				if(!strcmp(optarg, TIMING_TYPE_AOS)){
					i_icled_smi_timing_type = ICLED_SMI_TIMING_TYPE_AOS;
				}else if(!strcmp(optarg, TIMING_TYPE_ANAPEX)){
					i_icled_smi_timing_type = ICLED_SMI_TIMING_TYPE_APA104;	
				}else{
					log_error("unknow ICLED TIMING TYPE!\n");
					return -1;
				}
				break;
			case 'b':
				/*bits per pixel, 48 or 24 */
				log_debug("cmdopt: b, optarg = %s", optarg);
				if(!strcmp(optarg, BITS_48_STR)){
					i_bits_per_pixel = BITS_PER_PIXEL_48;
				}else if(!strcmp(optarg, BITS_24_STR)){
					i_bits_per_pixel = BITS_PER_PIXEL_24;	
				}else{
					log_error("unknow BITS Per Pixel!\n");
					return -1;
				}
				break;
			case 'c':
				/*color, r/g/b/w */
				log_debug("cmdopt: c, optarg = %s", optarg);
				if(!strcmp(optarg, "r")){
					i_color_type = COLOR_RED;
				}else if(!strcmp(optarg, "g")){
					i_color_type = COLOR_GREEN;
				}else if(!strcmp(optarg, "b")){
					i_color_type = COLOR_BLUE;
				}else if(!strcmp(optarg, "w")){
					i_color_type = COLOR_WHITE;
				}else{
					log_error("unknow Color Type!\n");
					return -1;	
				}
				log_debug("i_color_type = %d\n", i_color_type);
				break;
			case 'l':
				/*color level 0~65535*/
				log_debug("cmdopt: l, optarg = %s", optarg);
				i_color_level = atoi(optarg);
				if(i_bits_per_pixel == BITS_PER_PIXEL_48){
					if((i_color_level < 0)||(i_color_level > 65535)){
						log_error("color level is out of range");
						return -1;
					}
				}else if(i_bits_per_pixel == BITS_PER_PIXEL_24){
					if((i_color_level < 0)||(i_color_level > 255)){
						log_error("color level is out of range");
						return -1;
					}
				}
				break;
			case 'g':
				/*current gain value depends on ICLED
				 * usage: -g rg:gg:bg*/
				log_debug("cmdopt: g, optarg = %s", optarg);
				char *ch;
				int i_color_gain_count = 0;
				ch = strtok(optarg, ":");
				while(ch != NULL){
					i_color_gain_count ++;
					if(i_color_gain_count == 1){
						log_debug("reg gain :%s", ch);
						i_red_current_gain = atoi(ch);
					}else if(i_color_gain_count == 2){
						log_debug("green gain :%s", ch);	
						i_green_current_gain = atoi(ch);
					}else if(i_color_gain_count == 3){
						log_debug("blue gain : %s", ch);
						i_blue_current_gain = atoi(ch);
					}
					ch = strtok(NULL, ":");
				}
				if(i_color_gain_count < 3){
					log_error("optarg of current gain should be [reg_gain:green_gain:blue_gain] ");
					return -1;
				}
				b_current_gain_mode = true;
				break;

		}
	}
	log_debug("smi_trigger starts\n");
	log_debug("i_bits_per_pixel = %d\n", i_bits_per_pixel);	

	/*Init smi device*/	
	int iret = init_rpi_smi(i_icled_smi_timing_type, i_bits_per_pixel, b_current_gain_mode);
	if(iret < 0){
		log_error("init rpi smi error!\n");
		smi_terminate(0);
		return -1;
	}
	log_debug("smi init ok!\n");
	
	if(b_current_gain_mode == true){
		/*fill the current gain value with ul_rgb_data/ui_rgb_data*/
		iret = set_current_gain_buffer(i_bits_per_pixel, i_red_current_gain, i_green_current_gain, i_blue_current_gain);
		if(iret < 0){
			log_error("set test pattern error!\n");
			smi_terminate(0);
			return -1;
		}
		
		/*set smi rgb buffer with ul_rgb_data/ui_rgb_data*/
		if(i_bits_per_pixel == BITS_PER_PIXEL_48){
			log_debug("set smi buffer!\n");
			rpi_set_smi_buffer_48bit(ul_rgb_data);
		}else if(i_bits_per_pixel == BITS_PER_PIXEL_24){
			rpi_set_smi_buffer_24bit(ui_rgb_data);	
		}	

	}else{
		/*fill test pattern to ul_rgb_data/ui_rgb_data*/
		iret = set_test_pattern(i_bits_per_pixel, i_color_type, i_color_level);
		if(iret < 0){
			log_error("set test pattern error!\n");
			smi_terminate(0);
			return -1;
		}
		log_debug("set rgb test pattern  ok!\n");
		/*set smi rgb buffer with ul_rgb_data/ui_rgb_data*/
		if(i_bits_per_pixel == BITS_PER_PIXEL_48){
			log_debug("set smi buffer!\n");
			rpi_set_smi_buffer_48bit(ul_rgb_data);
		}else if(i_bits_per_pixel == BITS_PER_PIXEL_24){
			rpi_set_smi_buffer_24bit(ui_rgb_data);	
		}	
		log_debug("set smi test buffer ok!\n");
	}
	rpi_start_smi(i_bits_per_pixel, b_current_gain_mode, i_icled_smi_timing_type);
	log_debug("rpi smi trigger ok!\n");
	usleep(10000);
	smi_terminate(0);

	return 0;
}

