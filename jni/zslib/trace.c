#include "manifest.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

#define PREFIX "jni/"

static const char* unprefix(const char* file) {
    if (file != null) {
        const char* prefix = strstr(file, PREFIX);
        if (prefix != null) {
            file = prefix + strlen(PREFIX);
        }
    }
    return file;
}

bool assert_(const char* condition, const char* file, int line, const char* func, const char* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    char buf[16*1024];
    if (file != null) {
        file = unprefix(file);
        if (func != null) {
           sprintf(buf, "%s:%d %s ", file, line, func);
        } else {
           sprintf(buf, "%s:%d ", file, line);
        }
        sprintf(buf + strlen(buf), "assertion \"%s\" failed ", condition);
        vsprintf(buf + strlen(buf), fmt, vl);
    } else {
        sprintf(buf, "assertion \"%s\" failed ", condition);
        vsprintf(buf + strlen(buf), fmt, vl);
    }
    va_end(vl);
    const char* st = stacktrace();
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s %s", buf, st);
#else
    printf("%s %s\n", buf, st);
#endif
    *(int*)null = 1; /* crash here */
    return false;
}

static void log_(const char* file, int line, const char* func, const char* fmt, va_list vl) {
    char buf[16*1024];
    if (file != null) {
        file = unprefix(file);
        if (func != null) {
           sprintf(buf, "%s:%d %s ", file, line, func);
        } else {
           sprintf(buf, "%s:%d ", file, line);
        }
        vsprintf(buf + strlen(buf), fmt, vl);
    } else {
        vsprintf(buf, fmt, vl);
    }
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", buf);
#else
    printf("%s\n", buf);
#endif
}

void trace_(const char* file, int line, const char* func, const char* fmt, ...) {
    int errno_ = errno;
    va_list vl;
    va_start(vl, fmt);
    log_(file, line, func, fmt, vl);
    va_end(vl);
    errno = errno_;
}

void hexdump_(const char* file, int line, const char* func, void* data, int bytes) {
    int errno_ = errno;
    unsigned char* p = data;
    char s[1024];
    for (int i = 0; i < bytes; i += 16) {
        int len = bytes - i < 16 ? bytes - i : 16;
        for (int k = 0; k < 16; k++) {
            if (k < len) {
                sprintf(&s[k * 3 + k / 4], "%02X  ", p[k]);
            } else {
                strcpy(&s[k * 3 + k / 4], "    ");
            }
        }
        int a = strlen(s);
        for (int k = 0; k < len; k++) {
            s[a + k] = k < len && 32 <= p[k] && p[k] <= 127 ? p[k] : '?';
        }
        s[a + len] = 0;
        trace_(file, line, func, "%p: %s", p, s);
        p += 16;
    }
    errno = errno_;
}

