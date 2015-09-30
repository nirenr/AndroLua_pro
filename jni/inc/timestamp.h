#ifndef TIMESTAMP_H
#define TIMESTAMP_H

int64_t walltime();
int64_t cputime();  /* of calling thread */
void nsleep(int64_t nanoseconds);

#if defined(DEBUG) || defined(TIMESTAMP)
#define timestamp(...) timestamp_(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define timestamp(...)
#endif

int64_t timestamp_(const char* file, int line, const char* func, const char* label);

#endif /* TIMESTAMP_H */
