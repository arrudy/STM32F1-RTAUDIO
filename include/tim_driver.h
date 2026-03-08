#ifndef TIM_DRIVER_H
#define TIM_DRIVER_H

/*
 * 10 Hz orchestration timer
 *
 */
#define TIM1_PRESCALER 7199 //10k cycles per second
#define TIM1_AUTORELOAD 999
#define TIM1_COMPARE    500


/* 
 * 16 kHz Timer Configuration 
 * 72MHz / 4500 = 16000 Hz 
 */
#define TIM2_PRESCALER 0
#define TIM2_AUTORELOAD 4499 
#define TIM2_COMPARE   2250  /* Trigger halfway through the cycle */

void configure_tim1(void);
void configure_tim2(void);


#endif