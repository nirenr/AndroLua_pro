#include "manifest.h"
#ifdef __MACH__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

bool strequ(const char* s1, const char* s2) {
    return s1 == null || s2 == null ? s1 == s2 : (s1 == s2 || strcmp(s1, s2) == 0);
}

enum { RESERVED_MEMORY_SIZE = 1 * 1024 * 1024 }; /* 1 MB */
static void* reserved;
bool memory_low;

static volatile int64_t total_allocations;
static volatile int64_t total_allocated;

static mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static bool trace_allocs = false;
static mapll_t leaks;

/* _stacktrace returns global static buffer because it is called from mem_alloc */
extern char* stacktrace_(bool detailed, siginfo_t *sig_info, void* sig_context, int from);

void* (*mem_alloc)(size_t size);
void  (*mem_free)(const void* p);

size_t mem_size(const void* a) {
#ifdef __MACH__
    return malloc_size(a);
#elif __ANDROID__
    size_t* s = &((size_t*)a)[-1];
    int allocated = (*s + 3) & ~0x3; /* malloc_usable_size(r); */
    return allocated;
#else
    return malloc_usable_size(a);
#endif
}

void* mem_alloc_(size_t size) {
    mutex_lock(&mutex);
    void* a = malloc(size);
    if (a == null && reserved != null) {
        trace("MEMORY LOW: allocations=%lld allocated %lld", total_allocations, total_allocated);
        memory_low = true;
        free(reserved);
        reserved = null;
        a = malloc(size);
    }
    if (a != null) {
        int allocated = mem_size(a);
        const char* st = null;
        if (trace_allocs)  {
            st = stacktrace_(false, null, null, 3);
            trace("%p allocated=%d size=%d total: allocations=%lld allocated %lld %s",
                   a, allocated, size, total_allocations, total_allocated, st);
        }
        total_allocations++;
        total_allocated += allocated;
        if (leaks != null) { /* IMPORTANT: intentionally not mem_strdup */
            st = st != null ? st : stacktrace_(false, null, null, 3);
            mapll_put(leaks, p2ll((void*)a), p2ll((void*)strdup(st)));
        }
    }
    mutex_unlock(&mutex);
    return a;
}

void* mem_allocz(size_t size) {
    void* a = mem_alloc(size);
    if (a != null) {
        memset(a, 0, size);
    }
    return a;
}

void mem_free_(const void* a) {
    if (a != null) {
        size_t allocated = mem_size(a);
        if (trace_allocs) {
            trace("%p allocated=%d total: allocations=%lld allocated %lld", a, allocated, total_allocations, total_allocated);
        }
        mutex_lock(&mutex);
        total_allocations--;
        total_allocated -= allocated;
        if (leaks != null) {
            mapll_remove(leaks, p2ll((void*)a));
        }
        mutex_unlock(&mutex);
        free((void*)a);
    }
}

static void mem_dump_each_leak(void* that, mapll_t m, int64_t k) {
    char* st = (char*)ll2p(mapll_remove(leaks, k));
    void* p = ll2p(k);
    size_t bytes = mem_size(p);
    trace("leak %p allocated %d at: %s", p, bytes, st != null ? st : "???");
    free(st); /* IMPORTANT: not mem_free see: mem_alloc code */
}

void mem_dump_leaks() {
    if (leaks != null) {
        mutex_lock(&mutex);
        int size = mapll_size(leaks);
        mapll_foreach(null, leaks, mem_dump_each_leak);
        trace("detected %d leaks", size);
        total_allocations = 0;
        total_allocated = 0;
        mutex_unlock(&mutex);
    }
}

static void mem_clear_each_leak(void* that, mapll_t m, int64_t k) {
    char* st = (char*)ll2p(mapll_remove(leaks, k));
    free(st); /* IMPORTANT: not mem_free see: mem_alloc code */
}

void mem_clear_leaks() {
    if (leaks != null) {
        mutex_lock(&mutex);
        mapll_foreach(null, leaks, mem_clear_each_leak);
        mapll_clear(leaks);
        total_allocations = 0;
        total_allocated = 0;
        mutex_unlock(&mutex);
    }
}

char* mem_strdup(const char* s) {
    assertion(s != null, "cannot be null s=%p", s);
    char* r = null;
    if (s != null) {
        r = mem_alloc(strlen(s) + 1);
        assert(r != null);
        if (r != null) {
            strcpy(r, s);
        }
    }
    return r;
}

void* mem_dup(void* a, size_t bytes) {
    assertion(a != null && bytes > 0, "a=%p bytes=%d", a, bytes);
    void* r = null;
    if (a != null && bytes > 0) {
        r = mem_alloc(bytes);
        assert(r != null);
        if (r != null) {
            memcpy(r, a, bytes);
        }
    }
    return r;
}

int64_t mem_allocated() {
    return total_allocated;
}

int64_t mem_allocations() {
    return total_allocations;
}

__attribute__((constructor)) /* alternatively LOCAL_LDFLAGS=-Wl,-init,foo -fini,bar */
void mem_init(void) {
    /* init/fini is called on dlclose on the loading thread.
       IMPORTANT: it is assumed that only on copy of zslib per process and it this
       loaded on main thread to avoid possible mem_init racing conditions.
       Just in case this condition does not hold (bad idea) we lock the global mutex on init;
       "leaks" table is fixed size and won't grow. Always allocated with malloc */
    typedef void (*mem_free_t)(const void* p);
    mutex_lock(&mutex);
    if (reserved == null) {
#if defined(MEM) && defined(LEAKS)
//      #pragma message "#defined MEM && LEAKS"
        if (leaks == null) {
            mem_alloc = malloc;
            mem_free = (mem_free_t)free;
            leaks = mapll_create_fixed(1024*1024); /* map will be allocated with malloc */
        }
#endif
        reserved = malloc(RESERVED_MEMORY_SIZE);
        unsigned char* p = (unsigned char*)reserved;
        for (int i = 0; i < RESERVED_MEMORY_SIZE; i += 1024) {
            p[i] = (unsigned char)i; // commit pages
        }
#if defined(DEBUG) || defined(MEM) || defined(LEAKS)
//      #pragma message "#defined DEBUG || MEM || LEAKS"
        mem_alloc = mem_alloc_;
        mem_free = (mem_free_t)mem_free_;
#else
        mem_alloc = malloc;
        mem_free = (mem_free_t)free;
#endif
        total_allocations = 0;
        total_allocated = 0;
    }
    mutex_unlock(&mutex);
}

__attribute__((destructor))
void mem_fini(void) {
    /* intentionally empty */
}
