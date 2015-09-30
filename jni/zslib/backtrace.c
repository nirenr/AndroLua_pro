#include "manifest.h"
#include <dlfcn.h>
#include <signal.h>

static mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef __ANDROID__

/* see: https://github.com/android/platform_system_core/blob/master/include/corkscrew/backtrace.h */

static void* libcorkscrew;

typedef struct map_info_t map_info_t;

typedef struct {
    uintptr_t absolute_pc;
    uintptr_t stack_top;
    size_t stack_size;
} backtrace_frame_t;

typedef struct {
    uintptr_t relative_pc;
    uintptr_t relative_symbol_addr;
    char* map_name;
    char* symbol_name;
    char* demangled_name;
} backtrace_symbol_t;

typedef struct {
    uintptr_t start;
    uintptr_t end;
    char* name;
} symbol_t;

typedef struct {
    symbol_t* symbols;
    size_t num_symbols;
} symbol_table_t;


/* see: https://github.com/android/platform_system_core/blob/master/include/corkscrew/backtrace.h */

enum { /* A hint for how big to make the line buffer for format_backtrace_line */
    MAX_BACKTRACE_LINE_LENGTH = 800
};

typedef ssize_t (*unwind_backtrace_signal_arch_t)(siginfo_t* si, void* sc,
                                                  const map_info_t* lst, backtrace_frame_t* bt,
                                                  size_t ignore_depth, size_t max_depth);
typedef ssize_t (*unwind_backtrace_t)(backtrace_frame_t* backtrace, size_t ignore_depth, size_t max_depth);

typedef map_info_t* (*acquire_my_map_info_list_t)();

typedef void (*release_my_map_info_list_t)(map_info_t* mi_list);

typedef void (*get_backtrace_symbols_t)(const backtrace_frame_t* backtrace,
                                        size_t frames, backtrace_symbol_t* symbols);
typedef void (*free_backtrace_symbols_t)(backtrace_symbol_t* symbols, size_t frames);

typedef void (*format_backtrace_line_t)(unsigned frameNumber, const backtrace_frame_t* frame,
                                        const backtrace_symbol_t* symbol, char* buffer, size_t bufferSize);

typedef symbol_table_t* (*load_symbol_table_t)(const char* filename);
typedef void (*free_symbol_table_t)(symbol_table_t* table);
typedef const symbol_t* (*find_symbol_t)(const symbol_table_t* table, uintptr_t addr);

static unwind_backtrace_t unwind_backtrace;
static unwind_backtrace_signal_arch_t unwind_backtrace_signal_arch;
static acquire_my_map_info_list_t acquire_my_map_info_list;
static release_my_map_info_list_t release_my_map_info_list;
static get_backtrace_symbols_t get_backtrace_symbols;
static free_backtrace_symbols_t free_backtrace_symbols;
static load_symbol_table_t load_symbol_table;
static free_symbol_table_t free_symbol_table;
static find_symbol_t find_symbol;
static format_backtrace_line_t format_backtrace_line;

static void* corkscrew_sym(const char* func) {
    void* s = dlsym(libcorkscrew, func);
    assertion(libcorkscrew != null && s != null, "libcorkscrew=%p func=%s s=%p", libcorkscrew, func, s);
    return s;
}

static void init() {
    if (libcorkscrew == null) {
        libcorkscrew = dlopen("libcorkscrew.so", RTLD_LAZY | RTLD_LOCAL);
    }
    if (libcorkscrew != null) {
        unwind_backtrace =  (unwind_backtrace_t)corkscrew_sym("unwind_backtrace");
        unwind_backtrace_signal_arch = (unwind_backtrace_signal_arch_t)corkscrew_sym("unwind_backtrace_signal_arch");
        acquire_my_map_info_list = (acquire_my_map_info_list_t)corkscrew_sym("acquire_my_map_info_list");
        release_my_map_info_list = (release_my_map_info_list_t)corkscrew_sym("release_my_map_info_list");
        get_backtrace_symbols = (get_backtrace_symbols_t)corkscrew_sym("get_backtrace_symbols");
        free_backtrace_symbols = (free_backtrace_symbols_t)corkscrew_sym("free_backtrace_symbols");
        load_symbol_table = (load_symbol_table_t)corkscrew_sym("load_symbol_table");
        free_symbol_table = (free_symbol_table_t)corkscrew_sym("free_symbol_table");
        find_symbol = (find_symbol_t)corkscrew_sym("find_symbol");
        format_backtrace_line = (format_backtrace_line_t)corkscrew_sym("format_backtrace_line");
    }
}

/* stacktrace_ intentionally not static because it is used (under locked mutex) in debug version of mem_free */
/* IMPORTANT: !stacktrace_ returns global static buffer! this is by design only for leaks detection */

