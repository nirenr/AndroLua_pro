#include "manifest.h"

queue_t* queue_append(queue_t* head, queue_t* elem) {
    if (head == null) {
        head = elem->next = elem->back = elem;
    } else {
        assert(head->back->next == head && head->next->back == head);
        queue_t* back = head->back;
        elem->next = head;
        elem->back = back;
        head->back = elem;
        back->next = elem;
        assert(elem->back->next == elem && elem->next->back == elem);
    }
    assert(head->back->next == head && head->next->back == head);
    return head;
}

queue_t* queue_remove(queue_t* head) {
    assert(head != null && head->back->next == head && head->next->back == head);
    queue_t* back = head->back;
    queue_t* next = back != head ? head->next : null;
    if (next != null) {
        back->next = next;
        next->back = back;
    }
    head->back = head->next = null;
    assert(next == null || next->back->next == next && next->next->back == next);
    return next;
}

