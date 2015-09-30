#include "manifest.h"

#undef trace /* comment these 2 lines out to enable trace */
#define trace(...)

static const char* REMOVED = "removed";

typedef struct {
    int capacity;
    int size;
    int* hashes;
    int64_t* values;
    const char** keys;
} _mapsl_t;

static bool mapsl_put_hashed(mapsl_t m, const char* k, int64_t v, int hash, bool dup);

mapsl_t mapsl_create(int capacity) {
    trace("mapsl_create pid=%d tid=%d", getpid(), gettid());
    capacity = capacity < 16 ? 16 : capacity;
    _mapsl_t* map = mem_allocz(sizeof(_mapsl_t));
    if (map != null) {
        map->keys = (const char**)mem_allocz(capacity * sizeof(char*));
        map->values = (int64_t*)mem_allocz(capacity * sizeof(int64_t));
        map->hashes = (int*)mem_allocz(capacity * sizeof(int));
        map->capacity = capacity;
        if (map->keys == null || map->values == null || map->hashes == null) {
            mapsl_dispose(map);
            map = null;
        }
    }
    return map;
}

int mapsl_size(mapsl_t m) {
    _mapsl_t* map = (_mapsl_t*)m;
    return map->size;
}

static void mapsl_dispose_innards(mapsl_t m) {
    _mapsl_t* map = (_mapsl_t*)m;
    if (map != null) {
        if (map->keys != null) {
            mem_free(map->keys);
            map->keys = null;
        }
        if (map->values != null) {
            mem_free(map->values);
            map->values = null;
        }
        if (map->hashes != null) {
            mem_free(map->hashes);
            map->hashes = null;
        }
    }
}

void mapsl_foreach(mapsl_t m, void (*each)(mapsl_t map, const char* key, void* rt), void* rt) {
    _mapsl_t* map = (_mapsl_t*)m;
    for (int i = 0; i < map->capacity; i++) {
        if (map->keys[i] != null && map->keys[i] != REMOVED) {
            each(m, map->keys[i], rt);
        }
    }
}

void mapsl_clear(mapsl_t m) {
    _mapsl_t* map = (_mapsl_t*)m;
    assert(map != null);
    if (map != null) {
        if (map->keys != null) {
            for (int i = 0; i < map->capacity; i++) {
                if (map->keys[i] != null && map->keys[i] != REMOVED) {
                    mem_free((void*)map->keys[i]);
                }
                map->keys[i] = null;
                map->values[i] = 0;
                map->hashes[i] = 0;
            }
        }
        map->size = 0;
    }
}

void mapsl_dispose(mapsl_t m) {
    _mapsl_t* map = (_mapsl_t*)m;
    if (map != null) {
        mapsl_clear(m);
        mapsl_dispose_innards(m);
        mem_free(map);
    }
}

static int hash0(const char* k) { /* assumes k != null && k[0] != null */
    int h = *k++;
    while (*k) {
        h = h * 13 + *k++;
    }
    return (h & 0x7FFFFFFF);
}

static bool mapsl_grow(mapsl_t m);

bool mapsl_put(mapsl_t m, const char* k, int64_t v) {
    trace("mapsl_put[\"%s\",%lld]", k, v);
    assertion(v != 0 && k != null && k[0] != 0, "k=%s v=%lld", k == null ? "null" : k, v);
    if (v == 0 || k == null || k[0] == 0) {
        trace("mapsl_put[%s,%lld] = nulls not allowed", k == null ? "null" : k, v);
        return false;
    } else {
        int hash = hash0(k);
        bool b = mapsl_put_hashed(m, k, v, hash, true);
        b = b || (mapsl_grow(m) && mapsl_put_hashed(m, k, v, hash, true));
        return b;
    }
}

