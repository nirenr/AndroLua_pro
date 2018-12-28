LOCAL_PATH := $(call my-dir)

# Need to build as libsocket_core.so
# since Android libs directory cannot have subdirectories
# and because all-in-one loader loads before Android asset loader
include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE := socket
LOCAL_SRC_FILES := \
	luasocket.c \
	timeout.c \
	buffer.c \
	io.c \
	auxiliar.c \
	options.c \
	inet.c \
	tcp.c \
	udp.c \
	except.c \
	select.c \
	usocket.c
LOCAL_STATIC_LIBRARIES := luajava
include $(BUILD_SHARED_LIBRARY)

# lua socket mime.core module (all-in-one loader should open libmime.so)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE := mime
LOCAL_SRC_FILES := mime.c
LOCAL_STATIC_LIBRARIES := luajava
include $(BUILD_SHARED_LIBRARY)
