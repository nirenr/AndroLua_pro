LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE     := md5
LOCAL_SRC_FILES  := md5lib.c \
		md5.c \
		compat-5.2.c
		
LOCAL_STATIC_LIBRARIES := luajava

include $(BUILD_SHARED_LIBRARY)

