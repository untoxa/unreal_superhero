#ifndef _QUEUES_H_INCLUDE_
#define _QUEUES_H_INCLUDE_

#include <stdint.h>

#define QUEUE_LEN 32u
#define QUEUE_MASK (QUEUE_LEN - 1u)

extern uint8_t free_head, free_tail;
extern uint8_t free[QUEUE_LEN];

inline uint8_t alloc_id(void) {
    if (free_head != free_tail) {
        return free[free_tail = (free_tail + 1) & QUEUE_MASK];
    }
    return 0;
}

inline void free_id(uint8_t id) {
    free[free_head = (free_head + 1) & QUEUE_MASK] = id;
}

extern uint8_t display_head, display_tail;
extern uint8_t display[QUEUE_LEN];

inline uint8_t remove_from_display(void) {
    if (display_head != display_tail) {
        return free[display_tail = (display_tail + 1) & QUEUE_MASK];
    }
    return 0;
}

inline uint8_t add_to_display(uint8_t id) {
    display_head++; display_head &= QUEUE_MASK;
    return (display[display_head] = id);
}


#endif