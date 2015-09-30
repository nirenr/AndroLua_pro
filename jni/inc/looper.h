#ifndef LOOPER_H
#define LOOPER_H

typedef void* looper_t;

looper_t* looper_create(void* that,
                        void (*start)(void* that),
                        void (*dispatch)(void* that, void* data, int bytes),
                        void (*stop)(void* that));
void looper_destroy(looper_t* looper);

void looper_post(looper_t* looper, void* data, int bytes, bool copy);
void looper_post_dup(looper_t* looper, void* data, int bytes);
void looper_post_disposable(looper_t* looper, void* data, int bytes, void (*dispose)(void* that, void* data, int bytes));

#endif /* LOOPER_H */
