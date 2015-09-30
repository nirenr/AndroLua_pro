#include "manifest.h"
#include "socketio.h"
#include <netinet/tcp.h>

#ifdef __MACH__
#define MSG_NOSIGNAL 0
#endif

int tcp_nodelay(int s) {
    assertion(s > 0, "expected open socket s=%d", s);
    int set = 1;
    int r = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &set, sizeof(int));
    posix_ok(r);
    return r;
}

static int nosigpipe(int s) {
#ifdef __MACH__
    if (s > 0) {
        int set = 1;
        int r = setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(int));
        posix_ok(r);
    }
#endif
    return s;
}

int receive_fully(int s, void** data_out, int *bytes_out) {
    assertion(s > 0, "s=%d", s);
    int e = 0;
    int received = 0;
    bool eof = false;
    unsigned char len_bytes_buffer[4] = {0};
    unsigned char* buf = len_bytes_buffer;
    int len = 4;
    while (len > 0) {
        received = recv(s, buf, len, 0);
        if (received < 0) {
            e = errno;
            break; /* socket error */
        } else if (received == 0) {
            eof = true;
            break;
        }
        assertion(received > 0, "socket=%d received=%d", s, received);
        buf += received;
        len -= received;
    }
    if (eof) {
        *data_out = null;
        *bytes_out = 0;
        return e;
    }
    if (e != 0) {
        return e;
    }
    int bytes = len_bytes_buffer[0] | (len_bytes_buffer[1] << 8) |
        (len_bytes_buffer[2] << 16) | (len_bytes_buffer[3] << 24);
    assertion(bytes > 0, "socket=%d bytes=%d", s, bytes);
    void* data = mem_allocz(bytes);
    assert(data != null);
    if (data != null) {
        len = bytes;
        buf = ((unsigned char*)data);
        memcpy(buf, &bytes, sizeof(bytes));
        buf += sizeof(bytes);
        len -= sizeof(bytes);
        while (len > 0) {
            received = recv(s, buf, len, 0);
            if (received < 0) {
                e = errno;
                break; /* socket closed */
            } else if (received == 0) {
                eof = true;
                break;
            }
            assertion(0 < received && received <= len,
                      "socket=%d received=%d len=%d bytes=%d",
                      s, received, len, bytes);
            buf += received;
            len -= received;
        }
        if (eof) {
            *data_out = null;
            *bytes_out = 0;
        }
        if (e != 0 || eof) {
            mem_free(data);
            return e;
        }
        assertion(len == 0, "socket=%d bytes=%d len=%d", s, bytes, len);
        int in = ((int*)data)[0];  /* this assumes we are on little-endian host */
        assertion(bytes == in, "expected %d in(header)=%d", bytes, in);
        *bytes_out = bytes;
    }
    *data_out = data;
    return e;
}

int send_fully(int s, void* data, int bytes) {
    assertion(s > 0, "s=%d", s);
    assertion(data != null && bytes > 0, "data=%p bytes=%d", data, bytes);
    unsigned char* p = (unsigned char*)data;
    int len = ((int*)data)[0];  /* this assumes we are on little-endian host */
    assertion(len == bytes, "len=%d bytes=%d", len, bytes);
    bool eof = false;
    int e = 0;
    while (len > 0 && !eof) {
        int sent = send(s, p, len, MSG_NOSIGNAL);
        if (sent == 0) {
            eof = true;
        } else if (sent == -1) {
            int e = errno;
            assertion(e != 0, "socket=%d sent=%d len=%d bytes=%d e=%d", s, sent, len, bytes, e);
            break;
        } else {
            assertion(0 < sent && sent <= len, "sent=%d len=%d bytes=%d", s, sent, len, bytes);
            p += sent;
            len -= sent;
        }
    }
    assertion(eof || len == 0 && e != 0 || len == 0 && e == 0, "socket=%d len=%d errno=%d", s, len, e);
    return e != 0 ? e : (eof ? -1 : 0);
}

int socket_bound_listen(const char* ipa, int port) {
    struct sockaddr_in ain = {0};
    ain.sin_family = AF_INET;
    /* sin_addr.s_addr and sin_port have to be in network byte order according to
       http://www.freebsd.org/doc/en/books/developers-handbook/sockets-essential-functions.html */
    ain.sin_addr.s_addr = ipa == null ? htonl(INADDR_ANY) : inet_addr(ipa);
    ain.sin_port = htons(port);
    int e = 0;
    int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0) {
        return s;
    }
    int reuse = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        e = errno;
    } else if (bind(s, (struct sockaddr*)&ain, sizeof(ain)) < 0 ) {
        e = errno;
    } else if (listen(s, SOMAXCONN) != 0) {
        e = errno;
    }
    if (e != 0) {
        close(s);
        s = -1;
        errno = e;
    }
    return nosigpipe(s);
}

int socket_connect(const char *address, int port) {
    struct sockaddr_in ain = {0};
    ain.sin_family = AF_INET;
    struct hostent* host = gethostbyname(address);
    memcpy(&(ain.sin_addr.s_addr), host->h_addr, host->h_length);
    ain.sin_port = htons(port);
    int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int r = connect(s, (struct sockaddr*)&ain, sizeof(ain));
    return r < 0 ? -1 : nosigpipe(s);
}
