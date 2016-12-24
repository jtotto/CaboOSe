#ifndef CABOOSE_RINGBUFFER_H
#define CABOOSE_RINGBUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct ringbuffer {
    uint8_t *buf;
    /* Size of each ring entry. */
    size_t unit;
    /* Number of entries in the ring. */
    size_t size;
    /* The first entry in the ring. */
    unsigned int head;
    /* The first free slot in the ring. */
    unsigned int tail;
    /* Is the ring empty?  Distinguishes between a completely full and
     * completely empty ring. */
    bool empty;
};

void ringbuffer_init(struct ringbuffer *ring,
                     uint8_t *buf,
                     size_t unit,
                     size_t size);

void ringbuffer_append(struct ringbuffer *ring,
                       const uint8_t *in,
                       size_t len);

bool ringbuffer_consume(struct ringbuffer *ring, uint8_t *c);
bool ringbuffer_empty(struct ringbuffer *ring);

/* Variants for unit > 1 */
void ringbuffer_append_unit(struct ringbuffer *ring, const uint8_t *in);
bool ringbuffer_consume_unit(struct ringbuffer *ring, uint8_t *c);
bool ringbuffer_peek_unit(struct ringbuffer *ring, uint8_t *c);

#endif
