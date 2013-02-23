#ifndef __HU_INPUT_H__
#define __HU_INPUT_H__

#include <linux/types.h>
#include <linux/input.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __emsuinput_context {
	int fd;
	struct input_event evt;
};

typedef struct __emsuinput_context emsuinput_context;

emsuinput_context *emsuinput_new_context(const char *name,
					 int *keybits, int keybits_cnt,
					 int *relbits, int relbits_cnt);
void emsuinput_release_context(emsuinput_context * context);

int emsuinput_send_events(emsuinput_context * ctx,
			  __u16 type, __u16 * codes, __s32 * values,
			  int cnt);
int emsuinput_send_key_down(emsuinput_context * ctx, __u16 key_code);
int emsuinput_send_key_up(emsuinput_context * ctx, __u16 key_code);
int emsuinput_send_rel_xy(emsuinput_context * ctx, __s32 x, __s32 y);

#ifdef __cplusplus
}
#endif
#endif
