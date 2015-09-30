#include "manifest.h"
#ifdef __MACH__
#include <mach/thread_info.h>
#include <mach/mach_time.h>

static void mach_clock_gettime(int id, struct timespec* tm);
static void mach_clock_get_thread_time(struct timespec* tm);

#endif

enum { N = 1024, NANOSECONDS_IN_SECOND = 1000000000 }; /* 2 x 8KB hashmaps */

int64_t walltime() {
    struct timespec tm = {0};
#ifdef __ANDROID__
    clock_gettime(CLOCK_REALTIME, &tm);
#elif __MACH__
    mach_clock_gettime(REALTIME_CLOCK, &tm);
#else
    clock_gettime(CLOCK_REALTIME, &tm);
#endif
    return NANOSECONDS_IN_SECOND * (int64_t)tm.tv_sec + tm.tv_nsec;
}

int64_t cputime() {
    struct timespec tm = {0};
#ifdef __ANDROID__
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tm);
#elif __MACH__
    mach_clock_get_thread_time(&tm);
#else
    clock_gettime(CLOCK_REALTIME, &tm);
#endif
    return NANOSECONDS_IN_SECOND * (int64_t)tm.tv_sec + tm.tv_nsec;
}

void nsleep(int64_t nanoseconds) {
    struct timespec rq = { (long)(nanoseconds / NANOSECONDS_IN_SECOND), (long)(nanoseconds % NANOSECONDS_IN_SECOND) };
    struct timespec rm = { 0, 0 };
    nanosleep(&rq, &rm);
}

static mapll_t cpu() {
    static mapll_t c;
    if (c == null) {
        c = mapll_create(N);
    }
    return c;
}

static mapll_t wall() {
    static mapll_t w;
    if (w == null) {
        w = mapll_create(N);
    }
    return w;
}

static void traceHumanReadable(const char* file, int line, const char* func, const char* label, int64_t delta, int64_t wall) {
    if (delta < 10LL * 1000) {
        trace_(file, line, func, "time: \"%s\" %lld nanoseconds (wall: %lld)", label, delta, wall);
    } else if (delta < 10LL * 1000 * 1000) {
        trace_(file, line, func, "time: \"%s\" %lld microseconds (wall: %lld)", label, delta / 1000LL, wall / 1000LL);
    } else if (delta < 10LL * 1000 * 1000 * 1000) {
        trace_(file, line, func, "time: \"%s\" %lld milliseconds (wall: %lld)", label, delta / (1000LL * 1000), wall / (1000LL * 1000));
    } else {
        trace_(file, line, func, "time: \"%s\" %lld seconds (wall: %lld)", label, delta / (1000LL * 1000 * 1000), wall / (1000LL * 1000 * 1000));
    }
}

volatile int64_t timestamp_self_time; /* = 0 */
volatile bool timestamp_trace; /* = false */

static void timestamp_init_self_time() {
    if (timestamp_self_time == 0) {
        timestamp_self_time = 1;
        timestamp_(__FILE__, __LINE__, __func__, "timestamp-self-delta");
        timestamp_self_time = timestamp_(__FILE__, __LINE__, __func__, "timestamp-self-delta");
        if (timestamp_self_time <= 0) {
            timestamp_self_time = 1;
        }
        timestamp_trace = true;
    }
}

int64_t timestamp_(const char* file, int line, const char* func, const char* label) {
    /* returns 0 on the first use and delta in nanoseconds on second call >= 1 */
    timestamp_init_self_time();
    mapll_t c = cpu();
    mapll_t w = wall();
    int64_t ct = cputime();
    int64_t wt = walltime();
    int64_t lba = p2ll((char*)label) ^ (((uint64_t)gettid()) << 32);
    int64_t cts = mapll_remove(c, lba);
    if (cts == 0) {
        mapll_put(c, lba, ct);
        mapll_put(w, lba, wt);
        return 0;
    } else {
        int64_t wts = mapll_remove(w, lba);
        int64_t cpu_delta = ct - cts - timestamp_self_time;
        int64_t wall_delta = wt - wts - timestamp_self_time;
        cpu_delta = cpu_delta < 1 ? 1 : cpu_delta;
        wall_delta = wall_delta < 1 ? 1 : wall_delta;
        if (timestamp_trace) {
            traceHumanReadable(file, line, func, label, cpu_delta, wall_delta);
        }
        return cpu_delta;
    }
}

