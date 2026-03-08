#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdint.h>
#include <stdlib.h>

#include <zephyr/kernel.h>


#define FFT_SIZE     256
#define FFT_SEGMENTS 5
#define BUFFER_COUNT 3


void proc_invalidate();
void proc_acq_next_buffer(volatile uint16_t ** buffer);
int proc_sup_next_buffer(volatile uint16_t * buffer);
int proc_tick();

#endif