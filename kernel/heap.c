#include <stdint.h>
#include "heap.h"
#include "platform.h"

#define TMP(heap) ((void *)((heap)->data))
#define IDX(heap, idx)                                      \
({                                                          \
    struct heap *_h = (heap);                               \
    /* First, jump over the temporary item. */              \
    uint8_t *_h_start = (uint8_t *)TMP(_h) + _h->size;      \
    /* Then compute the position of the item itself. */     \
    (void *)(_h_start + (idx) * _h->size);                  \
})

void heap_init(struct heap *heap,
               void *buf,
               heap_comparator_f cmp,
               int itemcnt,
               size_t size)
{
	heap->cmp = cmp;
	heap->itemcnt = itemcnt;
	heap->size = size;
	heap->last = -1; /* initially empty */
    heap->data = buf;
}

void *heap_peek(struct heap *heap)
{
	return heap->last < 0 ? NULL : IDX(heap, 0);
}

static void swap_indices(struct heap *heap, int x, int y)
{
    memcpy(TMP(heap), IDX(heap, x), heap->size);
	memcpy(IDX(heap, x), IDX(heap, y), heap->size);
	memcpy(IDX(heap, y), TMP(heap), heap->size);
}

void heap_insert(struct heap *heap, void *item)
{
    ASSERT(heap->last < heap->itemcnt);

	heap->last++;
    memcpy(IDX(heap, heap->last), item, heap->size);

	/* Bubble up while the rank of heap new item is greater than its parent's */
	int i = heap->last;
	while (i) {
		int parent = (i - 1) / 2;
		if(heap->cmp(IDX(heap, i), IDX(heap, parent))) {
			swap_indices(heap, i, parent);
			i = parent;
		} else {
			break;
		}
	}
}

void heap_delete(struct heap *heap)
{
    ASSERT(heap->last != -1);

	memcpy(IDX(heap, 0), IDX(heap, heap->last), heap->size);
	heap->last--;

	/* Boogie down! */
	int i = 0, child = (i * 2) + 1;
	while(child <= heap->last) {
		int winner = child;
		
		if(child + 1 <= heap->last) {
			winner += !(heap->cmp(IDX(heap, child), IDX(heap, child + 1)));
		}

		int do_swap = !(heap->cmp(IDX(heap, i), IDX(heap, winner)));

		if(do_swap) {
			swap_indices(heap, i, winner);
			i = winner;
			child = (i * 2) + 1;
		} else {
			break;
		}
	}
}
