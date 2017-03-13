#ifndef _CAN_H_
#define _CAN_H_

#include <stm32f10x.h>


//#define MEDIUM_DENSITY
#ifdef MEDIUM_DENSITY
		#define LEDPIN						  GPIO_Pin_9	
		#define LEDPORT						  GPIOC
		#define BOOT_PIN		  			GPIO_Pin_1
		#define BOOTBUTTON_PIN		  GPIO_IDR_IDR1
		#define BOOTBUTTON_PORT		  GPIOC
		#define RCC_APB2ENR_PORTEN_BOOT	 RCC_APB2ENR_IOPCEN
		#define RCC_APB2ENR_PORTEN_LED	 RCC_APB2ENR_IOPCEN
		#define  GPIO_BSRR_BRx 			GPIO_BSRR_BR9
		#define  GPIO_BSRR_BSx 			GPIO_BSRR_BS9
		#define  GPIO_IDR_IDRX      GPIO_IDR_IDR9
#else
		#define LEDPIN							GPIO_Pin_6
		#define LEDPORT						  GPIOC
		#define BOOT_PIN		  			GPIO_Pin_2
		#define BOOTBUTTON_PIN		  GPIO_IDR_IDR2
		#define BOOTBUTTON_PORT		  GPIOA
		#define RCC_APB2ENR_PORTEN_BOOT	 RCC_APB2ENR_IOPAEN
		#define RCC_APB2ENR_PORTEN_LED	 RCC_APB2ENR_IOPCEN
		#define GPIO_BSRR_BRx 			GPIO_BSRR_BR6
		#define GPIO_BSRR_BSx 			GPIO_BSRR_BS6
		#define GPIO_IDR_IDRX       GPIO_IDR_IDR6
#endif

#define RTC_GET 0
#define RTC_SET 1
#define TIM_GET 2
#define TIM_SET 3

typedef enum
{
	CAN_TXOK=0,
	CAN_TXERROR,
	CAN_TXBUSY
} CAN_TXSTATUS;

typedef enum
{
	CAN_RXOK=0,
	CAN_RXERROR,
} CAN_RXSTATUS;

typedef struct
{
	uint32_t ID;
	uint8_t DLC;
	uint8_t Data[8];
} CANTX_TypeDef; 

typedef struct
{
	uint16_t ID;
	uint8_t FMI;
	uint8_t DLC;
	uint8_t Data[8];
} CANRX_TypeDef; 

extern CANTX_TypeDef CAN_Data_TX;


void bxCAN_Init(void);
CAN_TXSTATUS CAN_Transmit_DataFrame(CANTX_TypeDef *Data);
CAN_TXSTATUS CAN_Transmit_RemoteFrame(uint16_t ID);
void CAN_Receive_IRQHandler(uint8_t FIFONumber);
void CAN_RXProcess0(void);
void CAN_RXProcess1(void);










#endif
