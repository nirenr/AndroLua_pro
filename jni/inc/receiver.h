#ifndef RECEIVER_H
#define RECEIVER_H
#include "looper.h"

typedef void* receiver_t;

typedef struct {
    int socket;
    void* data;
    int bytes;
} received_message_t;

typedef void (*receiver_error_callback_t)(void* that, receiver_t* r, int err);

/*  receiver_create creates receiver thread that will receive messages via recv(socket)
    and post each received message into the looper as received_message_t struct.
    receiver works with the following assumptions:
    1. only one receiver per socket
    2. each message starts with 4 bytes representing complete message length in bytes
       ("int bytes") including this four in little endian (not network) order (legacy)
    When socket is closed gracefully receiver thread terminates.
    If receive fails at any moment "error" callback is called before receiving thread terminates.
*/
receiver_t* receiver_create(void* that, int socket, looper_t* looper, receiver_error_callback_t error);

/* receiver_destroy closes(!) the socket and joins receiver thread */

void receiver_destroy(receiver_t* receiver);

#endif /* RECEIVER_H */
