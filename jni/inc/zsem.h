#ifndef ZSEM_H
#define ZSEM_H
#ifdef __MACH__
#include <mach/semaphore.h>
#endif

/* At the moment of implementation Android does not support named semaphores
   via sem_open() sem_close() and sem_unlink() API.
   This is very simple implementation of process shared semaphores for IPC.
   It is memory expensive (PAGE_SIZE=4K for 12 bytes structure) but it is
   expected that IPC will have very small and limited number of semaphores in use.
   It also works on MACH (APPLE) with some twists.
 */

typedef struct _zsem_t {
    sem_t sem;
#ifdef __MACH__
    sem_t* shared;
    semaphore_t semaphore;
#else
    int fd;
    void* sm;
#endif
} zsem_t;

extern zsem_t* zsem_create(const char* name); /* named shared between processes */
extern int zsem_init(zsem_t* s);
extern int zsem_wait(zsem_t* s);
extern int zsem_timedwait(zsem_t* s, struct timespec* ts);
extern int zsem_post(zsem_t* s);
extern int zsem_destroy(zsem_t* s);

/* worst time seen: sem_post(s)=62  sem_wait(s)=69 microseconds */

#endif /* ZSEM_H */
