#ifndef DMA_DRV_H
#define DMA_DRV_H


#define DMA_CHANNEL1_EV 0x01
#define DMA_CHANNEL2_EV 0x02 //when DMA ISR fires for that channel
#define DMA_CHANNEL2_DISABLE 0x04 //no more data to transfer; queue ownership is transferred

extern struct k_event dma_event;


void configure_dma1(void);

int dma1ch1_oldest(void);
int dma1ch1_latest(void);
volatile int * dma1ch1_rdy_cnt(void);
volatile uint16_t * dma1ch1_get(uint32_t idx);



int dma1ch2_rdy(void);
int dma1ch2_memcpy(void *destination , const void* source, size_t size );

#endif