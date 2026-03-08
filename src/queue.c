#include "queue.h"

int enqueue(volatile Queue *queue, void *data) {
IRQ_LOCKED{
  if (queue->size >= queue->max_size) {
    return EXIT_FAILURE; // Queue full
  }
  queue->buffer[queue->front++] = data;
  queue->size++;
  queue->front %= queue->max_size;
}
  return EXIT_SUCCESS; // Success
}


int dequeue(volatile Queue *queue, void **result) {
IRQ_LOCKED{
  if (queue->size == 0) {
    *result = NULL;
    return EXIT_FAILURE; // Queue empty
  }
  // Calculate the back index (oldest element)
  uint8_t back = (queue->front - queue->size + queue->max_size) % queue->max_size;
  if(result)
    *result = queue->buffer[back];
  queue->size--;
}
  return EXIT_SUCCESS; // Success
}


int queue_get(const volatile Queue *queue, uint8_t n, void **result) { 
IRQ_LOCKED{
  if (n >= queue->size) {
    *result = NULL;
    return EXIT_FAILURE; // Index out of bounds
  }
  // Calculate the circular index for the n-th element (0 = oldest)
  uint8_t index = (queue->front - queue->size + n + queue->max_size) % queue->max_size;
  *result = queue->buffer[index];
}
  return EXIT_SUCCESS; // Success
}



