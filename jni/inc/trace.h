#ifndef TRACE_H
#define TRACE_H
#include "manifest.h"

#if defined(DEBUG) || defined(TRACE)
#define trace(...) trace_(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define trace(...)
#endif

#if defined(DEBUG) || defined(ASSERT)
#define assertion(e, ...) (void)(!(e) ? assert_(#e, __FILE__, __LINE__, __func__, ##__VA_ARGS__) : true)
#undef assert
#define assert(e) assertion(e, "")
#else
#define assertion(...)
#endif

#if defined(DEBUG) || defined(HEXDUMP)
#define hexdump(data, length) hexdump_(__FILE__, __LINE__, __func__, data, length)
#else
#define hexdump(...)
#endif

#if defined(DEBUG) || defined(ASSERT) /* posix_ok(r) asserts that posix result is 0 aka OK */
#define posix_ok(r) do { int _##r = (r); int e = errno; assertion(_##r == 0, "ret=%d errno=%d \"%s\"", _##r, e, strerror(e)); } while (false)
#else
#define posix_ok(r) do { (void)(r); } while (false)
#endif

void trace_(const char* file, int line, const char* func, const char* fmt, ...);
bool assert_(const char* condition, const char* file, int line, const char* func, const char* fmt, ...);
void hexdump_(const char* file, int line, const char* func, void* data, int bytes);

#endif /* TRACE_H */
