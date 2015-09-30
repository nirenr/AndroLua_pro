#include "manifest.h"

static int64_t REMOVED = -1;

typedef struct {
    int capacity;
    int size;
    int* hashes;
    int64_t* values;
    int64_t* keys;
    bool can_grow;
} _mapll_t;

static bool mapll_put_hashed(mapll_t m, int64_t k, int64_t v, int hash);
static void mapll_dispose_innards(mapll_t m);

static mapll_t mapll_create_(int capacity, bool can_grow) {
    capacity = capacity < 16 ? 16 : capacity;
    _mapll_t* map = mem_allocz(sizeof(_mapll_t));
    if (map != null) {
        map->keys = (int64_t*)mem_allocz(capacity * sizeof(int64_t));
        map->values = (int64_t*)mem_allocz(capacity * sizeof(int64_t));
        map->hashes = (int*)mem_allocz(capacity * sizeof(int));
        map->capacity = capacity;
        map->can_grow = can_grow;
        if (map->keys == null || map->values == null || map->hashes == null) {
            mapll_dispose(map);
            map = null;
        }
    }
    return map;
}

mapll_t mapll_create_fixed(int capacity) {
    return mapll_create_(capacity, false);
}

mapll_t mapll_create(int capacity) {
    return mapll_create_(capacity, true);
}

int mapll_size(mapll_t m) {
    _mapll_t* map = (_mapll_t*)m;
    return map->size;
}

static int hash0(int64_t k) {
    unsigned int h = (unsigned int)k ^ (unsigned int)(((uint64_t)k) >> 32);
    /* Doug Lea's supplemental secondaryHash function */
    h ^= (h >> 20) ^ (h >> 12);
    h ^= (h >> 7) ^ (h >> 4);
    return (int)(h & 0x7FFFFFFF);
}

static bool mapll_grow(mapll_t m) {
    _mapll_t* m1 = (_mapll_t*)m;
    int c = m1->capacity;
    _mapll_t* m2 = (_mapll_t*)mapll_create(c * 3 / 2);
    if (m2 != null) {
        assertion(m2 != null, "out of memory");
        int i = 0;
        while (i < c) {
            if (m1->keys[i] != 0 && m1->keys[i] != REMOVED) {
                bool b = mapll_put_hashed(m2, m1->keys[i], m1->values[i], m1->hashes[i]);
                if (!b) {
                    mapll_dispose(m2);
                    return false;
                }
                m1->keys[i] = 0;
            }
            i++;
        }
    }
    assert(m2->size == m1->size);
    mapll_dispose_innards(m1);
    m1->keys = m2->keys;
    m1->values = m2->values;
    m1->hashes = m2->hashes;
    m1->capacity = m2->capacity;
    m2->keys = null;
    m2->values = null;
    m2->hashes = null;
    mapll_dispose(m2);
    return true;
}

bool mapll_put(mapll_t m, int64_t k, int64_t v) {
    assertion(v != 0 && k > 0, "expected k > 0 k=%lld v=%lld", k, v);
    if (v == 0 || k == 0) {
        return false;
    } else {
        _mapll_t* map = (_mapll_t*)m;
        int hash = hash0(k);
        bool b = mapll_put_hashed(m, k, v, hash);
        b = b || map->can_grow && (mapll_grow(m) && mapll_put_hashed(m, k, v, hash));
        return b;
    }
}

static bool mapll_put_hashed(mapll_t m, int64_t k, int64_t v, int hash) {
    assertion(k > 0, "only keys > 0 are supported k=%lld v=%lld", k, v);
    _mapll_t* map = (_mapll_t*)m;
    int h = hash % map->capacity;
    int h0 = h;
    for (;;) {
        if (map->keys[h] == 0 || map->keys[h] == k || map->keys[h] == REMOVED) {
            break;
        }
        h = (h + 1) % map->capacity;
        if (h == h0) {
            return false; /* need to grow */
        }
    }
    if (map->keys[h] == 0 || map->keys[h] == REMOVED) {
        map->keys[h] = k;
        map->hashes[h] = hash;
        map->size++;
    }
    map->values[h] = v;
    return true;
}

int64_t mapll_get(mapll_t m, int64_t k) {
    _mapll_t* map = (_mapll_t*)m;
    int hash = hash0(k);
    int h = hash % map->capacity;
    int h0 = h;
    for (;;) {
        if (map->keys[h] == 0) {
            return 0;
        }
        if (map->keys[h] == k) {
            return map->values[h];
        }
        h = (h + 1) % map->capacity;
        if (h == h0) {
            return 0;
        }
    }
}

int64_t mapll_remove(mapll_t m, int64_t k) {
    assertion(k > 0, "k=%d 0x%016llX", k, k);
    _mapll_t* map = (_mapll_t*)m;
    int hash = hash0(k);
    int h = hash % map->capacity;
    int h0 = h;
    for (;;) {
        if (map->keys[h] == 0) {
            return 0;
        }
        if (map->keys[h] == k) {
            map->keys[h] = REMOVED;
            map->hashes[h] = 0;
            map->size--;
            assertion(map->values[h] > 0, "map->values[%d]=%lld 0x%016llX", h, map->values[h]);
            return map->values[h];
        }
        h = (h + 1) % map->capacity;
        if (h == h0) {
            return 0;
        }
    }
}

void mapll_foreach(void* that, mapll_t m, void (*each)(void* that, mapll_t, int64_t)) {
    _mapll_t* map = (_mapll_t*)m;
    for (int i = 0; i < map->capacity; i++) {
        if (map->keys[i] != 0 && map->keys[i] != REMOVED) {
            assertion(map->keys[i] > 0, "mapll=%p[%d] = (%lld=0x%016llX)", m, i, map->keys[i], map->keys[i]);
            each(that, m, map->keys[i]);
        }
    }
}

void mapll_clear(mapll_t m) {
    _mapll_t* map = (_mapll_t*)m;
    assert(map != null);
    if (map != null) {
        if (map->keys != null) {
            for (int i = 0; i < map->capacity; i++) {
                map->keys[i] = 0;
                map->values[i] = 0;
                map->hashes[i] = 0;
            }
        }
        map->size = 0;
    }
}

static void mapll_dispose_innards(mapll_t m) {
    _mapll_t* map = (_mapll_t*)m;
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

void mapll_dispose(mapll_t m) {
    _mapll_t* map = (_mapll_t*)m;
    if (map != null) {
        mapll_dispose_innards(m);
        mem_free(map);
    }
}


