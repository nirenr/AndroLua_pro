LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += -std=c99
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc
LOCAL_MODULE    := zslib
LOCAL_SRC_FILES := manifest.c queue.c mapsl.c mapll.c timestamp.c
LOCAL_SRC_FILES += mem.c trace.c
LOCAL_SRC_FILES += zashmem.c zmutex.c zsem.c looper.c socketio.c
LOCAL_SRC_FILES += receiver.c backtrace.c
LOCAL_SRC_FILES += zslib.c zslib_jni.c
LOCAL_LDLIBS    += -llog -ljnigraphics
include $(BUILD_SHARED_LIBRARY)
APP_MODULES += zslib


