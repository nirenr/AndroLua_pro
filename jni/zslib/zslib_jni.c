#include "manifest.h"
#include "zslib.h"
#include "revision.h"
#include <jni.h>
#include <android/bitmap.h>

void Java_com_zspace_Native_memcpyAB(JNIEnv* env, jobject that, jlong address, jbyteArray data, jint offset, jint length) {
    jbyte* a = (*env)->GetByteArrayElements(env, data, NULL);
    jsize  bytes = (*env)->GetArrayLength(env, data);
//  trace("%p %p offset=%d length=%d bytes=%d", ll2p(address), a, offset, length, bytes);
    assertion(0 <= offset && offset + length <= bytes, "offset=%d length=%d bytes=%d", offset, length, bytes);
    memcpy(ll2p(address), a + offset, length);
    (*env)->ReleaseByteArrayElements(env, data, a, 0);
}

void Java_com_zspace_Native_memcpyBA(JNIEnv* env, jobject that, jbyteArray data, jlong address, jint offset, jint length) {
    jbyte* a = (*env)->GetByteArrayElements(env, data, NULL);
    jsize  bytes = (*env)->GetArrayLength(env, data);
//  trace("%p %p offset=%d length=%d bytes=%d", ll2p(address), a, offset, length, bytes);
    assertion(0 <= offset && offset + length <= bytes, "offset=%d length=%d bytes=%d", offset, length, bytes);
    memcpy(a + offset, ll2p(address), length);
    (*env)->ReleaseByteArrayElements(env, data, a, 0);
}

void Java_com_zspace_Native_memcpyAI(JNIEnv* env, jobject that, jlong address, jintArray data, jint offset, jint length) {
    jbyte* a = (jbyte*)(*env)->GetIntArrayElements(env, data, NULL);
    jsize  bytes = (*env)->GetArrayLength(env, data) * sizeof(jint);
    offset *= sizeof(jint);
    length *= sizeof(jint);
//  trace("%p %p offset=%d length=%d bytes=%d", ll2p(address), a, offset, length, bytes);
    assertion(0 <= offset && offset + length <= bytes, "offset=%d length=%d bytes=%d", offset, length, bytes);
    memcpy(ll2p(address), a + offset, length);
    (*env)->ReleaseByteArrayElements(env, data, a, 0);
}

void Java_com_zspace_Native_memcpyIA(JNIEnv* env, jobject that, jintArray data, jlong address, jint offset, jint length) {
    jbyte* a = (jbyte*)(*env)->GetIntArrayElements(env, data, NULL);
    jsize  bytes = (*env)->GetArrayLength(env, data) * sizeof(jint);
    offset *= sizeof(jint);
    length *= sizeof(jint);
//  trace("%p %p offset=%d length=%d bytes=%d", ll2p(address), a, offset, length, bytes);
    assertion(0 <= offset && offset + length <= bytes, "offset=%d length=%d bytes=%d", offset, length, bytes);
    memcpy(a + offset, ll2p(address), length);
    (*env)->ReleaseByteArrayElements(env, data, a, 0);
}

jlong Java_com_zspace_Native_malloc(JNIEnv* env, jobject that, jint bytes) {
    return p2ll(mem_allocz(bytes));
}

void Java_com_zspace_Native_free(JNIEnv* env, jobject that, jlong address) {
    mem_free(ll2p(address));
}

void Java_com_zspace_Native_memcpy(JNIEnv* env, jobject that, jlong to, jlong from, jint bytes) {
    memcpy(ll2p(to), ll2p(from), bytes);
}

/* TODO: as soon as I have a second platform specific function updateBitmap will deserve it's own library */

void Java_com_zspace_Android_updateBitmap(JNIEnv* env, jobject that, jobject bitmap, jlong data) {
    // timestamp: 122..305 microseconds mean=152. Contested lock up to 5800 mcs
//  timestamp("updateBitmap");
    void* buffer = ll2p(data);
    AndroidBitmapInfo info = {0};
    int r = AndroidBitmap_getInfo(env, bitmap, &info);
    if (r != 0) {
        trace("AndroidBitmap_getInfo() failed ! error=%d", r);
        return;
    }
    int width = info.width;
    int height = info.height;
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888 && info.format != ANDROID_BITMAP_FORMAT_A_8) {
        trace("Bitmap format is not RGBA_8888 or A_8");
        return;
    }
    void* pixels = null;
    r = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (r != 0) {
        trace("AndroidBitmap_lockPixels() failed ! error=%d", r);
        return;
    }
    memcpy(pixels, buffer, width * height * (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ? sizeof(int) : 1));
    AndroidBitmap_unlockPixels(env, bitmap);
//  timestamp("updateBitmap");
}

/*
TODO: https://vec.io/posts/how-to-render-image-buffer-in-android-ndk-native-code
void Java_com_zspace_Android_drawToSurface(JNIEnv* env, jobject that, jobject surface, jlong pixels, int w, int h) {
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(window, &buffer, NULL) == 0) {
      memcpy(buffer.bits, pixels,  w * h * 4);
      ANativeWindow_unlockAndPost(window);
    }
    ANativeWindow_release(window);
}
*/

jint Java_com_zspace_Android_byteArrayToFile(JNIEnv* env,jobject that, jstring filename, jbyteArray array) {
    jboolean isCopy = false;
    const char* fn = (*env)->GetStringUTFChars(env, filename, &isCopy);
    jbyte* data = (*env)->GetByteArrayElements(env, array, NULL);
    jsize bytes = (*env)->GetArrayLength(env, array);
    mode_t mode = S_IRUSR | S_IWUSR /* | S_IRGRP | S_IROTH */; // only user permissions have effect
    int fd = open(fn, O_WRONLY|O_CREAT|O_TRUNC, mode);
    int e = 0;
    if (fd > 0) {
        int r = write(fd, data, bytes);
        if (r != bytes) {
            e = EIO;
        }
        close(fd);
    } else {
        e = errno;
    }
/*
    fd = open(fn, O_RDONLY);
    if (fd > 0) {
        char buf[bytes];
        int r = read(fd, buf, bytes);
        if (r != bytes || memcmp(buf, data, bytes) != 0) {
            e = EIO;
        }
        close(fd);
    } else {
        e = errno;
    }
*/
    (*env)->ReleaseByteArrayElements(env, array, data, 0);
    (*env)->ReleaseStringUTFChars(env, filename, fn);
    return -e;
}
