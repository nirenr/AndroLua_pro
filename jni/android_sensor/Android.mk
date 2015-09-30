LOCAL_PATH := $(call my-dir)

# sensor module to access Android sensor (accel, mag, gyr, etc.)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE := sensor
LOCAL_LDLIBS := -landroid -llog
LOCAL_SRC_FILES := luasensor.cpp
LOCAL_SHARED_LIBRARIES := luajava
include $(BUILD_SHARED_LIBRARY)
