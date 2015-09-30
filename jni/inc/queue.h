#ifndef QUEUE_H
#define QUEUE_H

typedef struct _queue_t {
   struct _queue_t *next;
   struct _queue_t *back;
   void* data;
   size_t bytes; /* size of data in bytes, 0 if unknown (e.g. mem_strdup result */
   void (*dispose)(void* that, void* data, int bytes); /* if not null called before zfree */
   bool copy; /* true => call mem_free(data) */
} queue_t;

/* both append and remove returns (possibly new) head */
queue_t* queue_append(queue_t* head, queue_t* elem);
queue_t* queue_remove(queue_t* head); /* returns null on the last remove */

#endif /* QUEUE_H */
