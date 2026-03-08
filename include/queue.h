#include <stdint.h>
#include <stddef.h> // for NULL
#include <stdlib.h>

#include "common.h"

typedef struct {
    void * volatile *buffer;      // Pointer to the buffer (now holds void*)
    uint8_t max_size;   // Maximum size of the queue
    volatile uint8_t front;      // Index of the front
    volatile uint8_t size;      // Current number of elements
} Queue;

#define DEFINE_QUEUE(name, count) \
  static void * volatile name##_buffer[count]; \
  static Queue name = { \
    .buffer = name##_buffer, \
    .max_size = count, \
    .front = 0, \
    .size = 0, \
  }

int enqueue(volatile Queue *queue, void *data);
int dequeue(volatile Queue *queue, void **result);
int queue_get(const volatile Queue *queue, uint8_t n, void **result);

inline uint8_t queue_size(const volatile Queue *queue) {return queue->size;}
inline void queue_reset(volatile Queue *queue){
IRQ_LOCKED{
  queue->size = 0;
}
}