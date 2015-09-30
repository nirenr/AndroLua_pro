#include "manifest.h"
#include "looper.h"

typedef struct {
    pthread_t thread;
    queue_t* queue;
    mutex_t mutex;
    zsem_t semaphore;
    void (*start)(void* that);
    void (*dispatch)(void* that, void* data, int bytes);
    void (*stop)(void* that);
    void* that;
    bool quit;  /* set to true (within locked mutex) to request quit */
    bool done;  /* set to true by the looper ((within locked mutex) to respond to quit message, after all data is dispatched */
} looper_t_;

static void* looper_main(void* p);

looper_t* looper_create(void* that,
                        void (*start)(void* that),
                        void (*dispatch)(void* that, void* data, int bytes),
                        void (*stop)(void* that)) {
    assert(dispatch != null);
    looper_t_* looper = mem_allocz(sizeof(looper_t_));
    assert(looper != null);
    if (looper != null) {
        looper->that = that;
        looper->start = start;
        looper->dispatch = dispatch;
        looper->stop = stop;
        looper->quit = false;
        looper->done = false;
        assert(looper->queue == null);
        zsem_init(&looper->semaphore);
        mutex_init(&looper->mutex, null);
        mutex_lock(&looper->mutex);
        pthread_create(&looper->thread, null, looper_main, looper);
        mutex_unlock(&looper->mutex);
    }
    return (looper_t*)looper;
}

void looper_destroy(looper_t* lp) {
    looper_t_* looper = (looper_t_*)lp;
    assert(looper != null);
    if (looper != null) {
        mutex_lock(&looper->mutex);
        looper->quit = true;
        mutex_unlock(&looper->mutex);
        for (;;) {
            mutex_lock(&looper->mutex);
            if (looper->done) {
                break;
            } else {
                zsem_post(&looper->semaphore);
            }
            mutex_unlock(&looper->mutex);
        }
        assertion(looper->done && looper->queue == null, "looper->done %d looper->queue=%p", looper->done, looper->queue);
        pthread_join(looper->thread, null);
        mutex_unlock(&looper->mutex);
        assertion(looper->queue == null, "looper->queue=%p must be null", looper->queue);
        mutex_destroy(&looper->mutex);
        zsem_destroy(&looper->semaphore);
        looper->quit = false;
        looper->done = false;
        looper->thread = 0;
        looper->dispatch = null;
        mem_free(lp);
    }
}

static void* looper_main(void* p) {
    looper_t_* looper = (looper_t_*)p;
    if (looper->start != null) {
        looper->start(looper->that);
    }
    bool quit = false;
    bool done = false;
    while (!done) {
        mutex_lock(&looper->mutex);
        while (looper->queue != null) {
            queue_t* q = looper->queue;
            looper->queue = queue_remove(looper->queue);
            assert(q->data != null);
            mutex_unlock(&looper->mutex);
            looper->dispatch(looper->that, q->data, q->bytes);
            mutex_lock(&looper->mutex);
            if (q->dispose != null) {
                q->dispose(looper->that, q->data, q->bytes);
            }
            if (q->copy) {
                mem_free(q->data);
            }
            mem_free(q);
        }
        quit = looper->quit;
        if (quit) {
            assertion(looper->queue == null, "looper->queue=%p expected null", looper->queue);
            done = looper->done = true;
        }
        mutex_unlock(&looper->mutex);
        if (!quit && !done) {
            zsem_wait(&looper->semaphore);
        }
    }
    if (looper->stop != null) {
        looper->stop(looper->that);
    }
    return null;
}

static void _looper_post(looper_t_* looper, void* data, int bytes, bool copy, void (*dispose)(void* that, void*, int)) {
//  timestamp("_looper_post");
    assert(data != null && bytes > 0);
    mutex_lock(&looper->mutex);
    assertion(!looper->quit && !looper->done, "all posting threads must be joined before stop_looper() call");
    queue_t* e = (queue_t*)mem_allocz(sizeof(queue_t));
    e->data = data;
    e->bytes = bytes;
    e->copy = copy;
    e->dispose = dispose;
    looper->queue = queue_append(looper->queue, e);
    zsem_post(&looper->semaphore);
    mutex_unlock(&looper->mutex);
//  timestamp("_looper_post");
}

void looper_post(looper_t* lp, void* data, int bytes, bool copy) {
    looper_t_* looper = (looper_t_*)lp;
    _looper_post(looper, data, bytes, copy, null);
}

void looper_post_dup(looper_t* lp, void* data, int bytes) {
    looper_t_* looper = (looper_t_*)lp;
    void* dup = mem_dup(data, bytes);
    _looper_post(looper, dup, bytes, true, null);
}

void looper_post_disposable(looper_t* lp, void* data, int bytes, void (*dispose)(void* that, void*, int)) {
    looper_t_* looper = (looper_t_*)lp;
    _looper_post(looper, data, bytes, false, dispose);
}


