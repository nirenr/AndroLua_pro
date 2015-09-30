#include "manifest.h"
#include "zslib.h"
#include "revision.h"

void free_elem(void* that, mapll_t map, int64_t key) {
    mem_free(ll2p(key));
}

static void memory_low_smoke_test() {
    trace(">MEMORY"); // ~ 1296MB - 1304MB
    mapll_t* map = mapll_create(2048);
    enum { CHUNK = 64 * 1024 };
    int i = 0;
    for (i = 0; i < 2048 * 1024; i++) {
        unsigned char* b = (unsigned char*)mem_alloc(CHUNK);
        if (b == null) {
            trace("mem_alloc == null");
            break;
        } else if (memory_low) {
            trace("memory_low");
            break;
        } else {
            mapll_put(map, p2ll(b) & 0xFFFFFFFFLL, 1);
        }
        b[2047] = (unsigned char)i;
        if (i % 1024 == 0) {
            trace(" MEMORY %dMB", i * CHUNK / (1024*1024));
        }
    }
    trace(" MEMORY %dMB", i * CHUNK / (1024*1024));
    mapll_foreach(null, map, free_elem);
    mapll_dispose(map);
    trace("<MEMORY");
}

extern int backtrace_smoke_test();
extern void sigaction_smoke_test();

int smoke_tests(const char* s) {
    if (false) {
        trace("zSpace svn revision %d: %s", ZSPACE_REVISION, s);
        sigaction_smoke_test();
        memory_low_smoke_test();
        backtrace_smoke_test();
    }
    return 0;
}
