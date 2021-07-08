#ifndef _QUEUES_H_INCLUDE_
#define _QUEUES_H_INCLUDE_

#include <stdint.h>

#define QUEUE_LEN 32u
#define QUEUE_MASK (QUEUE_LEN - 1u)

extern uint8_t free_head, free_tail;
extern uint8_t free[QUEUE_LEN];

inline uint8_t alloc_id() {
    if (free_head != free_tail) {
        free_tail++; free_tail &= QUEUE_MASK;
        return free[free_tail];
    }
    return 0;
}

inline void free_id(uint8_t id) {
    free_head++; free_head &= QUEUE_MASK;
    free[free_head] = id;
}

extern uint8_t display_head, display_tail;
extern uint8_t display[QUEUE_LEN];

inline uint8_t remove_from_display() {
    if (display_head != display_tail) {
        display_tail++; display_tail &= QUEUE_MASK;
        return free[display_tail];
    }
    return 0;
}

inline void add_to_display(uint8_t id) {
    display_head++; display_head &= QUEUE_MASK;
    display[display_head] = id;
}


#endif