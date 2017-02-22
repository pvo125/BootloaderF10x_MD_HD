#include <stm32f10x.h>
#include "CAN.h"


/**
  * @brief  This function handles USB_LP_CAN1_RX0_IRQHandler interrupt request.
  * @param  None
  * @retval None
  */
void USB_LP_CAN1_RX0_IRQHandler(void){
	
	CAN_Receive_IRQHandler(0);
	CAN_RXProcess0();
	
}


