LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS := -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../openssl/include
LOCAL_CFLAGS += -std=c99

CAL_ARM_MODE := arm
TARGET_PLATFORM := armeabi-v7a
TARGET_ABI := android-14-armeabi

LOCAL_MODULE     := sec
LOCAL_SRC_FILES  := context.c ssl.c x509.c openssl.c
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -lz -ldl
LOCAL_LDLIBS += $(LOCAL_PATH)/../openssl/libssl.a
LOCAL_LDLIBS += $(LOCAL_PATH)/../openssl/libcrypto.a
LOCAL_STATIC_LIBRARIES += luajava
LOCAL_STATIC_LIBRARIES += socket

include $(BUILD_SHARED_LIBRARY)
