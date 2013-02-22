#ifndef __HU_INPUT_H__
#define __HU_INPUT_H__

#include <linux/types.h>

int emsuinput_device_create(const char *name,
			    int *keybits, int keybits_cnt,
			    int *relbits, int relbits_cnt);
void emsuinput_device_destroy(int fd);

int emsuinput_send_events(int fd,
			  __u16 type, __u16 * codes, __s32 * values, int cnt);
int emsuinput_send_key_down(int fd, __u16 key_code);
int emsuinput_send_key_up(int fd, __u16 key_code);
int emsuinput_send_rel_xy(int fd, __s32 x, __s32 y);

#endif
