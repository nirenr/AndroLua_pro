#ifndef ZASHMEM_H
#define ZASHMEM_H

/* ashmem.h/libcutil.so is not a public Android API and subject to change w/o notice.
   But #include <linux/ashmem.h> is a public API and should be stable. pin/unpin
   is not implemented in linux/ashmem driver for Android at the moment of this
   implementation but all created regions are pinned by default */

/* zashmem_create_region - creates a new ashmem region and returns the file descriptor, or <0 on error
 * `name' is an optional label to give the region (visible in /proc/pid/maps)
 * `size' is the size of the region, in page-aligned bytes
 * use close(fd) on the result
 */
extern int zashmem_create_protected_region(const char* name, size_t size, int prot);

/* "For Android, we use ashmem to implement SharedMemory. ashmem_create_region
    will automatically pin the region. We never explicitly call pin/unpin. When
    all the file descriptors from different processes associated with the region
    are closed, the memory buffer will go away."
    see: http://src.chromium.org/svn/trunk/src/base/memory/shared_memory_android.cc
extern int zashmem_pin_region(int fd, size_t offset, size_t len);
extern int zashmem_unpin_region(int fd, size_t offset, size_t len);
extern int zashmem_get_pin_status(int fd, size_t offset, size_t len);
*/

#endif /* ZASHMEM_H */
