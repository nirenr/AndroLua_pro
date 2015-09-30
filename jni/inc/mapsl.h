#ifndef MAPSL_H
#define MAPSL_H

/* Only not null and not empty strings are allowed as keys.
   Zero returned as a value for the absent key.
   Thus _put(any_key, 0) is prohibited too. */
typedef void* mapsl_t;

mapsl_t mapsl_create(int capacity);
int64_t mapsl_get(mapsl_t m, const char* k); /* put & get time: ~2725 nanoseconds */
bool mapsl_put(mapsl_t m, const char* k, int64_t v);
int64_t mapsl_remove(mapsl_t map, const char* key);
int mapsl_size(mapsl_t m);
void mapsl_foreach(mapsl_t m, void (*each)(mapsl_t, const char* key, void* rt), void* rt); /* it is OK to call remove for current key from each() */
void mapsl_clear(mapsl_t map); /* remove all entries */
void mapsl_dispose(mapsl_t m);


#endif /* MAPSL_H */
