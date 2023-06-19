#ifndef _PICOUSB_H_
#define _PICOUSB_H_
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "libusb.h"
#include "errno.h"
#include "utildbg.h"

#define BULK_EP_OUT     0x01
#define BULK_EP_IN      0x82


struct libusb_device_handle *picousb_init(void);
int picousb_out_transfer(struct libusb_device_handle *h, unsigned char *data, int len);
int picousb_in_transfer(struct libusb_device_handle *h, unsigned char *data, int len);
int picousb_set_cmd(struct libusb_device_handle *h, char *cmd, char *recv_buf);
int reset_usb_device(struct libusb_device_handle *devh);
int picousb_close(struct libusb_device_handle *h );
int probe_pico(void);
int reset_usb_hub(void);
#endif
