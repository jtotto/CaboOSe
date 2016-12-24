#ifndef CABOOSE_INTRUSIVE_LIST_H
#define CABOOSE_INTRUSIVE_LIST_H

/* queue(3) STAILQ knockoff */

#include <stdbool.h>
#include <stddef.h>

struct node {
    struct node *next;
};

struct list {
    struct node *head;
    struct node *tail;
};

#define list_init(l) do {    \
    typeof(l) _l = (l);      \
    (_l)->head = NULL;       \
    (_l)->tail = NULL;       \
} while (false)

#define list_empty(l) (!(l)->head)

#define list_next(node)         \
({                              \
    (node)->next;               \
})

#define list_head(l) ((l)->head)
#define list_tail(l) ((l)->tail)

#define list_insert_tail(l, node) do {      \
    typeof(node) _node = (node);            \
    typeof(l) _l = (l);                     \
    _node->next = NULL;                     \
    if (!_l->head) {                        \
        _l->head = _node;                   \
    } else {                                \
        _l->tail->next = _node;             \
    }                                       \
    _l->tail = _node;                       \
} while (0)

#define list_remove_head(l)                 \
({                                          \
    typeof(l) _l = (l);                     \
    typeof(_l->head) _rv = _l->head;        \
    _l->head = _l->head->next;              \
    _rv;                                    \
})

#endif
