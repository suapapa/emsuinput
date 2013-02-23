#define LOG_TAG "emsuinput"
#ifdef ANDROID
//#define LOG_NDEBUG 0
#include <utils/Log.h>
#else
#include <stdio.h>
#define LOGE(FMT, ARGS...) printf("E/uinput: " FMT "\n", ## ARGS)
#define LOGW(FMT, ARGS...) printf("W/uinput: " FMT "\n", ## ARGS)
#define LOGI(FMT, ARGS...) printf("I/uinput: " FMT "\n", ## ARGS)
#define LOGD(FMT, ARGS...) printf("D/uinput: " FMT "\n", ## ARGS)
//#define LOGV(FMT, ARGS...) printf("V/uinput: " FMT "\n", ## ARGS)
#define LOGV(FMT, ARGS...)
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
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

static int emsuinput_device_create(const char *name,
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

static void emsuinput_device_destroy(int fd)
{
	LOGV("%s", __func__);

	int err = ioctl(fd, UI_DEV_DESTROY);
	if (err) {
		LOGE("failed to dectory uinput device: %s", strerror(err));
	}

	close(fd);

	LOGI("uinput device destroyed");
}

emsuinput_context *emsuinput_new_context(const char *name,
					 int *keybits, int keybits_cnt,
					 int *relbits, int relbits_cnt)
{
	emsuinput_context *ctx;

	int fd = emsuinput_device_create(name,
					 keybits, keybits_cnt,
					 relbits, relbits_cnt);
	if (fd == -1) {
		LOGE("failed to create device");
		return NULL;
	}

	ctx = (emsuinput_context *) calloc(1, sizeof(emsuinput_context));
	if (ctx == NULL) {
		LOGE("failed to alloc memory for context");
	} else {
		ctx->fd = fd;
	}

	return ctx;
}

void emsuinput_release_context(emsuinput_context * ctx)
{
	if (ctx == NULL) {
		LOGE("got null for context");
		return;
	}

	emsuinput_device_destroy(ctx->fd);
	free(ctx);
}

int emsuinput_send_events(emsuinput_context * ctx, __u16 type,
			  __u16 * codes, __s32 * values, int cnt)
{
	int err;
	int i;
	struct input_event *evt = &(ctx->evt);

	LOGV("%s", __func__);

	gettimeofday(&(evt->time), NULL);
	evt->type = type;

	for (i = 0; i < cnt; i++) {
		evt->code = codes[i];
		evt->value = values[i];
		err = write(ctx->fd, evt, sizeof(struct input_event));
		if (err < 0) {
			LOGE("failed to send event %d/%d: %s",
			     i, cnt, strerror(err));
			return err;
		}
	}

	evt->type = EV_SYN;
	evt->code = SYN_REPORT;
	evt->value = 0;
	err = write(ctx->fd, evt, sizeof(struct input_event));
	if (err < 0) {
		LOGE("failed to send sync report: %s", strerror(err));
		return err;
	}

	return 0;
}

int emsuinput_send_key_down(emsuinput_context * ctx, __u16 key_code)
{
	__s32 value = 1;
	return emsuinput_send_events(ctx, EV_KEY, &key_code, &value, 1);
}

int emsuinput_send_key_up(emsuinput_context * ctx, __u16 key_code)
{
	__s32 value = 0;
	return emsuinput_send_events(ctx, EV_KEY, &key_code, &value, 1);
}

int emsuinput_send_rel_xy(emsuinput_context * ctx, __s32 x, __s32 y)
{
	__u16 codes[2] = { REL_X, REL_Y };
	__s32 values[2] = { x, y };

	LOGV("%s", __func__);

	return emsuinput_send_events(ctx, EV_REL, codes, values, 2);
}
