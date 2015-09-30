#include "manifest.h"
#ifdef __ANDROID__
#include <linux/ashmem.h>
#elif __MACH__
#include <sys/shm.h>
#include <sys/mman.h>
#endif

#define ASHMEM_DEVICE	"/dev/ashmem"

#ifdef __ANDROID__

int zashmem_set_prot_region(int fd, int prot) {
    return ioctl(fd, ASHMEM_SET_PROT_MASK, prot);
}

int zashmem_create_protected_region(const char* name, size_t size, int prot) {
    int fd = open(ASHMEM_DEVICE, O_RDWR);
    if (fd < 0) {
	return fd;
    }
    if (name != null) {
        assertion(strlen(name) < ASHMEM_NAME_LEN, "strlen(%s)=%d >= ", name, strlen(name), ASHMEM_NAME_LEN);
	int r = ioctl(fd, ASHMEM_SET_NAME, name);
	if (r >= 0) {
            r = ioctl(fd, ASHMEM_SET_SIZE, size);
	}
	if (r < 0) {
	    close(fd);
	    return r;
	}
    }
    if (fd >= 0) {
        int r = zashmem_set_prot_region(fd, prot);
        if (r < 0) {
            close(fd);
            return r;
        }
    }
    return fd;
}

/* // Not available in user land on Android:
int zashmem_pin_region(int fd, size_t offset, size_t len) {
    struct ashmem_pin p = { offset, len };
    return ioctl(fd, ASHMEM_PIN, &p);
}

int zashmem_unpin_region(int fd, size_t offset, size_t len) {
    struct ashmem_pin p = { offset, len };
    return ioctl(fd, ASHMEM_UNPIN, &p);
}

int zashmem_get_pin_status(int fd, size_t offset, size_t len) {
    struct ashmem_pin p = { offset, len };
    return ioctl(fd, ASHMEM_UNPIN, &p);
}
*/

#elif __MACH__

int zashmem_create_protected_region(const char* name, size_t size, int prot) {
    assert(name != null && name[0] != 0);
    int n = strlen(name) + 15;
    char buf[n];
    if (name[0] != '/') {
        snprintf(buf, n, "/%s", name);
        name = buf;
    }
    mode_t mode = 0;
    const int p =  prot & (PROT_READ|PROT_WRITE);
    if (p == PROT_READ) {
        mode = O_RDONLY;
    } else if (p == PROT_WRITE) {
        mode = O_WRONLY;
    } else if (p == (PROT_READ|PROT_WRITE)) {
        mode = O_RDWR;
    } else {
        assert(((PROT_READ|PROT_WRITE) & prot) != 0);
        return -1;
    }
    int fd = shm_open(name, mode);
    return fd;
}

#endif

