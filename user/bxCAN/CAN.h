#ifndef _CAN_H_
#define _CAN_H_

#include <stm32f10x.h>


#define MEDIUM_DENSITY

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
