#include "manifest.h"
#include "zashmem.h"
#ifdef __MACH__
#include <mach/mach.h>
#include <mach/task.h>
#include <mach/semaphore.h>
#endif

#if defined(__MACH__)

zsem_t* zsem_create(const char* name) {
    zsem_t* s = (zsem_t*)mem_allocz(sizeof(zsem_t));
    if (s != null) {
        sem_unlink(name);
        s->shared = sem_open(name, O_CREAT, 0);
        if (s->shared == null) {
            int e = errno;
            mem_free(s);
            s = null;
            errno = e;
        }
    }
    return s;
}

int zsem_destroy(zsem_t* s) {
    int r = 0;
    if (s != null) {
        if (s->shared != null) {
            r = sem_close(s->shared);
            int e = errno;
            mem_free(s); /* shared semaphore has been mem_alloc-ed */
            if (r != 0) {
                errno = e;
            }
        } else if (s->semaphore != 0) {
            r = semaphore_destroy(mach_task_self(), s->semaphore);
            s->semaphore = 0; /* "s" has been init(&s)-alized no need to mem_free() */
        } else {
            r = sem_destroy(&s->sem);
        }
    }
    return r;
}

int zsem_init(zsem_t* s) {
    memset(s, 0, sizeof(*s));
    kern_return_t r = semaphore_create(mach_task_self(), &s->semaphore, SYNC_POLICY_FIFO, 0);
    return r;
}

int zsem_post(zsem_t* s) {
    return s->shared != null ? sem_post(s->shared) : semaphore_signal(s->semaphore);
}

int zsem_wait(zsem_t* s) {
    return s->shared != null ? sem_wait(s->shared) : semaphore_wait(s->semaphore);
}

int zsem_timedwait(zsem_t* s, struct timespec* ts) {
    if (s->shared != null) {
        errno = ENOSYS;
        return -1;
    }
    mach_timespec_t mt;
    mt.tv_sec = ts->tv_sec;
    mt.tv_nsec = ts->tv_nsec;
    return semaphore_timedwait(s->semaphore, mt);
}

#else

 zsem_t* zsem_create(const char* name) {
    /* sizeof(zsem_t) == 12 */
    assertion(sizeof(zsem_t) <= PAGE_SIZE, "sizeof(zsem_t)=%d PAGE_SIZE=%d", sizeof(zsem_t), PAGE_SIZE);
    zsem_t* s = null;
    int fd = zashmem_create_protected_region(name, PAGE_SIZE, PROT_READ | PROT_WRITE);
    if (fd < 0) {
        return null;
    }
    int r = 0;
    if (r != 0) {
        close(fd);
        return null;
    }
    void* sm = mmap(null, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sm != null) {
        s = (zsem_t*)sm;
        r = sem_init(&s->sem, 1, 0);
    }
    if (sm == null || r != 0) {
        if (sm != null) {
            munmap(sm, PAGE_SIZE);
        }
        close(fd);
        return null;
    }
    s->fd = fd;
    s->sm = sm;
    return s;
}

int zsem_init(zsem_t* s) {
    memset(s, 0, sizeof(*s));
    return sem_init(&s->sem, 0, 0);
}

int zsem_destroy(zsem_t* s) {
    int fd = s->fd;
    void* sm = s->sm;
    int r = sem_destroy(&s->sem);
    posix_ok(r);
    if (sm != null) {
        r = munmap(sm, PAGE_SIZE);
        posix_ok(r);
    }
    if (fd > 0) {
        r = close(fd);
        posix_ok(r);
    }
    memset(s, 0, sizeof(*s));
    return r;
}

int zsem_post(zsem_t* s) {
    return sem_post(&s->sem);
}

int zsem_wait(zsem_t* s) {
    return sem_wait(&s->sem);
}

int zsem_timedwait(zsem_t* s, struct timespec* ts) {
    return sem_timedwait(&s->sem, ts);
}

#endif
