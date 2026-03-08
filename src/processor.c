#include "processor.h"
#include "queue.h"
#include "common.h"
#include "dma_driver.h"

#include <zephyr/dsp/dsp.h>
#include <arm_math.h>


static volatile uint16_t data_buffer[FFT_SEGMENTS][FFT_SIZE] __attribute__((aligned(2)));;
DEFINE_QUEUE(ready_buffer, FFT_SEGMENTS); //hold the current FFT window & track its status

#include "ABcoef_arr.h"


//number of buffers that underwent an FFT
static volatile int ffted_buffers = 0;

void proc_invalidate()
{
  queue_reset(&ready_buffer);
  ffted_buffers = 0;
}




/**
 * @brief Pointer for the next elegible DMA buffer
 * 
 * @param uint8_t the amout of ready buffers
 * @return uint16_t* 
 * Meant for ISR context
 */
void proc_acq_next_buffer(volatile uint16_t ** buffer)
{
  volatile int * ready_count = dma1ch1_rdy_cnt();

  if(*ready_count >= BUFFER_COUNT) //may cause a race; minimal distorsion expected
  {
    proc_invalidate();
    *ready_count = 2;
    *buffer = data_buffer[0];
    return;
  }

  uint8_t data_cnt = queue_size(&ready_buffer);
  if(data_cnt == FFT_SEGMENTS) //full; re-using existing buffers
  {
    dequeue(&ready_buffer, (void**)buffer);
    if(ffted_buffers > 0)
    {
      ffted_buffers--;
    }
    return;
  }
  else //filling-up
  {
    *buffer = data_buffer[data_cnt];
    return;
  }
}

int proc_sup_next_buffer(volatile uint16_t * buffer)
{
  return enqueue(&ready_buffer, (void*)buffer);
}


static q15_t fft_buffer[FFT_SIZE * 2];

int proc_tick() {
int exit_status = EXIT_SUCCESS;

  
  if(ffted_buffers == ready_buffer.max_size) //an update has occured
  {
    exit_status = EXIT_FAILURE;
    goto cleanup;
  }
    
  //do the FFT on the frozen chain
  arm_rfft_instance_q15 S;
  arm_status status;

  // 1. Initialize the RFFT. 
  // This points to internal CMSIS tables for 256-point RFFT.
  status = arm_rfft_init_256_q15(&S, 0, 1);

  if (status != ARM_MATH_SUCCESS) {
    exit_status = (int)status; // Initialization failed
    goto cleanup;
  }

  for(; ffted_buffers < ready_buffer.size; ++ffted_buffers)
  {
    volatile uint16_t * buf_ptr;
    (void)queue_get(&ready_buffer, ffted_buffers, &buf_ptr ); 
    arm_rfft_q15(&S, (void*)buf_ptr , fft_buffer);
    arm_cmplx_mag_q15(fft_buffer+2, (void*)(buf_ptr+1), FFT_SIZE);
  }

//do the ML & hand out a decision

  


  if(ready_buffer.size < FFT_SEGMENTS) 
  {
    exit_status = 2; //all buffers not ready yet
    goto cleanup;
  }


  
// IRQ_LOCKED{
//   if(!dma1ch2_rdy() || LL_DMA_IsActiveFlag_TC2(DMA1))
//     goto cleanup;

//   volatile uint16_t * next_data;
//   proc_acq_next_buffer(&next_data);
//   dma1ch2_memcpy((void*)next_data, (void*)dma1ch1_get(dma1ch1_oldest()), FFT_SIZE);
// }


cleanup:
  return exit_status;
}