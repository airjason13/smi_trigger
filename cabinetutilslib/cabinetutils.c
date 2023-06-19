#include "cabinetutils.h"


void gen_cabinet_params_config_file(int port_id){
	char config_file_uri[256];
	FILE *config_file;
	char cmd[256];
	sprintf(cmd, "mkdir -p %s", cabinet_params_config_folder);
	system(cmd);
	sprintf(config_file_uri, "%s%d", cabinet_params_config_file_prefix, port_id);
	config_file = fopen(config_file_uri, "w");
	fprintf(config_file, "cabinet_width 0\n");
	fprintf(config_file, "cabinet_height 0\n");
	fprintf(config_file, "start_x 0\n");
	fprintf(config_file, "start_y 0\n");
	fprintf(config_file, "layout_type 0\n");
	log_debug("file close!\n");
	fclose(config_file);		
	system("sync");
}

void write_cabinet_params_config_file(int port_id, struct cabinet_params *params){
	char config_file_uri[256];
	FILE *config_file;
	char cmd[256];
	sprintf(cmd, "mkdir -p %s", cabinet_params_config_folder);
	system(cmd);
	sprintf(config_file_uri, "%s%d", cabinet_params_config_file_prefix, port_id);
	config_file = fopen(config_file_uri, "w");
	fprintf(config_file, "cabinet_width %d\n", params->cabinet_width);
	fprintf(config_file, "cabinet_height %d\n", params->cabinet_height);
	fprintf(config_file, "start_x %d\n", params->start_x);
	fprintf(config_file, "start_y %d\n", params->start_y);
	fprintf(config_file, "layout_type %d\n", params->layout_type);
	log_debug("file close!\n");
	fclose(config_file);		
	system("sync");
}

void write_brightness_config_file(int value){
	char config_file_uri[256];
	FILE *config_file;
	char cmd[256];
	sprintf(cmd, "mkdir -p %s", cabinet_params_config_folder);
	system(cmd);
	sprintf(config_file_uri, "%s", brightness_config_file);
	config_file = fopen(config_file_uri, "w");
	fprintf(config_file, "frame_brightness %d\n", value);
	fclose(config_file);		
	system("sync"); 
}

void write_contrast_config_file(int value){
	char config_file_uri[256];
	FILE *config_file;
	char cmd[256];
    log_debug("enter write_contrast_config_file!");
	sprintf(cmd, "mkdir -p %s", cabinet_params_config_folder);
	system(cmd);
	sprintf(config_file_uri, "%s", contrast_config_file);
	config_file = fopen(config_file_uri, "w");
	fprintf(config_file, "frame_contrast %d\n", value);
	fclose(config_file);		
	system("sync"); 
    log_debug("out write_contrast_config_file!");
}


void write_gamma_config_file(float value){
	char config_file_uri[256];
	FILE *config_file;
	char cmd[256];
    log_debug("");
	sprintf(cmd, "mkdir -p %s", cabinet_params_config_folder);
	system(cmd);
	sprintf(config_file_uri, "%s", gamma_config_file);
	config_file = fopen(config_file_uri, "w");
	fprintf(config_file, "frame_gamma %f\n", value);
	fclose(config_file);		
	system("sync"); 
    log_debug("");
}

int get_brightness_from_config(void){
	char config_file_uri[256];
	FILE *config_file;
    char tagbuf[256];
    int value=0;
	sprintf(config_file_uri, "%s", brightness_config_file);
	log_debug("config_file_uri : %s\n", config_file_uri);
	if( access( config_file_uri, F_OK ) == 0 ) {
    	// file exists

	} else {
    	// file doesn't exist
		log_info("brightness config file, generate one!\n");
        write_brightness_config_file(50);
	}
     
	config_file = fopen(config_file_uri, "r");
	fscanf(config_file, "%s %d\n", tagbuf, &value);	
    log_debug("frame brightness : %d", value);
    return value;
}

int get_contrast_from_config(void){
	char config_file_uri[256];
	FILE *config_file;
    char tagbuf[256];
    int value=0;
	sprintf(config_file_uri, "%s", contrast_config_file);
	log_debug("config_file_uri : %s\n", config_file_uri);
	if( access( config_file_uri, F_OK ) == 0 ) {
    	// file exists

	} else {
    	// file doesn't exist
		log_info("brightness config file, generate one!\n");
        write_contrast_config_file(0);
	}
    
    log_debug(""); 
	config_file = fopen(config_file_uri, "r");
    log_debug(""); 
	fscanf(config_file, "%s %d\n", tagbuf, &value);	
    log_debug("frame brightness : %d", value);
    return value;
}

float get_gamma_from_config(void){
	char config_file_uri[256];
	FILE *config_file;
    char tagbuf[256];
    float value=0;
	sprintf(config_file_uri, "%s", gamma_config_file);
	log_debug("config_file_uri : %s\n", config_file_uri);
	if( access( config_file_uri, F_OK ) == 0 ) {
    	// file exists

	} else {
    	// file doesn't exist
		log_info("gamma config file, generate one!\n");
        write_gamma_config_file(2.2);
	}
     
	config_file = fopen(config_file_uri, "r");
	fscanf(config_file, "%s %f\n", tagbuf, &value);	
    log_debug("frame gamma : %f", value);
    return value;
}

int cabinet_params_init(unsigned int port_id, struct cabinet_params *cab_params){
	char config_file_uri[256];
	FILE *config_file;
	char tagbuf[256];
	int value;

	sprintf(config_file_uri, "%s%d", cabinet_params_config_file_prefix, port_id);
	log_debug("config_file_uri : %s\n", config_file_uri);
	if( access( config_file_uri, F_OK ) == 0 ) {
    	// file exists

	} else {
    	// file doesn't exist
		log_info("no cabinet params config file, generate one!\n");
		gen_cabinet_params_config_file(port_id);
	}

	cab_params->port_id = port_id;
	config_file = fopen(config_file_uri, "r");
	for(int i = 0; i < 6; i++){
		fscanf(config_file, "%s %d\n", tagbuf, &value);	
		if(strcmp(tagbuf, "cabinet_width") == 0){
			cab_params->cabinet_width = value;
		}else if(strcmp(tagbuf, "cabinet_height") == 0){
			cab_params->cabinet_height = value;
		}else if(strcmp(tagbuf, "start_x") == 0){
			cab_params->start_x = value;
		}else if(strcmp(tagbuf, "start_y") == 0){
			cab_params->start_y = value;
		}else if(strcmp(tagbuf, "layout_type") == 0){
			cab_params->layout_type = value;
		}
	}
	return 0;
}

