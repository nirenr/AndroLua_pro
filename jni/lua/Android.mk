LOCAL_PATH := $(call my-dir)

# Core Lua library
# compiled as static library to embed in liblua-activity.so
include $(CLEAR_VARS)
LOCAL_MODULE := lua
CAL_ARM_MODE := arm
TARGET_PLATFORM := armeabi-v7a
TARGET_ABI := android-14-armeabi
LOCAL_CFLAGS += -std=c99


LOCAL_SRC_FILES := \
	lapi.c \
	lauxlib.c \
	lbaselib.c \
	lbitlib.c \
	lcode.c \
	lcorolib.c \
	lctype.c \
	ldblib.c \
	ldebug.c \
	ldo.c \
	ldump.c \
	lfunc.c \
	lgc.c \
	linit.c \
	liolib.c \
	llex.c \
	lmathlib.c \
	lmem.c \
	loadlib.c \
	lobject.c \
	lopcodes.c \
	loslib.c \
	lparser.c \
	lstate.c \
	lstring.c \
	lstrlib.c \
	ltable.c \
	ltablib.c \
	ltm.c \
	lua.c \
	luac.c \
	lundump.c \
	lutf8lib.c \
	lvm.c \
	lzio.c 

# Auxiliary lua user defined file
# LOCAL_SRC_FILES += luauser.c
# LOCAL_CFLAGS := -DLUA_DL_DLOPEN -DLUA_USER_H='"luauser.h"'

LOCAL_CFLAGS := -DLUA_DL_DLOPEN
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog -ldl
include $(BUILD_STATIC_LIBRARY) 
