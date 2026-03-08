#include <zephyr/device.h>

#include <stm32_ll_dma.h>
#include <stm32_ll_tim.h>
#include <stm32_ll_adc.h>
#include <stm32_ll_bus.h>


#include "common.h"
#include "dma_driver.h"
#include "processor.h"


volatile uint16_t adc_buffer[BUFFER_COUNT][FFT_SIZE] __attribute__((aligned(2)));
volatile int current_buffer = 0; //the one we are currently writing to
volatile int buffers_ready = 0;

K_EVENT_DEFINE(dma_event);



int dma1ch1_oldest(void)
{
    int cur = current_buffer;
    int rdy = buffers_ready;
    int idx = (cur - 1 - rdy) % BUFFER_COUNT;
    if (idx < 0) {
        idx += BUFFER_COUNT;
    }

    return idx;
}

int dma1ch1_latest(void)
{
    return (current_buffer + BUFFER_COUNT - 1 ) % BUFFER_COUNT;
}

volatile uint16_t * dma1ch1_get(uint32_t idx)
{
    if(idx > BUFFER_COUNT) return NULL;
    return adc_buffer[idx];
}



volatile int * dma1ch1_rdy_cnt(void)
{
    return &buffers_ready;
}

int dma1ch2_rdy(void)
{
    return !LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2);
}


// /\ basically a round robin


void DMA1_Channel1_IRQHandler(const void *arg)
{
    ARG_UNUSED(arg);

    /*
    if (LL_DMA_IsActiveFlag_HT1(DMA1)) { 
        LL_DMA_ClearFlag_HT1(DMA1);
    }*/

    if (LL_DMA_IsActiveFlag_TC1(DMA1)) {  // Transfer Complete
        LL_DMA_ClearFlag_TC1(DMA1);   // Clear flag

        LL_TIM_DisableCounter(TIM2);
        LL_TIM_SetCounter(TIM2, 0); 
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);


        buffers_ready++;
        current_buffer = (current_buffer + 1) % BUFFER_COUNT;


        if(dma1ch2_rdy() &&                                         // CH2 idle, 
            (
                !k_event_test(&dma_event, DMA_CHANNEL2_DISABLE) &&  // not locked
                !LL_DMA_IsActiveFlag_TC2(DMA1)                      // and not awaiting ISR access
            ))
        {
            volatile uint16_t * next_address;
            proc_acq_next_buffer(&next_address);
            dma1ch2_memcpy((void*)next_address ,(void*)adc_buffer[dma1ch1_oldest()], FFT_SIZE );
        }

        
        LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)adc_buffer[current_buffer]);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, FFT_SIZE);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);

        k_event_post(&dma_event, DMA_CHANNEL1_EV ); 
    }
    
}


static volatile void * dma1ch2_last_target = NULL;

int dma1ch2_memcpy(void *destination , const void* source, size_t size ) {
IRQ_LOCKED {

    compiler_barrier();
    if(LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2) || !source || !destination) //already running
    {
        return EXIT_FAILURE;
    }

    dma1ch2_last_target = destination;
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2,(uint32_t)source, (uint32_t)destination, LL_DMA_DIRECTION_MEMORY_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, size);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
}
return EXIT_SUCCESS;
}


void DMA1_Channel2_IRQHandler(const void *arg)
{
    ARG_UNUSED(arg);

    if (LL_DMA_IsActiveFlag_TC2(DMA1)) {  // Transfer Complete
        LL_DMA_ClearFlag_TC2(DMA1);   // Clear flag

        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);

        if(buffers_ready)
        {
            buffers_ready--;
        }

        if(k_event_test(&dma_event, DMA_CHANNEL2_DISABLE))
        {
            //lost transfer; invalidate the queue TODO
            proc_invalidate();
            goto finalize;
        }

        proc_sup_next_buffer(dma1ch2_last_target);

        if(!buffers_ready /*or queue filling up*/) //no more data to be transferred; ML may start processing
        {
            k_event_post(&dma_event, DMA_CHANNEL2_DISABLE);
            goto finalize;
        }

        volatile uint16_t * next_address;
        proc_acq_next_buffer(&next_address);
        dma1ch2_memcpy((void*)next_address ,(void*)adc_buffer[dma1ch1_oldest()], FFT_SIZE );

finalize:
        k_event_post(&dma_event, DMA_CHANNEL2_EV);   
    }
    
}




void configure_dma1(void)
{

    /**
     * @brief Channel 1 - ADC to MEM
     * 
     */

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    DMA1_Channel1->CCR = 0; // Clear config first

    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_1, 
        LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA), 
        (uint32_t)adc_buffer, 
        LL_DMA_DIRECTION_PERIPH_TO_MEMORY
    );
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1 ,LL_DMA_CHANNEL_1, FFT_SIZE);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1 , LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_SetChannelPriorityLevel(DMA1 , LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_HIGH);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT );
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT); 

    //LL_DMA_EnableIT_HT(DMA1,LL_DMA_CHANNEL_1);   
    LL_DMA_EnableIT_TC(DMA1,LL_DMA_CHANNEL_1);   
    IRQ_CONNECT(DMA1_Channel1_IRQn,     /* NOT TIM1_IRQn */
            1,               /* priority */
            DMA1_Channel1_IRQHandler,
            NULL,
            0);
    irq_enable(DMA1_Channel1_IRQn);

    /**
     * @brief Channel 2 - MEM to MEM
     * 
     */


    DMA1_Channel2->CCR = 0;
    LL_DMA_SetDataLength(DMA1 ,LL_DMA_CHANNEL_2, FFT_SIZE);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_2 , LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_SetChannelPriorityLevel(DMA1 , LL_DMA_CHANNEL_2, LL_DMA_PRIORITY_MEDIUM);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PERIPH_INCREMENT );
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_INCREMENT); 
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_MEMORY_TO_MEMORY);
    
    LL_DMA_EnableIT_TC(DMA1,LL_DMA_CHANNEL_2);   
    IRQ_CONNECT(DMA1_Channel2_IRQn,     /* NOT TIM1_IRQn */
            1,               /* priority */
            DMA1_Channel2_IRQHandler,
            NULL,
            0);
    irq_enable(DMA1_Channel2_IRQn);

}
