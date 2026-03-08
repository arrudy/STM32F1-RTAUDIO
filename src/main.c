
#include <zephyr/device.h>
#include <zephyr/fatal.h>

/* Low Level Headers */
#include <stm32_ll_adc.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_dma.h>
#include <stm32_ll_tim.h>

#include "processor.h"
#include "common.h"

#include "dma_driver.h"
#include "tim_driver.h"



void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf)
{
    ARG_UNUSED(reason);

    printk("💥 FATAL ERROR: %u\n", reason);

#if defined(CONFIG_ARM)
    printk("r0  = 0x%08x\n", esf->basic.r0);
    printk("r1  = 0x%08x\n", esf->basic.r1);
    printk("r2  = 0x%08x\n", esf->basic.r2);
    printk("r3  = 0x%08x\n", esf->basic.r3);
    printk("pc  = 0x%08x\n", esf->basic.pc);
    printk("lr  = 0x%08x\n", esf->basic.lr);
    printk("psr = 0x%08x\n", esf->basic.xpsr);
#endif

    while (1) {
        k_cpu_idle();
    }
}


// void ADC1_2_IRQn_IRQHandler(const void * arg)
// {
//     ARG_UNUSED(arg);

//     if(LL_ADC_IsActiveFlag_EOS(ADC1))
//         LL_ADC_ClearFlag_EOS(ADC1);
    
//     printk("ADC!");
// }








static inline void configure_adc1(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);
    ADC1->CR2 = 0; // Reset control register

    // Enable ADC
    LL_ADC_Enable(ADC1);

    // Wait ADC to be ready (optional but recommended)
    while (!LL_ADC_IsEnabled(ADC1)) {}
    LL_ADC_StartCalibration(ADC1);
    while (LL_ADC_IsCalibrationOnGoing(ADC1)) {}

    LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT);

    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_0, LL_ADC_SAMPLINGTIME_55CYCLES_5);

    // Select external trigger: TIM1 CC1 (EXTSEL=000) and rising edge
    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_EXT_TIM2_CH2 );
    LL_ADC_REG_StartConversionExtTrig(ADC1, LL_ADC_REG_TRIG_EXT_RISING);
    LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED );

    // LL_ADC_EnableIT_EOS(ADC1);
    // IRQ_CONNECT(ADC1_2_IRQn,     /* NOT TIM1_IRQn */
    //         0,               /* priority */
    //         ADC1_2_IRQn_IRQHandler,
    //         NULL,
    //         0);
    // irq_enable(ADC1_2_IRQn);
}






int main(void)
{

    configure_adc1();
    configure_dma1();
    LL_DMA_EnableChannel(DMA1,LL_DMA_CHANNEL_1);
    configure_tim1();
    configure_tim2();
    LL_TIM_EnableCounter(TIM1);
    
    printk("Orchestration running at 10Hz. Timer triggering at 16kHz. ADC armed on Hardware Trigger.\n");
    
    while (1) {
        // k_event_wait(&dma_event, 0x1, true, K_FOREVER);
        
        // if (buffers_ready > 1) {
        //     buffers_ready--;
        //     printk("Buffer %d ready\n", BUFFER_LATEST );
        // }
        const uint32_t tracked_events = DMA_CHANNEL1_EV | DMA_CHANNEL2_DISABLE;
        uint32_t status = k_event_wait(&dma_event, tracked_events, false, K_FOREVER);
        
        if(status & DMA_CHANNEL1_EV && *dma1ch1_rdy_cnt() > 1)
        {
            //printk("B%d ready, %d/3\n", dma1ch1_latest(), *dma1ch1_rdy_cnt() );
            k_event_clear(&dma_event, DMA_CHANNEL1_EV);
        }
        if(status & DMA_CHANNEL2_DISABLE)
        {
            printk("P%d/3\n", *dma1ch1_rdy_cnt() );
            proc_tick();
            k_event_clear(&dma_event, DMA_CHANNEL2_DISABLE);
        }
    }
    return 0;
}