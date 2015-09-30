#ifndef MEMORY_H
#define MEMORY_H

/* #define DEBUG or MEM to switch to debug version of mem_alloc/mem_free -
   mem_allocated()/mem_allocations() are counted and
   memory_low is set to true if less than RESERVED_MEMORY size remained.
   #define LEAKS to tack memory leaks - call mem_dump_leaks() at check points */

extern void* (*mem_alloc)(size_t size);
extern void (*mem_free)(const void* p);
void* mem_allocz(size_t size); /* allocate and set memory content to zero */
void* mem_dup(void* a, size_t bytes); /* similar to strdup but for known bytes */
size_t mem_size(const void* a); /* number of actual bytes allocated by mem_alloc */

void mem_dump_leaks(); /* report and clear the leasks */
void mem_clear_leaks(); /* clears memory leaks info w/o reporting the leaks */

bool strequ(const char*, const char*);
char* mem_strdup(const char* s); /* same as strdup but counted/leak detected */

int64_t mem_allocated();   /* number of total allocated bytes */
int64_t mem_allocations(); /* number of total allocations */

extern bool memory_low; /* is set to true when the reserved memory started to be used */

/* warning free inline conversions for pointers to long long and back */
#pragma GCC diagnostic push "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
inline static int64_t p2ll(void* p) { return (int64_t)(uint32_t)p; }
#pragma GCC diagnostic pop "-Wpointer-to-int-cast"

#pragma GCC diagnostic push "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
inline static void* ll2p(int64_t ll) { return (void*)ll; }
#pragma GCC diagnostic pop "-Wint-to-pointer-cast"

#endif /* MEMORY_H */