#ifdef DEBUG
static void clock_study() {
#ifdef __ANDROID__
    static int ids[] = { CLOCK_MONOTONIC, CLOCK_REALTIME, CLOCK_THREAD_CPUTIME_ID, CLOCK_PROCESS_CPUTIME_ID };
    static const char* names[] = { "CLOCK_MONOTONIC", "CLOCK_REALTIME", "CLOCK_THREAD_CPUTIME_ID", "CLOCK_PROCESS_CPUTIME_ID" };
#else
    static int ids[] = { CLOCK_MONOTONIC, CLOCK_REALTIME, CLOCK_THREAD_CPUTIME_ID, CLOCK_PROCESS_CPUTIME_ID };
    static const char* names[] = { "CLOCK_MONOTONIC", "CLOCK_REALTIME", "CLOCK_THREAD_CPUTIME_ID", "CLOCK_PROCESS_CPUTIME_ID" };
#endif
    static int iterations[] = {5, 10, 100, 1000, 100000};
    for (int k = 0; k < countof(ids); k++) {
        struct timespec rs;
        clock_getres(ids[k], &rs);
        trace("%s res: sec=%d nano=%d", names[k], rs.tv_sec, rs.tv_nsec);
        int64_t deltas[countof(iterations)];
        for (int j = 0; j < countof(iterations); j++) {
            int n = iterations[j];
            struct timespec ts[n];
            for (int i = 0; i < countof(ts); i++) {
                clock_gettime(ids[k], &ts[i]);
            }
            int64_t max = 0;
            for (int i = 1; i < countof(ts); i++) {
                int64_t t0 = ts[i-1].tv_sec * 1000000000LL + ts[i-1].tv_nsec;
                int64_t t1 = ts[i].tv_sec * 1000000000LL + ts[i].tv_nsec;
                int64_t delta = t1 - t0;
//              trace("clock_gettime nano=%lld delta=%lld %s", t1, delta, names[k]);
                max = delta > max ? delta : max;
            }
            deltas[j] = max;
        }
        char buf[4096];
        char* p = buf;
        *p = 0;
        for (int j = 0; j < countof(iterations); j++) {
            char s[128];
            sprintf(s, "%lld (%d) ", deltas[j], iterations[j]);
            p += strlen(strcat(p, s));
        }
        trace("max delta in nanoseconds(iterations) = %s", buf);
    }
}
#endif

/*  OMAP5 board:

    CLOCK_MONOTONIC res: sec=0 nano=1
    max delta in nanoseconds(iterations) = 488 (5) 326 (10) 2,116 (100) 7,650 (1000) 902,995 (100000)
    CLOCK_REALTIME_HR res: sec=0 nano=1
    max delta in nanoseconds(iterations) = 2441 (5) 325 (10) 2,116 (100) 6,022 (1000) 363,444 (100000)
    CLOCK_REALTIME res: sec=0 nano=1
    max delta in nanoseconds(iterations) = 488 (5) 326 (10) 1,954 (100) 7,976 (1000) 515,625 (100000)
    CLOCK_THREAD_CPUTIME_ID res: sec=0 nano=1
    max delta in nanoseconds(iterations) = 0 (5) 30,517 (10) 3,0518 (100) 30518 (1000) 152,587 (100000)
    CLOCK_PROCESS_CPUTIME_ID res: sec=0 nano=1
    max delta in nanoseconds(iterations) = 30,518 (5) 30,518 (10) 30,518 (100) 30,518 (1000) 61,036 (100000)

   numbers suggest ~ 0.3 millisecond thread switching and ~1.0 millisecond process scheduling resolution
*/

#if defined(DEBUG) || defined(TIMESTAMP)

extern void mem_init(void);

__attribute__((constructor)) /* alternatively LOCAL_LDFLAGS=-Wl,-init,foo -fini,bar */
void timestamp_init(void) {
    mem_init(); /* because order of constructor is no guaranteed */
    timestamp_init_self_time();
    mem_clear_leaks(); /* do not account for global maps for timestamps */
#ifdef DEBUG
    if (false) { /* switch to true to see real clock resolution */
        clock_study();
    }
#endif
}

#ifdef __MACH__

static void mach_clock_gettime(int id, struct timespec* tm) {
    static mach_timebase_info_data_t tb;
    uint64_t t = mach_absolute_time();
    if (tb.denom == 0) {
        mach_timebase_info(&tb);
    }
    t = t * tb.numer / tb.denom;
    tm->tv_sec = (__darwin_time_t)(t / NANOSECONDS_IN_SECOND);
    tm->tv_nsec = (long)(t % NANOSECONDS_IN_SECOND);
}

static void mach_clock_get_thread_time(struct timespec* tm) {
    struct thread_basic_info tbi = {{0}};
    mach_msg_type_number_t flag = THREAD_BASIC_INFO_COUNT;
    int r = thread_info(mach_thread_self(), THREAD_BASIC_INFO, (task_info_t)&tbi, &flag);
    if (r == KERN_SUCCESS) {
        tm->tv_sec = tbi.user_time.seconds + tbi.system_time.seconds;
        tm->tv_nsec = (tbi.user_time.microseconds + tbi.system_time.microseconds) * 1000LL;
    }
    assertion(r == KERN_SUCCESS && (tm->tv_sec != 0 || tm->tv_nsec != 0), "r=%d", r);
}

#endif

#endif
