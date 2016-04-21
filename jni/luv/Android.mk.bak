LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua $(LOCAL_PATH)/include $(LOCAL_PATH)/uv-src

LOCAL_MODULE     := luv

LOCAL_SRC_FILES  := uv-src/fs-poll.c \
uv-src/inet.c \
uv-src/threadpool.c \
uv-src/uv-common.c \
uv-src/version.c \
uv-src/unix/android-ifaddrs.c \
uv-src/unix/async.c \
uv-src/unix/core.c \
uv-src/unix/dl.c \
uv-src/unix/fs.c \
uv-src/unix/getaddrinfo.c \
uv-src/unix/getnameinfo.c \
uv-src/unix/linux-core.c \
uv-src/unix/linux-inotify.c \
uv-src/unix/linux-syscalls.c \
uv-src/unix/loop.c \
uv-src/unix/loop-watcher.c \
uv-src/unix/pipe.c \
uv-src/unix/poll.c \
uv-src/unix/process.c \
uv-src/unix/proctitle.c \
uv-src/unix/pthread-fixes.c \
uv-src/unix/signal.c \
uv-src/unix/stream.c \
uv-src/unix/tcp.c \
uv-src/unix/thread.c \
uv-src/unix/timer.c \
uv-src/unix/tty.c \
uv-src/unix/udp.c \
luv/luv.c


		
LOCAL_STATIC_LIBRARIES := luajava

include $(BUILD_SHARED_LIBRARY)

