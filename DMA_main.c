#include "stm32f4xx.h"
#include "GLCD.h"

void RunMenu(void);
void DMAInit(void);
void USART1_Config(void);
void USART_BRR_Configuration(uint32_t USART_BaudRate);
void GPIO_Config(void);

void SerialPutChar(uint8_t c);
void Serial_PutString(char *s);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint32_t str[20];

int main(void)
{
	DMAInit();
    GPIO_Config();
    USART1_Config();
	LCD_Init();					// LCD 구동 함수
	DelayMS(1000);				// LCD구동 딜레이
          
	//LCD 초기화면구동 함수
	RunMenu();
   
 	while(1)
	{
		LCD_DisplayChar(4,0,str[1]);
		LCD_DisplayChar(4,1,str[0]);

	}
}




void GPIO_Config(void)
{

    //USART1
    
    //TX(PA9)
    RCC->AHB1ENR	|= 0x01;					// RCC_AHB1ENR GPIOA Enable
    GPIOA->MODER 	|= 0x00080000;				// GPIOA PIN9 Output Alternate function mode					
    GPIOA->OSPEEDR |= 0x000C0000;				// GPIOA PIN9 Output speed (100MHz Very High speed)
    GPIOA->OTYPER	|= 0x00000000;				// GPIOA PIN9 Output type push-pull (reset state)
    GPIOA->PUPDR	|= 0x00040000;				// GPIOA PIN9 Pull-up
    
    GPIOA->AFR[1]	|= 0x70;					//Connect GPIOA pin9 to AF7(USART1)
    
    //RX(PA10)
    RCC->AHB1ENR 	|= 0x01;					// RCC_AHB1ENR GPIOA Enable    				   
    GPIOA->MODER 	|= 0x200000;				// GPIOA PIN10 Output Alternate function mode
    GPIOA->OSPEEDR |= 0x00300000;				// GPIOA PIN10 Output speed (100MHz Very High speed
    
    GPIOA->AFR[1]	|= 0x700;					//Connect GPIOA pin10 to AF7(USART1)
    
    //SW

    // LED GPIO 설정
    RCC->AHB1ENR	|= 0x40;  					// RCC_AHB1ENR GPIOG Enable		
    GPIOG->MODER 	|= 0x5555;					// GPIOG PIN0~PIN7 Output mode	
    GPIOG->OSPEEDR 	|= 0x5555;				// GPIOG PIN0~PIN7 Output speed (25MHZ Medium speed) 
    
    // SW GPIO 설정
    RCC->AHB1ENR 	|= 0x80;					// RCC_AHB1ENR GPIOH Enable
    GPIOH->MODER 	|= 0x00000000;				// GPIOH PIN8~PIN15 Input mode (reset state)				
 
}

void USART1_Config(void)
{
    RCC->APB2ENR		|=	0x0010;			// RCC_APB2ENR USART1 Enable
    
    USART_BRR_Configuration(9600);    				// USART Baudrate Configuration
    
    USART1->CR1		|=	0x0000;			// USART_WordLength 8 Data bit
    USART1->CR1		|=	0x0000;			// USART_Parity_No
    USART1->CR1		|=	0x0004;			// USART_Mode_RX Enable
    USART1->CR1		|=	0x0008;			// USART_Mode_Tx Enable
    USART1->CR2		|=	0x0000;			// USART_StopBits_1
    USART1->CR3		|=	0x00C0;			// USART_HardwareFlowControl_None;
    
    NVIC->ISER[1] 		&= 	( 1 << 5 ); 		// Enable Interrupt USART1

    USART1->CR1 		|= 	0x20;				// RXNE interrupt Enable
    USART1->CR1 		|=	0x2000;			// USART1 Enable   
}

