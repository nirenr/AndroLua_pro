LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE := gl

LOCAL_CFLAGS := -DANDROID_NDK \
                -DDISABLE_IMPORTGL

LOCAL_SRC_FILES := \
    luagl.c \
    luagl_const.c \
    luagl_util.c \
    

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog
LOCAL_STATIC_LIBRARIES := luajava

include $(BUILD_SHARED_LIBRARY)
