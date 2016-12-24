#ifndef CABOOSE_UTIL_H
#define CABOOSE_UTIL_H

#define __packed __attribute__((packed))
#define __noreturn __attribute__((noreturn))
#define __unused __attribute__((unused))
#define __aligned(alignment) __attribute__((aligned(alignment)))

#define MIN(a,b)                                        \
({                                                      \
    typeof(a) _a = (a);                                 \
    typeof(b) _b = (b);                                 \
    /* Compile time type-compatibility assertion */     \
    (void) (&_a == &_b);                                \
    _a < _b ? _a : _b;                                  \
})

#define MAX(a,b)                                        \
({                                                      \
    typeof(a) _a = (a);                                 \
    typeof(b) _b = (b);                                 \
    /* Compile time type-compatibility assertion */     \
    (void) (&_a == &_b);                                \
    _a > _b ? _a : _b;                                  \
})

#define ALIGN(n, a)                                     \
({                                                      \
    typeof(a) _a = (a);                                 \
    ((((n) + (_a - 1)) / _a) * _a);                     \
})

/* The linux kernel classic. */
#define containerof(ptr, type, member)                                  \
({                                                                      \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);                \
    (type *)( (char *)__mptr - offsetof(type,member) );                 \
})

#endif
