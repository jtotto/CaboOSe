#include "engine.h"
#include "events.h"
#include "state.h"
#include "tasks.h"

struct caboose caboose;

int caboose_init(uint8_t *pool)
{
    /* Each initialization routine is given a word-aligned pool pointer,
     * allocates any static storage it needs from it and returns a word-aligned
     * pointer to the new beginning of the free pool. */
    pool = tasks_init(pool);
    pool = events_init(pool);
    
    engine();

    return 0;
}
