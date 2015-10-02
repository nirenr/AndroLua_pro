LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -DTCC_TARGET_ARM -DTCC_ARM_VFP
LOCAL_CFLAGS += -DTCC_TARGET_ARM -DTCC_ARM_EABI
LOCAL_CFLAGS += -std=c99
LOCAL_CFLAGS += -Wno-pointer-sign
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../extra
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE    := tcc
LOCAL_SRC_FILES := tccpp.c libtcc.c tccelf.c tccasm.c tccgen.c tccrun.c arm-gen.c
LOCAL_SRC_FILES += tcc_jni.c
LOCAL_STATIC_LIBRARIES := luajava

LOCAL_LDLIBS    += -llog
LOCAL_SHARED_LIBRARIES += zslib
include $(BUILD_SHARED_LIBRARY)
APP_MODULES += tcc


