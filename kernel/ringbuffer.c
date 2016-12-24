#include "platform.h"
#include "ringbuffer.h"
#include "util.h"

void ringbuffer_init(struct ringbuffer *ring,
                     uint8_t *buf,
                     size_t unit,
                     size_t size)
{
    ring->buf = buf;
    ring->unit = unit;
    ring->size = size;
    ring->head = ring->tail = 0;
    ring->empty = true;
}

void ringbuffer_append(struct ringbuffer *ring,
                       const uint8_t *in,
                       size_t len)
{
    ASSERT(ring->unit == 1);

    if (!len) {
        return;
    }

    /* Make sure we're not already full. */
    ASSERT(ring->empty || ring->head != ring->tail);

    if (ring->head <= ring->tail) {
        size_t end_copy = MIN(len, ring->size - ring->tail);
        memcpy(&ring->buf[ring->tail], in, end_copy);
        len -= end_copy;
        in += end_copy;
        ring->tail = (ring->tail + end_copy) % ring->size;
    }

    if (ring->tail < ring->head) {
        ASSERT(len < ring->head - ring->tail);
        memcpy(&ring->buf[ring->tail], in, len);
        ring->tail += len;
    }

    ring->empty = false;
}

void ringbuffer_append_unit(struct ringbuffer *ring, const uint8_t *in)
{
    ASSERT(ring->empty || ring->head != ring->tail);
    memcpy(&ring->buf[ring->tail], in, ring->unit);
    ring->tail = (ring->tail + ring->unit) % ring->size;
    ring->empty = false;
}

bool ringbuffer_consume(struct ringbuffer *ring, uint8_t *c)
{
    if (ring->empty) {
        return false;
    }

    *c = ring->buf[ring->head];
    ring->head = (ring->head + 1) % ring->size;
    if (ring->head == ring->tail) {
        ring->empty = true;
    }

    return true;
}

bool ringbuffer_empty(struct ringbuffer *ring)
{
    return ring->empty;
}

bool ringbuffer_consume_unit(struct ringbuffer *ring, uint8_t *c)
{
    if (ring->empty) {
        return false;
    }

    /* Because we support peek() */
    if (c) {
        memcpy(c, &ring->buf[ring->head], ring->unit);
    }
    ring->head = (ring->head + ring->unit) % ring->size;
    if (ring->head == ring->tail) {
        ring->empty = true;
    }

    return true;
}

bool ringbuffer_peek_unit(struct ringbuffer *ring, uint8_t *c)
{
    if (ring->empty) {
        return false;
    }

    memcpy(c, &ring->buf[ring->head], ring->unit);
    return true;
}
