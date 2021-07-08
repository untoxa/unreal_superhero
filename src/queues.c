#include <stdint.h>
#include "queues.h"

uint8_t free_head = 0, free_tail = 0;
uint8_t free[QUEUE_LEN];

uint8_t display_head = 0, display_tail = 0;
uint8_t display[QUEUE_LEN];
