#include <stm32f10x.h>
#include "CAN.h"

#define FLASHBOOT_SIZE				0x2800		// 8 kB boot  + 2 kb flag_status
#define FLASHBOOT_PAGE		0x08000000		// page 0
#define FLAG_STATUS_PAGE	0x08002000		// page 1 	
#define FIRM_WORK_PAGE 		0x08002800		// page 10 for MD or 5 for HD		firmware work base



#ifdef MEDIUM_DENSITY					// if medium density
	#define FIRM_UPD_PAGE    		0x08010000		// page 64		firmware update base for MD
#else													// if high density
	#define FIRM_UPD_PAGE 		  0x08040000		// page 128		firmware update base for HD
#endif


#ifdef MEDIUM_DENSITY
	#define NETNAME_INDEX  03   //32VLDisc
#else
	#define NETNAME_INDEX  02   //F103_KIT
#endif

volatile uint8_t get_firmware_size;

extern volatile int size_firmware;
extern volatile uint8_t write_flashflag;

void assert_failed(uint8_t* file, uint32_t line)
{
	while(1){};
}

const uint32_t crc32_table[256]=
{	
		0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

/**********************************************************************************************
*																	CRC32 checksum
***********************************************************************************************/
uint32_t crc32_check(const uint8_t *buff,uint32_t nbytes){

	uint32_t crc=0xffffffff;
	while(nbytes--)
		crc=(crc>>8)^crc32_table[(crc^*buff++) & 0xFF];

	return crc^0xffffffff;

}
/**********************************************************************************************
*																	Flash_unlock
***********************************************************************************************/
void Flash_unlock(void){

	
	FLASH->KEYR=0x45670123;
	FLASH->KEYR=0xCDEF89AB;
}
/**********************************************************************************************
*																	Flash_sect_erase
***********************************************************************************************/
void Flash_page_erase(uint32_t address,uint8_t countpage){
	uint8_t i;
		
	while((FLASH->SR & FLASH_SR_BSY)==FLASH_SR_BSY) {}
	if(FLASH->SR & FLASH_SR_EOP)	// если EOP установлен в 1 значит erase/program complete
		FLASH->SR=FLASH_SR_EOP;		// сбросим его для следующей индикации записи
	
	FLASH->CR |=FLASH_CR_EOPIE;					// включим прерывание для индикации флага EOP 
	
	FLASH->CR |=FLASH_CR_PER;						// флаг  очистки сектора
	for(i=0;i<countpage;i++)
		{
#ifdef MEDIUM_DENSITY	
			FLASH->AR=address+i*1024;
#else
			FLASH->AR=address+i*2048;
#endif			
			FLASH->CR |=FLASH_CR_STRT;														// запуск очистки заданного сектора
			while((FLASH->SR & FLASH_SR_EOP)!=FLASH_SR_EOP) {}		// ожидание готовности
			FLASH->SR=FLASH_SR_EOP;	
		}
	FLASH->CR &= ~FLASH_CR_PER;																	// сбросим  флаг  очистки сектора
}
/**********************************************************************************************
*																	Flash_prog
***********************************************************************************************/
void Flash_prog(uint16_t * src,uint16_t * dst,uint32_t nbyte){
	uint32_t i;
	while((FLASH->SR & FLASH_SR_BSY)==FLASH_SR_BSY) {}
	if(FLASH->SR & FLASH_SR_EOP)	// если EOP установлен в 1 значит erase/program complete
		FLASH->SR=FLASH_SR_EOP;		// сбросим его для следующей индикации записи
	
	FLASH->CR |=FLASH_CR_EOPIE;					// включим прерывание для индикации флага EOP 
	
	FLASH->CR |=FLASH_CR_PG;	
	for(i=0;i<nbyte/2;i++)
		{
				*(uint16_t*)(dst+i)=*(uint16_t*)(src+i);
				while((FLASH->SR & FLASH_SR_EOP)!=FLASH_SR_EOP) {}
				FLASH->SR=FLASH_SR_EOP;	
		}
	
	FLASH->CR &= ~FLASH_CR_PG;

}


/**********************************************************************************************
*			Функция проверки флэш на стертость возвращает 0 если флэш стерта и 1 если проверка на чистоту не прошла
***********************************************************************************************/
uint8_t checkflash_erase(uint32_t start_addr,uint32_t byte){
	uint32_t i=0;
	
	while((*(uint32_t*)(start_addr+i))==0xFFFFFFFF)
	{
		i+=4;
		if(i>=byte)
			return 0;
	}
	return 1;
}

/**********************************************************************************************
*			Функция  ожидания прошивки и принятия  ее по CAN сети
***********************************************************************************************/
void Bootloader_upd_firmware(uint16_t countflag){

	uint8_t flag,n;
	GPIO_InitTypeDef GPIO_InitStruct;
	
		bxCAN_Init();
	
	// PC9(MEDIUM_DENSITY ) PC6 выход push-pull без подтяжки для моргания  светодиодом 
		RCC->APB2ENR|=RCC_APB2ENR_PORTEN_LED;
	
		GPIO_InitStruct.GPIO_Pin=LEDPIN;
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
		GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
		GPIO_Init(LEDPORT,&GPIO_InitStruct);	
			
		get_firmware_size=1;						// взводим флаг и ждем когда master пришдет свой запрос с данными размера прошивки 
	
		while(get_firmware_size) 
		{			
			// отправляем запрос по сети на выдачу прошивки
			
			CAN_Data_TX.ID=(NETNAME_INDEX<<8)|0x74;						// 0x174 GET_FIRMWARE			
			CAN_Data_TX.DLC=1;
			CAN_Data_TX.Data[0]=NETNAME_INDEX;
			CAN_Transmit_DataFrame(&CAN_Data_TX);
			
			if(LEDPORT->IDR & GPIO_IDR_IDRX)
				LEDPORT->BSRR=GPIO_BSRR_BRx;
			else
				LEDPORT->BSRR=GPIO_BSRR_BSx;
			// Пауза 1,5 секунды
			SysTick->LOAD=(2500000*4);
			SysTick->VAL=0;
			while(!(SysTick->CTRL&SysTick_CTRL_COUNTFLAG_Msk)){}
		}
		// Если вышли из while значит пришел ответ и в прерывании было принято size_firmware и  установлено get_firmware_size=0
		LEDPORT->BSRR=GPIO_BSRR_BRx;				// Гасим светодиод
		
		// Подготовим флэш для принятия данных и записи
		Flash_unlock();
		// Проверка flash на стертость
		if(checkflash_erase(FIRM_WORK_PAGE,size_firmware))	
			{	// Если секторы не чистые (0xFF) 
#ifdef MEDIUM_DENSITY
		n=size_firmware/1024;
		if(size_firmware%1024)
			n++;	
#else
		n=size_firmware/2048;
		if(size_firmware%2048)
			n++;	
#endif			
				Flash_page_erase(FIRM_WORK_PAGE,n);		// Очистим n секторов 
			}
		// Отправим  первый запрос на принятия данных. Последующие запросы будем отправлять 
		//в ISR когда принимаем очередную порцию из 8 байт	
		CAN_Data_TX.ID=(NETNAME_INDEX<<8)|0x72;
		CAN_Data_TX.DLC=2;
		CAN_Data_TX.Data[0]=NETNAME_INDEX;
		CAN_Data_TX.Data[1]='g';								// GET_DATA!
		CAN_Transmit_DataFrame(&CAN_Data_TX);
		
	// Ожидаем когда вся прошивка примется по сети и в ISR после проверки crc32 установится 	write_flashflag=1
		while(write_flashflag==0) 
		{
		}
			
		// Запишем в сектор FLAG_STATUS флаг 0x00A3 по адресу  (FLAG_STATUS_PAGE+count) стирать предварительно не будем так как там 0xFFFF
			flag=0x00A3;	
			Flash_prog((uint16_t*)&flag,(uint16_t*)(FLAG_STATUS_PAGE+countflag),2);	// 2 байта 
			// Сделаем RESET 
			SysTick->LOAD=(2500000*4);
			SysTick->VAL=0;
			while(!(SysTick->CTRL&SysTick_CTRL_COUNTFLAG_Msk)){}
			NVIC_SystemReset();


}
/*
*/
int main (void) {
	uint32_t bin_size,crc;
	uint16_t flag;
	uint16_t count;
	uint8_t temp;
	GPIO_InitTypeDef 							GPIO_InitStruct;
 void(*pApplication)(void);		// указатель на функцию запуска приложения
	
	/*
	0x0800 0000 - 0x0800 1FFF   загрузчик						0 1 2 3 4 5 6 7 page flash for MD ( 0 1 2 3 page flash for HD)
	0x0800 2000 - 0x0800 27FF		page FLAG_STATUS	2 page for MD  or 1 page for HD	
#ifdef MEDIUM_DENSITY
	0x0800 2800 - 0x0800 FFFF		firmware work 	( max  0xD800  54*1024 B)
#else
	0x0800 2800 - 0x0803 FFFF		firmware work 	( max 0x3D800 246*1024 B)
#endif	

#ifdef MEDIUM_DENSITY
	0x0801 0000 - 0x0801 FFFF		firmware update ( max 0x10000 64*1024 B)
#else
	0x0804 0000 - 0x0807 FFFF		firmware update ( max 0x40000 256*1024 B)
#endif
*/	
	/*
		Загрузчик должен проверить флаг в секторе FLAG_STATUS_SECTOR во флэши.  
		Если установлен в 0x00A7:
		{	
			* по адресу FIRM_UPD_PAGE+0x1C считаем 4 байта размера прошивки bin_size
			* * проверить crc принятой обновленной прошивки во второй половине флэш сравнить со значениемпо адресу  FIRM_UPD_PAGE+bin_size
			*	переписать со второй половины флэш в первую(рабочую) 
			*  записать в сектор FLAG_STATUS_PAGE  флаг 0x00A3 следующим за считанным  0x00A7(в чистом месте 0xFFFF)
			* сделать reset для запуска обновленной прошивки из первой половины флэш.
		}
		Если установлен в 0x00A3:
		{
			* По адресу FIRM_WORK_PAGE+0x1C считаем 4 байта размера прошивки bin_size
			* Проверить crc прошивки и сравнить со значение которое лежит по адресу FIRM_WORK_PAGE+bin_size
			* если crc верное 
				* Передвинуть таблицу векторов на FLASH_BASE+FLASHBOOT_SIZE 		(0x08000000+0x2800)
        * Установить указатель стэка SP значением с адреса FIRM_WORK_PAGE
        * Запустить функцию (приложение ) по адресу взятому с (FIRM_WORK_PAGE+4)
			* если crc не верное запустить функцию Bootloader_upd_firmware() которая ожидает прошивку и принимает  ее по CAN сети
			
		}
		Если флаг 0xFFFF или другое любое значение кроме 0x00A3 0x00A7  также запускается Bootloader_upd_firmware()
		{	
			* Настроить периферию для работы CAN модуля.
			* Ожидать приема прошивки по CAN сети.
			* Записать принять прошивку по сети в рабочую часть флэш
			*	проверить crc обновленной прошивки в первой половине флэш
			* записать в сектор FLAG_STATUS_PAGE флаг 0x00A3 
			* сделать reset для запуска обновленной прошивки из первой половины флэш
		}
		
		*/
	
	/*Настройка вывода BOOT_PIN для принудительного запуска обновления из бутлоадера Bootloader_upd_firmware()	*/
	RCC->APB2ENR|=RCC_APB2ENR_PORTEN_BOOT;
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin=BOOT_PIN;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(BOOTBUTTON_PORT,&GPIO_InitStruct);

	
	
	// Настроим SysTick сбросим флаг CLKSOURCE выберем источник тактирования AHB/8
		SysTick->CTRL &=~SysTick_CTRL_CLKSOURCE_Msk;
		SysTick->CTRL |=SysTick_CTRL_ENABLE_Msk;
		SysTick->LOAD=(2500000*6);
		
	
	// проверим флаг  в секторе FLAG_STATUS во флэш.
	count=0;
	while(*(uint16_t*)(FLAG_STATUS_PAGE+count)!=0xFFFF)		// Перебираем байты пока не дойдем до неписанного поля 0xFF 
	{
		count+=2;
  	if(count>=2048)
		{
			count=0;
			// Для доступа к FLASH->CR	
			Flash_unlock();
#ifdef MEDIUM_DENSITY			
		Flash_page_erase(FLAG_STATUS_PAGE,2);
#else	
		Flash_page_erase(FLAG_STATUS_PAGE,1);
#endif				
			break;
		}
	}
	
	/* Если при включении зажата кнопка на BOOT_PIN то после паузы с проверкой на помеху запук функции обновления Bootloader_upd_firmware()
***********************************************************************************************************************************************************/	
	SysTick->VAL=0;

	if(!(BOOTBUTTON_PORT->IDR&BOOTBUTTON_PIN))
	{
		while(!(SysTick->CTRL&SysTick_CTRL_COUNTFLAG_Msk)){}
		if(!(BOOTBUTTON_PORT->IDR & BOOTBUTTON_PIN))
			Bootloader_upd_firmware(count);	
	}
		
/*
***********************************************************************************************************************************************************/	
	if(count==0)
		flag=*(uint16_t*)(FLAG_STATUS_PAGE);   // значение по адресу (FLAG_STATUS_SECTOR) // Читаем значение флага на 1 адресс меньше чистого поля.
	else
		flag=*(uint16_t*)(FLAG_STATUS_PAGE+count-2);   // значение по адресу (FLAG_STATUS_SECTOR+count-2) // Читаем значение флага на 1 адресс меньше чистого поля.
	
	if(flag==0x00A7)	
	{		// установлен флаг обновления firmware равный 0xA7
		// по адресу FIRM_UPD_PAGE+0x1C считаем 4 байта размера прошивки 
		bin_size=*((uint32_t*)(FIRM_UPD_PAGE+0x1C));
		
		crc=crc32_check((const uint8_t*)FIRM_UPD_PAGE,bin_size);
		/* Сравниваем полученный crc c тем что пришел с файлом прошивки */
		if(*(uint32_t*)(FIRM_UPD_PAGE+bin_size)==crc)
		{
			//Копируем проверенную прошивку в рабочую часть flash  по адресу FIRM_WORK_PAGE 
				// Подготовим flash для стирания и программирования 
			 // Для доступа к FLASH->CR	
				Flash_unlock();
			// Стирание секторов для записи  в рабочую часть флэш 	
			
#ifdef MEDIUM_DENSITY
		temp=(bin_size+4)/1024;
		if((bin_size+4)%1024)
			temp++;
#else
		temp=(bin_size+4)/2048;
		if((bin_size+4)%2048)
			temp++;
#endif					
				Flash_page_erase(FIRM_WORK_PAGE,temp);
			 // Запись обновленной прошивки с адреса FIRM_UPD_SECTOR в FIRM_WORK_SECTOR 
				Flash_prog((uint16_t*)FIRM_UPD_PAGE,(uint16_t*)FIRM_WORK_PAGE,(bin_size+4));	
			 
			// Запишем в сектор FLAG_STATUS флаг 0x00A3 по адресу  (FLAG_STATUS_PAGE+count) стирать предварительно не будем так как там 0xFFFF
			flag=0x00A3;	
			Flash_prog((uint16_t*)&flag,(uint16_t*)(FLAG_STATUS_PAGE+count),2);	// 2 байта 0x00FF
		// Сделаем RESET 
			NVIC_SystemReset();
		}	
	}
	else if(flag==0x00A3)						 
	{	// установлен флаг 0x00A3 запуск приложения		
		// по адресу FIRM_WORK_PAGE+0x1C считаем 4 байта размера прошивки 
		bin_size=*((uint32_t*)(FIRM_WORK_PAGE+0x1C));
		// Дополнительная проверка bin_size чтобы не получить HardFault на функции crc32_check()
#ifdef MEDIUM_DENSITY		
		if((bin_size>0)&&(bin_size<0x020000))
				crc=crc32_check((const uint8_t*)FIRM_WORK_PAGE,bin_size);
#else
		if((bin_size>0)&&(bin_size<0x080000))
			crc=crc32_check((const uint8_t*)FIRM_WORK_PAGE,bin_size);
#endif			
			
		/* Сравниваем полученный crc с рабочего сектора  c тем что лежит FIRM_WORK_PAGE+bin_size  */
		if(*(uint32_t*)(FIRM_WORK_PAGE+bin_size)==crc)
			{	// Если прошивка цела делаем запуск pApplication() с рабочего сектора
				// Передвинем таблицу векторов на FLASHBOOT_SIZE 	
				SCB->VTOR=FLASH_BASE|FLASHBOOT_SIZE;
				pApplication=(void(*)(void))*(__IO uint32_t*)(FIRM_WORK_PAGE+4);
				__set_MSP(*(__IO uint32_t*)FIRM_WORK_PAGE);
				
				pApplication();
			}
		else
			{// Иначе настраиваем интерфейс CAN и ждем прошивку по сети.
				Bootloader_upd_firmware(count);
			}
	}
	else
	{
		//Если CRC прошивки не совпадает с подсчитанной настраиваем интерфейс CAN и ждем прошивку по сети.
		Bootloader_upd_firmware(count);
		
	}
	
	while(1){}
}


