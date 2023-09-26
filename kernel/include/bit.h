#ifndef __BIT_H__
#define __BIT_H__

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define bit_test(var, offset)                                          \
    ({                                                                 \
        bool __ret;                                                    \
        __asm__ volatile("bt %2, %1;"                                  \
                         : "=@ccc"(__ret)                              \
                         : "r"((uint32_t)var), "r"((uint32_t)offset)); \
        __ret;                                                         \
    })

static inline bool bitmap_test(void *bitmap, size_t bit) {
    bool ret;
    __asm__ volatile("bt %2, (%1)" : "=@ccc"(ret) : "r"(bitmap), "r"(bit) : "memory");
    return ret;
}

static inline bool bitmap_set(void *bitmap, size_t bit) {
    bool ret;
    __asm__ volatile("bts %2, (%1)" : "=@ccc"(ret) : "r"(bitmap), "r"(bit) : "memory");
    return ret;
}

static inline bool bitmap_unset(void *bitmap, size_t bit) {
    bool ret;
    __asm__ volatile("btr %2, (%1)" : "=@ccc"(ret) : "r"(bitmap), "r"(bit) : "memory");
    return ret;
}

#endif
