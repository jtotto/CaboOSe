#ifndef CABOOSE_HEAP_H
#define CABOOSE_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* a heap_comparator_f returns 1 if x ranks strictly greater than y, else 0 */
typedef bool (*heap_comparator_f)(void *x, void *y);

struct heap {
    heap_comparator_f cmp;
    int itemcnt;
    size_t size;
    int last;
    uint8_t *data;
};

#define HEAP_ITEMS(cnt) ((cnt) + 1)

void heap_init(struct heap *heap,
               void *buf,
               heap_comparator_f cmp,
               int itemcnt,
               size_t size);

/* NOTE: this reference is invalidated by the next call to heap_delete */
void *heap_peek(struct heap *heap);
void heap_insert(struct heap *heap, void *item);
void heap_delete(struct heap *heap);

#endif