char* stacktrace_(bool detailed, siginfo_t* sig_info, void* sig_context, int from) { /* ~213 microseconds */
    int errno_ = errno;
    /* must be always called under locked mutex because it uses static global memory
      (to avoid malloc and stack growth) */
    if (libcorkscrew == null) {
        init();
    }
    if (unwind_backtrace_signal_arch == null) {
        return null;
    }
    map_info_t* map_info = acquire_my_map_info_list();
    enum { N = 4096, MAX_DEPTH = 64 };
    int n = N;
    static char buf[N];
    char*p = buf;
    *p = 0;
    static unsigned char frames_storage[MAX_DEPTH * sizeof(backtrace_frame_t)];
    backtrace_frame_t* frames = (backtrace_frame_t*)frames_storage;
    ssize_t frame_count = 0;
    if (sig_info != null) {
        frame_count = unwind_backtrace_signal_arch(sig_info, sig_context, map_info, frames, 1, MAX_DEPTH);
    } else {
        frame_count = unwind_backtrace(frames, 0, MAX_DEPTH);
    }
    static unsigned char bs_storage[sizeof(backtrace_symbol_t) * MAX_DEPTH];
    backtrace_symbol_t* bs = (backtrace_symbol_t*)bs_storage;
    get_backtrace_symbols(frames, frame_count, bs);
    int k = 0;
    size_t i;
    for (i = from; i < (size_t)frame_count && n > 128; ++i) {
        const char* name = bs[i].demangled_name ? bs[i].demangled_name : bs[i].symbol_name;
        if (strequ(name, "dvmPlatformInvoke")) {
            break;
        }
        int pc = bs[i].relative_pc - bs[i].relative_symbol_addr;
        if (!detailed) {
            if (name != null) {
                k = snprintf(p, n, p == buf ? "%s+%d" : " <- %s+%d", name, pc);
            } else {
                k = snprintf(p, n, p == buf ? "%p" : " <- %p", ll2p(bs[i].relative_pc));
            }
            p += k;
            n -= k;
        } else {
            static char line[MAX_BACKTRACE_LINE_LENGTH];
            format_backtrace_line(i, &frames[i], &bs[i], line, MAX_BACKTRACE_LINE_LENGTH);
            if (bs[i].symbol_name != null) {
                // get_backtrace_symbols found the symbol's name with dladdr(3).
                k = snprintf(p, n, "%s", line);
                p += k;
                n -= k;
            } else {
                // We don't have a symbol. Maybe this is a static symbol, and we can look it up?
                symbol_table_t* st = null;
                if (bs[i].map_name != null) {
                    st = load_symbol_table(bs[i].map_name);
                }
                const symbol_t* symbol = null;
                if (st != null) {
                    symbol = find_symbol(st, frames[i].absolute_pc);
                }
                if (symbol != null) {
                    uintptr_t offset = frames[i].absolute_pc - symbol->start;
                    k = snprintf(p, n, "%s (%s%+d)", line, symbol->name, (unsigned int)offset);
                    p += k;
                    n -= k;
                } else {
                    k = snprintf(p, n, "%s", line);
                    p += k;
                    n -= k;
                }
                free_symbol_table(st);
            }
        }
    }
    if (i < frame_count && n > 5) {
        snprintf(p, n, " ...");
    }
    free_backtrace_symbols(bs, frame_count);
    release_my_map_info_list(map_info);
    errno = errno_;
    return buf;
}

#elif __MACH__

#include <execinfo.h>

char* stacktrace_(bool detailed, siginfo_t *sig_info, void* sig_context, int from) {
    int errno_ = errno;
    enum { N = 4096, MAX_DEPTH = 64 };
    int n = N;
    static char buf[N];
    char* p = buf;
    *p++ = '\n';
    n--;
    *p = 0;
    void* callstack[64];
    int i, frames = backtrace(callstack, 128);
    char** ss = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i) {
        int k = snprintf(p, n, "%s\n", ss[i]);
        p += k;
        n -= k;
    }
    free(ss);
    errno = errno_;
    return buf;
}

#else

char* stacktrace_(bool detailed, siginfo_t *sig_info, void* sig_context, int from) {
    return "stacktrace not implemented";
}

#endif

const char* stacktrace() {
    mutex_lock(&mutex);
    const char* s = stacktrace_(false, null, null, 3);
    s = s != null ? mem_strdup(s) : null;
    mutex_unlock(&mutex);
    return s;
}

const char* stacktrace_detailed(bool detailed, siginfo_t* sig_info, void* sig_context) {
    mutex_lock(&mutex);
    const char* s = stacktrace_(detailed, sig_info, sig_context, 2);
    s = s != null ? mem_strdup(s) : null;
    mutex_unlock(&mutex);
    return s;
}

void print_stacktrace_(const char* file, int line, const char* func) {
    mutex_lock(&mutex);
    trace_(file, line, func, "%s", stacktrace_(false, null, null, 3));
    mutex_unlock(&mutex);
}

static void sig_handler(int sig_no, siginfo_t* sig_info, void* sig_context) {
    static int n = 0;
    if (n++ == 1) {
        const char* s = stacktrace_detailed(true, sig_info, sig_context);
        if (s != null) {
            trace("backtrace=%s", s);
            mem_free(s);
        }
    }
    n--;
}

void sigaction_smoke_test() {
    struct sigaction sig_action = {{0}};
    sig_action.sa_sigaction = &sig_handler;
    sig_action.sa_flags = SA_SIGINFO;
    struct sigaction before = {{0}};
    int r = sigaction(SIGSEGV, &sig_action, &before);
    posix_ok(r);
    raise(SIGSEGV);
    r = sigaction(SIGSEGV, &before, null);
    posix_ok(r);
}

static void bar() {
    const char* s = stacktrace();
    trace("backtrace=%s", s);
    mem_free(s);
    print_stacktrace();
    sigaction_smoke_test();
}

static int foo(int i) {
    if (i == 0) {
        bar();
        return 0;
    }
    return foo(i - 1);
}

int backtrace_smoke_test() {
  return foo(2);
}
