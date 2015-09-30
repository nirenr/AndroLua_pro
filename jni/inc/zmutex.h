#ifndef ZMUTEX_H
#define ZMUTEX_H

/* This is very simple implementation of process shared mutexes for IPC.
   It is memory expensive (PAGE_SIZE=4K for 12 bytes structure) but it is
   expected that IPC will have very small and limited number of mutexes in use.
 */

typedef pthread_mutex_t mutex_t;

#define mutex_init pthread_mutex_init
#define mutex_lock pthread_mutex_lock
#define mutex_unlock pthread_mutex_unlock
#define mutex_destroy pthread_mutex_destroy

extern mutex_t* zmutex_create(const char* name);
extern void zmutex_destroy(mutex_t* m);

/* worst time seen for inter-process contested mutex: mutex_lock(m)=76 mutex_unlock(m)=115 microseconds */

#endif /* ZMUTEX_H */
