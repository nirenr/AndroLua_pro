LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/uv

LOCAL_MODULE     := luv

LOCAL_SRC_FILES  := uv/unix/android-ifaddrs.c\
                    uv/unix/async.c\
                    uv/unix/core.c\
                    uv/unix/dl.c\
                    uv/unix/fs.c\
                    uv/unix/getaddrinfo.c\
                    uv/unix/getnameinfo.c\
                    uv/unix/linux-core.c\
                    uv/unix/linux-inotify.c\
                    uv/unix/linux-syscalls.c\
                    uv/unix/loop.c\
                    uv/unix/loop-watcher.c\
                    uv/unix/pipe.c\
                    uv/unix/poll.c\
                    uv/unix/process.c\
                    uv/unix/procfs-exepath.c\
                    uv/unix/proctitle.c\
                    uv/unix/pthread-fixes.c\
                    uv/unix/signal.c\
                    uv/unix/stream.c\
                    uv/unix/sysinfo-loadavg.c\
                    uv/unix/sysinfo-memory.c\
                    uv/unix/tcp.c\
                    uv/unix/thread.c\
                    uv/unix/tty.c\
                    uv/unix/udp.c\
                    uv/fs-poll.c\
                    uv/inet.c\
                    uv/threadpool.c\
                    uv/timer.c\
                    uv/uv-common.c\
                    uv/uv-data-getter-setters.c\
                    uv/version.c\
                    luv/luv.c

		
LOCAL_STATIC_LIBRARIES := luajava

include $(BUILD_SHARED_LIBRARY)

