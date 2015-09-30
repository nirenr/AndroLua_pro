#ifndef MAPLL_H
#define MAPLL_H

/* only positive keys are allowed (k > 0).
   Zero returned as a value for the absent key.
   Thus _put(any_key, 0) is prohibited. */

typedef void* mapll_t;

mapll_t mapll_create(int capacity); /* put & get time: ~517 nanoseconds */
mapll_t mapll_create_fixed(int capacity); /* to avoid any calls to malloc */
int64_t mapll_get(mapll_t m, int64_t k);
bool mapll_put(mapll_t m, int64_t k, int64_t v);
int64_t mapll_remove(mapll_t map, int64_t key);
int mapll_size(mapll_t m);
void mapll_foreach(void* that, mapll_t map, void (*each)(void* that, mapll_t, int64_t)); /* it is OK to call remove for current key from each() */
void mapll_clear(mapll_t map); /* remove all entries */
void mapll_dispose(mapll_t m);

#endif /* MAPLL_H */