static bool mapsl_put_hashed(mapsl_t m, const char* k, int64_t v, int hash, bool dup) {
    trace("mapsl_put_hashed[\"%s\",%lld]", k, v);
    assertion(v != 0 && k != null && k[0] != 0, "k=%s v=%lld", k == null ? "null" : k, v);
    _mapsl_t* map = (_mapsl_t*)m;
    int h = hash % map->capacity;
    int h0 = h;
    for (;;) {
        if (map->keys[h] == null || strequ(map->keys[h], k) || map->keys[h] == REMOVED) {
            break;
        }
        h = (h + 1) % map->capacity;
        if (h == h0) {
            trace("mapsl_put_hashed[%s,%lld] false; need to grow", k, v);
            return false; /* need to grow */
        }
    }
    if (map->keys[h] == null || map->keys[h] == REMOVED) {
        const char* s = dup ? mem_strdup(k) : k;
        if (s == null) {
            assertion(s != null, "out of memory");
            return false;
        }
        map->keys[h] = s;
        map->hashes[h] = hash;
        map->size++;
        trace("--------------");
        trace("size=%d", map->size);
        trace("h=%d", h);
        trace("map->keys[%d]=0x%08X", h, map->keys[h]);
        trace("map->keys[%d]=\"%s\"", h, map->keys[h] == null ? "null" : map->keys[h]);
        trace("++++++++++++++");
    }
    map->values[h] = v;
    trace("mapsl_put_hashed[%s=%s,%lld] h=%d", k, map->keys[h], v, h);
    return true;
}

int64_t mapsl_get(mapsl_t m, const char* k) {
    _mapsl_t* map = (_mapsl_t*)m;
    int hash = hash0(k);
    int h = hash % map->capacity;
    int h0 = h;
    for (;;) {
        if (map->keys[h] == null) {
            return 0;
        }
        if (strequ(map->keys[h], k)) {
            return map->values[h];
        }
        h = (h + 1) % map->capacity;
        if (h == h0) {
            return 0;
        }
    }
}

int64_t mapsl_remove(mapsl_t m, const char* k) {
    assert(k != null);
    _mapsl_t* map = (_mapsl_t*)m;
    int hash = hash0(k);
    int h = hash % map->capacity;
    int h0 = h;
    trace("map->keys=0x%08X", map->keys);
    trace("map->values=0x%08X", map->values);
    trace("map->hashes=0x%08X", map->hashes);
    trace("map->capacity=%d", map->capacity);
    for (;;) {
        trace("h=%d", h);
        trace("map->keys[%d]=0x%08X", h, map->keys[h]);
        trace("map->keys[%d]=\"%s\"", h, map->keys[h] == null ? "null" : map->keys[h]);
        if (map->keys[h] == null) {
            trace("");
            return 0;
        }
        trace("map->keys[%d]=%s", h, map->keys[h]);
        if (strequ(map->keys[h], k)) {
            mem_free((void*)map->keys[h]);
            map->keys[h] = (char*)REMOVED;
            map->hashes[h] = 0;
            map->size--;
            return map->values[h];
        }
        h = (h + 1) % map->capacity;
        if (h == h0) {
            trace("");
            return 0;
        }
    }
    trace("");
}

static bool mapsl_grow(mapsl_t m) {
    trace("");
    _mapsl_t* m1 = (_mapsl_t*)m;
    int c = m1->capacity;
    _mapsl_t* m2 = (_mapsl_t*)mapsl_create(c * 3 / 2);
    if (m2 != null) {
        assertion(m2 != null, "out of memory");
        int i = 0;
        while (i < c) {
            if (m1->keys[i] != null && m1->keys[i] != REMOVED) {
                trace(">mapsl_put_hashed m2->size=%d", m2->size);
                bool b = mapsl_put_hashed(m2, m1->keys[i], m1->values[i], m1->hashes[i], false);
                trace("<mapsl_put_hashed m2->size=%d", m2->size);
                if (!b) {
                    trace("");
                    mapsl_dispose(m2);
                    return false;
                }
                m1->keys[i] = null;
            }
            i++;
        }
    }
    assertion(m1->size == m2->size, "m1->size=%d m2->size=%d", m1->size, m2->size);
    mapsl_dispose_innards(m1);
    m1->keys = m2->keys;
    m1->values = m2->values;
    m1->hashes = m2->hashes;
    trace("%d to %d", m1->capacity, m2->capacity);
    m1->capacity = m2->capacity;
    m2->keys = null;
    m2->values = null;
    m2->hashes = null;
    mapsl_dispose(m2);
    return true;
}

