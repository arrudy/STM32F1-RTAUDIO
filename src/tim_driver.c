#include "tim_driver.h"

#include <stm32_ll_gpio.h>
#include <stm32_ll_tim.h>
#include <stm32_ll_bus.h>


// void TIM1_UP_IRQn_IRQHandler(const void * arg)
// {
//     ARG_UNUSED(arg);

//     if(LL_TIM_IsActiveFlag_UPDATE(TIM1))
//         LL_TIM_ClearFlag_UPDATE(TIM1);

//     printk("HzTRG!");
// }


/**
 * @brief Orchestration trigger, slow (in Hz)
 * 
 */
void configure_tim1(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

    LL_TIM_SetPrescaler(TIM1, TIM1_PRESCALER);
    LL_TIM_SetAutoReload(TIM1, TIM1_AUTORELOAD);
    LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);

    //LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
    //LL_TIM_OC_SetCompareCH1(TIM1, TIM1_COMPARE);

    LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_UPDATE );
    //LL_TIM_EnableAllOutputs(TIM1);

    // LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);//the funky undocumented case

    // LL_TIM_EnableIT_UPDATE(TIM1);
    // IRQ_CONNECT(TIM1_UP_IRQn,     /* NOT TIM1_IRQn */
    //         0,               /* priority */
    //         TIM1_UP_IRQn_IRQHandler,
    //         NULL,
    //         0);
    // irq_enable(TIM1_UP_IRQn);
}


/**
 * @brief Direct ADC trigger, fast (in kHz)
 * 
 */
void configure_tim2(void)
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    LL_TIM_SetPrescaler(TIM2, TIM2_PRESCALER);
    LL_TIM_SetAutoReload(TIM2, TIM2_AUTORELOAD);
    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);

    LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetCompareCH2(TIM2, TIM2_COMPARE);

    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_OC2REF);
    LL_TIM_EnableAllOutputs(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);//the funky undocumented case

    LL_TIM_SetTriggerInput(TIM2, LL_TIM_TS_ITR0);
    LL_TIM_SetSlaveMode(TIM2, LL_TIM_SLAVEMODE_TRIGGER);

    //#ifndef NDEBUG //OUT to PA1
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_1, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_1, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_1, LL_GPIO_SPEED_FREQ_HIGH);
    //#endif
    

    // LL_TIM_EnableIT_CC1(TIM2);
    // IRQ_CONNECT(TIM2_IRQn,     /* NOT TIM1_IRQn */
    //         0,               /* priority */
    //         TIM2_IRQn_IRQHandler,
    //         NULL,
    //         0);
    // irq_enable(TIM2_IRQn);
}