void DMAInit(void)
{
    
	// DMA2 Stream0 channel2 configuration *************************************
  
	RCC->AHB1ENR |= (1<<22);                       //DMA2 clock enable
	DMA2_Stream2->CR	|= (4<<25);                 //DMA2 Stream0 channel 0 selected
	DMA2_Stream2->PAR	|= (uint32_t)&USART1->DR;    //Peripheral address - ADC1 Regular data Address
	DMA2_Stream2->M0AR |= (uint32_t)str;  //Memory address - ADC1 Value
	DMA2_Stream2->CR	&= ~(3<<6);		//Data transfer direction : Peripheral-to-memory
	DMA2_Stream2->NDTR = 2;			//DMA_BufferSize = 2

	DMA2_Stream2->CR	&= ~(1<<9); 		//Peripheral increment mode - Peripheral address pointer is fixed
	DMA2_Stream2->CR	|= (1<<10);		//Memory increment mode - Memory address pointer is incremented after each data transferd 
	DMA2_Stream2->CR	|= (3<<11);		//Peripheral data size - halfword(16bit)
	DMA2_Stream2->CR	|= (3<<13);		//Memory data size - word(16bit)   
	DMA2_Stream2->CR	|= (1<<8);			//Circular mode enabled   
	DMA2_Stream2->CR	|= (2<<16);		//Priority level - High

	DMA2_Stream2->FCR &= ~(1<<2);		//DMA_FIFO_direct mode enabled
	DMA2_Stream2->FCR |= (1<<0);			//DMA_FIFO Threshold_HalfFull , Not used in direct mode

	DMA2_Stream2->CR 	&= ~(3<<23);		//Memory burst transfer configuration - single transfer
	DMA2_Stream2->CR 	&= ~(3<<21);		//Peripheral burst transfer configuration - single transfer  
	DMA2_Stream2->CR	|= (1<<0);			//DMA2_Stream0 enabled
}
void SerialPutChar(uint8_t c) // 1문자 보내기 함수
{
    while((USART1->SR & 0x0080) == RESET); 		// 송신 가능한 상태까지 대기

    USART1->DR = (c & (uint16_t)0x01FF);			// 전송
}


void Serial_PutString(char *s) // 여러문자 보내기 함수
{
    while (*s != '\0') // 종결문자가 나오기 전까지 구동. 종결문자가 나온후에도 구동시 메모리 오류 발생가능성 있음.
    {
        SerialPutChar(*s); // 포인터가 가르키는 곳의 데이터를 송신
        s++; // 포인터 수치 증가
    }
}

void USART_BRR_Configuration(uint32_t USART_BaudRate)
{ 
    uint32_t tmpreg = 0x00;
    uint32_t apbclock = 84000000;					//PCLK2_Frequency
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;

    /* Determine the integer part */
    if ((USART1->CR1 & 0x8000) != 0)
    {
	/* Integer part computing in case Oversampling mode is 8 Samples */
	integerdivider = ((25 * apbclock) / (2 * (USART_BaudRate)));    
    }
  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
  {
      /* Integer part computing in case Oversampling mode is 16 Samples */
      integerdivider = ((25 * apbclock) / (4 * (USART_BaudRate)));    
  }
  tmpreg = (integerdivider / 100) << 4;
  
  /* Determine the fractional part */
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

  /* Implement the fractional part in the register */
  if ((USART1->CR1 & USART_CR1_OVER8) != 0)
  {
    tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
  }
  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
  {
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
  }
    /* Write to USART BRR register */
  USART1->BRR = (uint16_t)tmpreg;
    
}

void USART1_IRQHandler(void)
{
    if ((USART1->CR1 & 0x20) && (USART1->SR & 0x20))
    {
	}
}
void RunMenu(void)
{
	LCD_Clear(RGB_WHITE);
	LCD_SetFont(&Gulim8);
	LCD_SetBackColor(RGB_GREEN);
	LCD_SetTextColor(RGB_BLACK);
 	LCD_DisplayText(0,0,"ADC");

	LCD_SetBackColor(RGB_WHITE);		//글자배경색
	LCD_SetTextColor(RGB_RED);

	LCD_DisplayText(1,0,"ADC 12bit");
	LCD_DisplayText(2,0,"REF 3.3V");

}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;

	for (i=0; i<wMS; i++)
		DelayUS(1000);         	// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
	for(; Dly; Dly--);
}

