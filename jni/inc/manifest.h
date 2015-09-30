#ifndef MANIFEST_H
#define MANIFEST_H
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#ifdef __ANDROID__
# include <sys/atomics.h>
#endif
#ifdef __MACH__
# include <mach/mach.h>
# include <mach/task.h> /* defines PAGE_SIZE to 4096 */
#endif
#ifdef __GNUC__
/*
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic error "-Wunused-function"
#pragma GCC diagnostic error "-Wuninitialized"
*/
#if !defined(DEBUG) || defined(NDEBUG)  /* because some vars are used only in trace() or assert() */
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#endif /* __GNUC__ */

#define LOG_TAG "adb-zspace"
#define null NULL
#define countof(a) (sizeof(a) / sizeof((a)[0]))

#ifdef __cplusplus
extern "C" {
#endif

#include "trace.h"
#include "backtrace.h"
#include "timestamp.h"
#include "mem.h"
#include "mapsl.h"
#include "mapll.h"
#include "queue.h"
#include "zmutex.h"
#include "zsem.h"
#include "socketio.h"
#include "looper.h"
#include "receiver.h"
#include "zashmem.h"

#ifdef __MACH__
inline static int gettid() { uint64_t tid; pthread_threadid_np(NULL, &tid); return (pid_t)tid; }
#endif

#ifdef __cplusplus
}
#endif

#endif /* MANIFEST_H */
