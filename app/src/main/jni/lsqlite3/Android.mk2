LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE := lsqlite3
LOCAL_SRC_FILES := \
	lsqlite3.c \
	sqlite3.c 
LOCAL_STATIC_LIBRARIES := luajava
include $(BUILD_SHARED_LIBRARY)
