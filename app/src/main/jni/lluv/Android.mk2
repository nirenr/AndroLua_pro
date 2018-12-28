LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/uv

LOCAL_MODULE     := lluv

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
                    lluv/l52util.c \
                    lluv/lluv.c \
                    lluv/lluv_check.c \
                    lluv/lluv_dns.c \
                    lluv/lluv_error.c \
                    lluv/lluv_fbuf.c \
                    lluv/lluv_fs.c \
                    lluv/lluv_fs_event.c \
                    lluv/lluv_fs_poll.c \
                    lluv/lluv_handle.c \
                    lluv/lluv_idle.c \
                    lluv/lluv_list.c \
                    lluv/lluv_loop.c \
                    lluv/lluv_misc.c \
                    lluv/lluv_pipe.c \
                    lluv/lluv_poll.c \
                    lluv/lluv_prepare.c \
                    lluv/lluv_process.c \
                    lluv/lluv_req.c \
                    lluv/lluv_signal.c \
                    lluv/lluv_stream.c \
                    lluv/lluv_tcp.c \
                    lluv/lluv_timer.c \
                    lluv/lluv_tty.c \
                    lluv/lluv_udp.c \
                    lluv/lluv_utils.c

		
LOCAL_STATIC_LIBRARIES := luajava

include $(BUILD_SHARED_LIBRARY)

