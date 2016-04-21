LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua $(LOCAL_PATH)/../luajava
LOCAL_MODULE     := jni
LOCAL_SRC_FILES  := jni.c
LOCAL_STATIC_LIBRARIES := luajava

include $(BUILD_SHARED_LIBRARY)
