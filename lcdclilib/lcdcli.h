#ifndef _LCDCLI_H_
#define _LCDCLI_H_
#include "utildbg.h"

int set_lcd_active(bool enable);
int lcd_send_command(int x, int y, char* cmd);
#endif
