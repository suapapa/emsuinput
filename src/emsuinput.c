#define LOG_TAG "uinput"
#ifdef ANDROID
//#define LOG_NDEBUG 0
#include <utils/Log.h>
#else
#include <stdio.h>
#define LOGE(FMT, ARGS...) printf("E/uinput: " FMT "\n", ## ARGS)
#define LOGW(FMT, ARGS...) printf("W/uinput: " FMT "\n", ## ARGS)
#define LOGD(FMT, ARGS...) printf("D/uinput: " FMT "\n", ## ARGS)
#define LOGV(FMT, ARGS...) printf("V/uinput: " FMT "\n", ## ARGS)
#endif

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include "emsuinput.h"

static int __batch_ioctl(int fd, int cid, int *vs, int vcnt)
{
	int i;
	int err = 0;

	for (i = 0; i < vcnt; i++) {
		err = ioctl(fd, cid, vs[i]);
		if (err) {
			LOGE("failed on batch_ioctl. cid=%d, i=%d : %s",
			     cid, i, strerror(err));
			break;
		}
	}

	return err;
}

#ifndef UINPUT_DEV_PATH
#define UINPUT_DEV_PATH "/dev/uinput"
/* #define UINPUT_DEV_PATH "/dev/misc/uinput" */
/* #define UINPUT_DEV_PATH "/dev/input/uinput" */
#endif

int emsuinput_device_create(const char *name,
			    int *keybits, int keybits_cnt,
			    int *relbits, int relbits_cnt)
{
	struct uinput_user_dev ui_dev;
	int err;

	if (name == NULL) {
		LOGE("got null for name");
		return -1;
	}
	// TODO: try other paths...
	int fd = open(UINPUT_DEV_PATH, O_WRONLY | O_NDELAY);
	if (fd == -1) {
		LOGE("failed to open %s", UINPUT_DEV_PATH);
		return -1;
	}

	if (keybits_cnt > 0) {
		if (keybits_cnt > KEY_MAX) {
			LOGE("invalid numbers of keybits");
			goto err_exit;
		}
		if (keybits == NULL) {
			LOGE("got null for keybits");
			goto err_exit;
		}
		ioctl(fd, UI_SET_EVBIT, EV_KEY);
		__batch_ioctl(fd, UI_SET_KEYBIT, keybits, keybits_cnt);
	}

	if (relbits_cnt > 0) {
		if (relbits_cnt > REL_MAX) {
			LOGE("invalid numbers of relbits");
			goto err_exit;
		}
		if (relbits == NULL) {
			LOGE("got null for relbits");
			goto err_exit;
		}
		ioctl(fd, UI_SET_EVBIT, EV_REL);
		__batch_ioctl(fd, UI_SET_RELBIT, relbits, relbits_cnt);
	}

	memset(&ui_dev, 0, sizeof(ui_dev));
	strncpy(ui_dev.name, name, UINPUT_MAX_NAME_SIZE);
	ui_dev.id.version = 4;
	ui_dev.id.bustype = BUS_USB;
	write(fd, &ui_dev, sizeof(ui_dev));

	err = ioctl(fd, UI_DEV_CREATE);
	if (err) {
		LOGE("failed to crate uinput device: %s", strerror(err));
		goto err_exit;
	}

	LOGI("uinput device(%d) created.", fd);
	return fd;

 err_exit:
	close(fd);
	return -1;
}

void emsuinput_device_destroy(int fd)
{
	int err = ioctl(fd, UI_DEV_DESTROY);
	if (err) {
		LOGE("failed to dectory uinput device");
	}

	close(fd);
}

int emsuinput_send_events(int fd, __u16 type,
			  __u16 * codes, __s32 * values, int cnt)
{
	struct input_event evt;
	int err;
	int i;

	gettimeofday(&evt.time, NULL);
	evt.type = type;

	for (i = 0; i < cnt; i++) {
		evt.code = codes[i];
		evt.value = values[i];
		err = write(fd, &evt, sizeof(evt));
		if (err) {
			LOGE("failed to send event %d/%d: %s",
			     i, cnt, strerror(err));
			return err;
		}
	}

	evt.type = EV_SYN;
	evt.code = SYN_REPORT;
	evt.value = 0;
	err = write(fd, &evt, sizeof(evt));
	if (err) {
		LOGE("failed to send sync report: %s", strerror(err));
		return err;
	}

	return 0;
}

int emsuinput_send_key_down(int fd, __u16 key_code)
{
	__s32 value = 1;
	return uinput_send_events(fd, EV_KEY, &key_code, &value, 1);
}

int emsuinput_send_key_up(int fd, __u16 key_code)
{
	__s32 value = 0;
	return uinput_send_events(fd, EV_KEY, &key_code, &value, 1);
}

int emsuinput_send_rel_xy(int fd, __s32 x, __s32 y)
{
	__u16 codes[2] = { REL_X, REL_Y };
	__s32 values[2] = { x, y };

	return uinput_send_events(fd, EV_REL, codes, values, 2);
}
