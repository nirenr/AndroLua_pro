#ifndef SOCKETIO_H
#define SOCKETIO_H

/* tries to sets IPPROTO_TCP TCP_NODELAY option to true and returns 0 is successful */
int tcp_nodelay(int s);

/*  receive_fully assumes the incoming message first four bytes is little endian int
    representing the length of the whole message (including the first 4 bytes).
    If successful receive_fully returns 0 and allocates memory to hold the complete
    message returning it's address and size in data_out and bytes_out, otherwise
    returns errno.
    return == 0 and data == null and bytes_out == 0 means graceful socket close.
*/
int receive_fully(int socket, void** data_out, int *bytes_out);

/*  send_fully assumes the outgoing message first four bytes is little endian int
    representing the length of the whole message (including the first 4 bytes).
    If successful receive_fully returns 0 otherwise errno (positive) or -1 on EOF.
*/
int send_fully(int socket, void* data, int bytes);

/* returns file descriptor (fd) upon successful completion or -1.
   errno is valid if return is -1 */
int socket_bound_listen(const char* address_IPv4, int port);

/* returns file descriptor (fd) upon successful completion or -1.
   errno is valid if return is -1 */
int socket_connect(const char* address, int port);

#endif /* SOCKETIO_H */
