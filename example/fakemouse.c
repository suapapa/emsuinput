#include <unistd.h>
#include <stdlib.h>
#include <linux/input.h>
#include <emsuinput.h>

#define CHECKERR(E) {if(E) exit(E);}

int main()
{
	int i;
	int fd;
	int err = 0;

	int keybits[] = {
		BTN_LEFT,
	};

	int relbits[] = {
		REL_X,
		REL_Y,
	};

	fd = emsuinput_device_create("fakemouse", keybits, 1, relbits, 2);
	if (fd <= 0) {
		exit(-1);
	}

	for (i = 0; i < 100; i++) {
		err = emsuinput_send_rel_xy(fd, 10, 10);
		CHECKERR(err);
		usleep(10 * 1000);
	}

	/* err = emsuinput_send_rel_xy(fd, -3000, -3000); */
	/* err = emsuinput_send_rel_xy(fd, 640, 480); */

	emsuinput_device_destroy(fd);

	exit(0);

}
