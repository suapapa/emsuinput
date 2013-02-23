#include <unistd.h>
#include <stdlib.h>
#include <linux/input.h>
#include <emsuinput.h>

#define CHECKERR(E) {if(E) exit(E);}

int main()
{
	int i;
	int err = 0;
	emsuinput_context* ctx = NULL;

	int keybits[] = {
		BTN_LEFT,
	};

	int relbits[] = {
		REL_X,
		REL_Y,
	};

	ctx = emsuinput_new_context("fakemouse", keybits, 1, relbits, 2);
	if (ctx == NULL) {
		exit(-1);
	}

	for (i = 0; i < 100; i++) {
		err = emsuinput_send_rel_xy(ctx, 10, 10);
		CHECKERR(err);
		usleep(10 * 1000);
	}

	/* err = emsuinput_send_rel_xy(ctx, -3000, -3000); */
	/* err = emsuinput_send_rel_xy(ctx, 640, 480); */

	emsuinput_release_context(ctx);

	exit(0);

}
