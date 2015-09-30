#include "manifest.h"

typedef struct {
    looper_t* looper;
    pthread_t thread;
    void* that;
    int socket;
    int err;
    receiver_error_callback_t error;
    bool quit;  /* set to true (within locked mutex) to request quit */
    bool done;  /* set to true by the receiver ((within locked mutex) to respond to quit message, after all data is dispatched */
} receiver_t_;

static void* receiver_main(void* p);

receiver_t* receiver_create(void* that, int socket, looper_t* looper, receiver_error_callback_t error) {
    assert(socket > 0 && looper != null);
    receiver_t_* receiver = mem_allocz(sizeof(receiver_t_));
    assert(receiver != null);
    if (receiver != null) {
        receiver->error = error;
        receiver->that = that;
        receiver->socket = socket;
        receiver->looper = looper;
        receiver->quit = false;
        receiver->done = false;
        pthread_create(&receiver->thread, null, receiver_main, receiver);
    }
    return (receiver_t*)receiver;
}

void receiver_destroy(receiver_t* rc) {
    receiver_t_* receiver = (receiver_t_*)rc;
    assert(receiver != null);
    if (receiver != null) {
        int sc = receiver->socket;
        receiver->socket = -1;
        if (sc > 0) {
            int r = shutdown(sc, SHUT_RDWR);
            posix_ok(r);
            r = close(sc);
            posix_ok(r);
        }
        pthread_join(receiver->thread, null);
        receiver->thread = 0;
        mem_free(rc);
    }
}

static void dispose_message(void* that, void* data, int bytes) {
    received_message_t* message = (received_message_t*)data;
    assert(bytes == sizeof(received_message_t));
    assert(message != null);
    assertion(message->data != null && message->bytes > 0, "data=%p bytes=%d", message->data, message->bytes);
    mem_free(message->data);
    mem_free(message);
}

static void* receiver_main(void* p) {
    receiver_t_* receiver = (receiver_t_*)p;
    bool done = false;
    while (!done) {
        void* data = null;
        int bytes = 0;
        int e = receiver->socket > 0 ? receive_fully(receiver->socket, &data, &bytes) : 0;
        if (e != 0 && receiver->socket > 0) {
            receiver->err = e;
        }
        bool eof = e == 0 && data == null && bytes == 0;
        done = e != 0 || eof;
        if (!done) {
            received_message_t message = { .socket = receiver->socket, .data = data, .bytes = bytes };
            looper_post_disposable(receiver->looper, mem_dup(&message, sizeof(message)), sizeof(message), dispose_message);
        }
    }
    if (receiver->socket > 0) {
        int sc = receiver->socket;
        receiver->socket = -1;
        int r = close(sc);
        posix_ok(r);
    }
    if (receiver->err != 0 && receiver->error != null) {
        receiver->error(receiver->that, (receiver_t)receiver, receiver->err);
    }
    return null;
}
