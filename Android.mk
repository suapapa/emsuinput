LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := src/emsuinput.c

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := emsuinput

include $(BUILD_STATIC_LIBRARY)
