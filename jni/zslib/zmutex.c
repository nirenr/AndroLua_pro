#include "manifest.h"
#include "zashmem.h"

typedef struct {
    pthread_mutex_t mx;
    int fd;
    void* sm;
} zmutex_t;

pthread_mutex_t* zmutex_create(const char* name) {
    /* sizeof(zmutex_t) == 12 */
    assertion(sizeof(zmutex_t) <= PAGE_SIZE, "sizeof(zmutex_t)=%d PAGE_SIZE=%d", sizeof(zmutex_t), PAGE_SIZE);
    pthread_mutex_t* m = null;
    int fd = zashmem_create_protected_region(name, PAGE_SIZE, PROT_READ | PROT_WRITE);
    if (fd < 0) {
        return null;
    }
    int r = 0;
    void* sm = mmap(null, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sm != null) {
        pthread_mutexattr_t ma;
        pthread_mutexattr_init(&ma);
        r = pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
        if (r == 0) {
            m = (pthread_mutex_t*)sm;
            r = pthread_mutex_init(m, &ma);
        }
    }
    if (sm == null || r != 0) {
        if (sm != null) {
            munmap(sm, PAGE_SIZE);
        }
        close(fd);
        return null;
    }
    zmutex_t* zmx = (zmutex_t*)m;
    zmx->fd = fd;
    zmx->sm = sm;
    return m;
}

void zmutex_destroy(pthread_mutex_t* m) {
    zmutex_t* zmx = (zmutex_t*)m;
    int fd = zmx->fd;
    void* sm = zmx->sm;
    int r = pthread_mutex_destroy(m);
    assertion(r == 0, "pthread_mutex_destroy=%d", r);
    r = munmap(sm, PAGE_SIZE);
    assertion(r == 0, "munmap(%p)=%d", sm, r);
    r = close(fd);
    assertion(r == 0, "close(fd=%d)=%d", fd, r);
}
