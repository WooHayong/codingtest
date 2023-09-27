//////////////////////////////////////////////////////////////////////////
// HW3.1 �µ�(��������)���� ��� Pulse �� ������ �߻�
// HW3.2 MCU ���� �µ� ���� �̿� MCU ���� �µ� ����
// ADC1 CH16
// ADC3 ,���� ��Ʈ TIM8_CH1(OC_mode)�̿�
//  150ms���� ADC ����, ADC3 module�� �̿�
// TIM14_CH1 Output Compare Mode �̿� PF9 ��� ����
// �ܺ� �µ� : �µ�(T)�� ����(V)������ �����: T = 3.5 V^2 + 1,���й���: 0.0~3.30, �µ�����: 1.0~39.1
//���� �µ� : Temperature (in ��C) = {(VSENSE - V25) / Avg_Slope} + 25,
// �µ���ȭ������ ���ļ��� ǥ���ϰ� ���ļ��� TIM14_CH1�� ���Ͽ� ���
// �̶� ���ļ��� 100 ������ ��� ���� ����ϰ� �Ͽ��� 
// ������: 2022. 11. 07
// ������ Ŭ����: ����Ϲ�
// �й�: 2018130026
// �̸�: ���Ͽ�
//////////////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"


#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15


void DisplayTitle(void);
void _GPIO_Init(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
//void BEEP(void);
uint16_t KEY_Scan(void);

void _ADC_Init(void);
void TIMER8_Init(void);
void TIMER14_Init(void);

double ADC1_Value,TEMP_VAL,VSENSE,ADC3_Value,Voltage,Temperature;        //�Ҽ��� ����� ���� DOUBLE�� �� ���� 
int Frequency; // FREQ ���� ����� ����
char FREQ[20]; // FREQ ��¹��ڿ�

void ADC_IRQHandler(void)
{
  //ADC3�� ���������� ���� ��� ���� �������� ���� -> ������ ���� �µ� ������ ��ȯ-> ���ǿ� ���߾� ȭ�鿡 ���÷���
  if (ADC3->SR && ADC_SR_EOC == ADC_SR_EOC){ // ADC3 EOC int
	ADC3->SR &= ~(1<<1);		// EOC flag clear  
	
	ADC3_Value = ADC3->DR;   	// DATA REG���� ���� ����  
	
	//�Ҽ��� �Ʒ� ���ڸ��� ������ �Ǳ� ������ x10
	Voltage =ADC3_Value * (3.3 * 10) / 4093 ;   //4093���� �ϸ� 3.3 , �µ� 39.1���� �����⿡ ����
	LCD_DisplayChar(1,8, (int)Voltage /10 + 0x30); //����ǥ��
	LCD_DisplayChar(1,9, '.'); 
	LCD_DisplayChar(1,10, (int)Voltage %10 + 0x30); //�Ҽ� ù°�ڸ�
	LCD_DisplayText(1,11,"0V"); 
	
	
	//�Ҽ��� �Ʒ� ���ڸ��� ������ �Ǳ� ������ Voltage���� *100�Ǿ����� /10 ����
	Temperature = (3.5 * Voltage * Voltage / 10 + 10);      //�µ� ���ϴ� ������ ���� �µ� ���� ��� ȭ�鿡 ���÷��� ���ش�.
	
	LCD_DisplayChar(2,8,(int)Temperature/100 + 0x30); 
	LCD_DisplayChar(2,9,(int)Temperature%100/10 + 0x30);
	LCD_DisplayChar(2,10, '.'); 
	LCD_DisplayChar(2,11,(int)Temperature%10 + 0x30); //�µ� ǥ��
	LCD_DisplayChar(2,12, 'C'); 
	
	sprintf(FREQ,"%4d",(int)Temperature*10);// FREQ= �µ� *100 -> �µ��� �̹� Voltage�� ���� 10 ����������           
	LCD_DisplayText(3,5,FREQ);
	LCD_DisplayText(3,9,"Hz");
	Frequency = (int)Temperature*10;
	LCD_SetBrushColor(RGB_YELLOW); 
	LCD_DrawFillRect(10,55,139,15);
	LCD_SetBrushColor(RGB_GREEN);
	LCD_DrawFillRect(10,55,0.0355*Frequency, 15);   // ���� �׸���	
  }
  if (ADC1->SR && ADC_SR_EOC == ADC_SR_EOC){ // ADC1 EOC int
	ADC1->SR &= ~(1<<1);		// EOC flag clear                         
	
	ADC1_Value = ADC1->DR;		// Reading ADC result      
	VSENSE = ((double)ADC1_Value * 3.3  / 4095);   //���а��� ���Ѵ�.     
	
	TEMP_VAL = (((VSENSE-0.76)/0.0025)+25)*10; //�µ� ���ϴ� ������ �̿�, *10�� ���־ �Ҽ��� ���ڸ� ���� ���ϵ���      
	LCD_DisplayChar(6,8,(int)TEMP_VAL/100 + 0x30);
	LCD_DisplayChar(6,9,(int)TEMP_VAL%100/10 + 0x30);
	LCD_DisplayChar(6,10, '.'); 
	
	LCD_DisplayChar(6,11,(int)TEMP_VAL%10 + 0x30); // ���� �µ� ǥ�� 
	LCD_DisplayChar(6,12, 'C'); 
  }
  
}
int main(void)
{
  LCD_Init();	// LCD ���� �Լ�
  DelayMS(10);		// LCD���� ������
  DisplayTitle();		//LCD �ʱ�ȭ�鱸�� �Լ�	
  _GPIO_Init();        
  TIMER8_Init();    //TIMER�ʱ�ȭ        
  _ADC_Init();        //while�� ���� ���� ���ִ� ���� ����,     
  TIMER14_Init();
  ADC1->CR2 |= ADC_CR2_SWSTART;	 //SW����
  while(1)
  {               
	switch((int)Frequency) //���ļ��� ���� ARR���� 10������ ��� ����
	{
	case 100 ... 199 : 
	  TIM14->ARR = 10000; break;
	case 200 ... 299 : 
	  TIM14->ARR = 5000; break;
	case 300 ... 399 : 
	  TIM14->ARR = 3333; break;
	case 400 ... 499 : 
	  TIM14->ARR = 2500; break;
	case 500 ... 599 : 
	  TIM14->ARR = 2500; break;
	case 600 ... 699 : 
	  TIM14->ARR = 1666; break;
	case 700 ... 799 : 
	  TIM14->ARR = 1428; break;
	case 800 ... 899 : 
	  TIM14->ARR = 1250; break;
	case 900 ... 999 : 
	  TIM14->ARR = 1111; break;
	case 1000 ... 1099 : 
	  TIM14->ARR = 1000; break;
	case 1100 ... 1199 : 
	  TIM14->ARR = 909; break;
	case 1200 ... 1299: 
	  TIM14->ARR = 833; break;
	case 1300 ... 1399 : 
	  TIM14->ARR = 769; break;
	case 1400 ... 1499 : 
	  TIM14->ARR = 714; break;
	case 1500 ... 1599 : 
	  TIM14->ARR = 666; break;
	case 1600 ... 1699 : 
	  TIM14->ARR = 625; break;
	case 1700 ... 1799 : 
	  TIM14->ARR = 588; break;
	case 1800 ... 1899 : 
	  TIM14->ARR = 555; break;
	case 1900 ... 1999 : 
	  TIM14->ARR = 526; break;
	case 2000 ... 2099 : 
	  TIM14->ARR = 500; break;
	case 2100 ... 2199 : 
	  TIM14->ARR = 476; break;
	case 2200 ... 2299 : 
	  TIM14->ARR = 455; break;
	case 2300 ... 2399 : 
	  TIM14->ARR = 435; break;
	case 2400 ... 2499: 
	  TIM14->ARR = 417; break;
	case 2500 ... 2599 : 
	  TIM14->ARR = 400; break;
	case 2600 ... 2699 : 
	  TIM14->ARR = 385; break;
	case 2700 ... 2799: 
	  TIM14->ARR = 370; break;
	case 2800 ... 2899 : 
	  TIM14->ARR = 357; break;
	case 2900 ... 2999 : 
	  TIM14->ARR = 345; break;
	case 3000 ... 3099 : 
	  TIM14->ARR = 333; break;
	case 3100 ... 3199 : 
	  TIM14->ARR = 323; break;
	case 3200 ... 3299: 
	  TIM14->ARR = 313; break;
	case 3300 ... 3399 : 
	  TIM14->ARR = 303; break;
	case 3400 ... 3499 : 
	  TIM14->ARR = 294; break;
	case 3500 ... 3599 : 
	  TIM14->ARR = 286; break;
	case 3600 ... 3699: 
	  TIM14->ARR = 278; break;
	case 3700 ... 3799 : 
	  TIM14->ARR = 270; break;
	case 3800 ... 3899 : 
	  TIM14->ARR = 263; break;
	case 3900 ... 3910 : 
	  TIM14->ARR = 256; break;   
	}

	switch(KEY_Scan())
	{
	case SW7_PUSH: // sW7 Push
	  TIM14->CR1 ^= (1<<0);	// CEN: Enable the Tim14 Counter  					
	  break;
	}
  }
}
void _ADC_Init(void)
{   	// ADC1, ADC3 �ʱ� ����        
  RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;	        // (1<<10) ENABLE ADC3 CLK (stm32f4xx.h ����)        
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;	// (1<<8) ENABLE ADC1 CLK (stm32f4xx.h ����)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   // (1<<0) ENABLE GPIOA CLK (stm32f4xx.h ����) �������׻��
  GPIOA->MODER |= (3<<2*1);      // CONFIG GPIOA PIN(PA1) TO ANALOG IN MODE
  
  ADC->CCR &= ~(0x1F<<0);		// MULTI[4:0]: ADC���: ADC_Mode_Independent
  ADC->CCR |=  (1<<16); 		        // ADCPRE:ADC_Prescaler_Div4 (ADC MAX Clock 36MHz, 84Mhz(APB2)/4 = 21MHz)
  ADC->CCR |=  (1<<23); 		        //�µ� ���� Enable
  
  //ADC1_IN16��  ���� �µ� ������ ���
  ADC1->CR1 |=  (1<<5);		// EOCIE=1: Interrupt enable for EOC                
  ADC1->CR1 &= ~(3<<24);		// RES[1:0]=0b00 : 12bit Resolution
  ADC1->CR1 &= ~(1<<8);		// SCAN=0 : ADC_ScanCovMode Disable	     
  ADC1->CR2 &= ~(1<<1);		// CONT=0: ADC_Continuous ConvMode Disable
  ADC1->CR2 &= ~(1<<10);	    // ��ȯ�� ������ EOC  �߻�                
  ADC1->CR2 &= ~(1<<11);		// ALIGN=0: ADC_DataAlign_Right
  ADC1->CR2 |= (2<<28);		// EXTEN[1:0]=0b00: ADC_ExternalTrigConvEdge		
  ADC1->SQR1 &= ~(0xF<<20);	// L[3:0]: ADC Regular channel sequece length_1conversion        
  ADC1->SQR3 |= (16<<0);   // �ѹ� ��ȯ-> 16CH�̱⿡ ���� ���� ����                 
  ADC1->SMPR1 |= (7<<18);	// ADC1_CH16 Sample Time_480Cycles (3*Channel_16) 	
  
  //ADC3: ���������� �ܺοµ� ������ ���, TIM8_CH1 ���
  ADC3->CR1 |=  (1<<5);		// EOCIE=1: Interrupt enable for EOC
  ADC3->CR1 &= ~(1<<8);		// SCAN=0 : ADC_ScanCovMode Disable	
  ADC3->CR1 &= ~(3<<24);		// RES[1:0]=0b00 : 12bit Resolution	
  ADC3->CR2 &= ~(1<<1);		// CONT=0: ADC_Continuous ConvMode Disable
  ADC3->CR2 |= (13<<24);    	// EXSEL[[3:0](27~24): ADC_External event select for regular group_Timer 8 CC1 event
  ADC3->CR2 |=  (2<<28);		// EXTEN[1:0]: ADC_ExternalTrigConvEdge_Enable(Falling Edge)	
  ADC3->CR2 &= ~(1<<11);		// ALIGN=0: ADC_DataAlign_Right
  ADC3->CR2 &= ~(1<<10);		// EOCS=0: The EOC bit is set at the end of each sequence of regular conversions
  ADC3->SQR1 &= ~(0xF<<20);	// L[3:0]: ADC Regular channel sequece length 					 	
  ADC3->SQR3 |= (1<<0);		// SQ1[4:0]=0b0001 : CH1
  ADC3->SMPR2 |= (7<<(3*1));	// ADC3_CH1 Sample TIme_480Cycles (3*Channel_1)
  
  NVIC->ISER[0] |= (1<<18);   //���ͷ�Ʈ NVIC����             
  
  ADC1->CR2 |= (1<<0);      // ADC1 ON
  ADC3->CR2 |= (1<<0);      // ADC3 ON
  
  
}
void TIMER8_Init(void)
{
  
  //TIM8_CH1 (PI5) : 150ms ���ͷ�Ʈ �߻�
  // Clock Enable : GPIOI 
  RCC->AHB1ENR |= (1<<8);	// GPIOI Enable
  RCC->APB2ENR |= (1<<1);	// TIMER8 Enable 
  
  //PI5�� ��¼����ϰ� Alternate function(TIM8_CH1)���� ��� ����
  GPIOI->MODER |= (1<<2*15);	// PI15 Output Alternate function mode				
  GPIOI->OSPEEDR |= (1<<2*15); // PI15 Output speed (100MHz High speed)
  GPIOI->OTYPER &= ~(1<<15); // PI15 Output type push-pull (reset state)
  GPIOI->AFR[1]|= (3<<28); // (AFR[1].()=0b0011): Connect TIM8 pins(PI15) to AF3(TIM8..11)
  // PI15 ==> TIM8_CH1                          
  
  //150mSec ���� ���ߴ� ����
  TIM8->PSC = 840-1;	// Prescaler 168MHz/840 = 0.2MHz (5us)
  TIM8->ARR = 30000-1;	// Auto reload  : 5us * 30K = 150ms(period)
  
  // CR1 : Up counting
  TIM8->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
  TIM8->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM8->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM8->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
  TIM8->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
  TIM8->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
  TIM8->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
  // Center-aligned mode: The counter counts Up and DOWN alternatively
  
  // Event & Interrup Enable : UI  
  TIM8->EGR |= (1<<0);    
  
  TIM8->BDTR |= (1<<15);
  
  
  // Define the corresponding pin by 'Output'  
  TIM8->CCER |= (1<<0);	// ��CC1E=1: CC1 channel Output Enable				
  TIM8->CCER &= ~(1<<1);	// ��CC1P=0: CC1 channel Output Polarity (OCPolarity_High : OC3���� �������� ���)  
  
  // 'Mode' Selection : Output mode, toggle  
  TIM8->CCMR1 &= ~(3<<0);         
  TIM8->CCMR1 &= ~(1<<3);	// OC1M=0b010: Output Compare 3 Mode : toggle				
  TIM8->CCMR1 |= (3<<4);	// OC1M=0b010: Output Compare 3 Mode : toggle				
  TIM8->CCR1 = 2000;	
  
  TIM8->CR1 |= (1<<0);	// CEN: Enable the Tim1 Counter  					
}

void TIMER14_Init(void)
{
  // TIM14_CH1 (PF9) : Out Compare Mode �̺�Ʈ �߻�
  // Clock Enable : GPIOF & TIMER14
  RCC->AHB1ENR	|= (1<<5);	// GPIOF Enable
  RCC->APB1ENR 	|= (1<<8);	// TIMER14 Enable 
  
  // PF9�� ��¼����ϰ� Alternate function(TIM14_CH1)���� ��� ���� 
  GPIOF->MODER 	|= (2<<2*9);	// PF9 Output Alternate function mode					
  GPIOF->OSPEEDR 	|= (3<<2*9);	// PF9 Output speed (100MHz High speed)
  GPIOF->OTYPER	&= ~(1<<9);	// PF9 Output type push-pull (reset state)
  GPIOF->PUPDR    |= (1<<18);
  GPIOF->AFR[1]	|= (9<<4); 	// 0x00000200	(AFR[1].(9)=0b1001): Connect TIM14 pins(PF9) to AF9(TIM12..14)
  // PF9 ==> TIM14_CH1
  // Assign 'Inxterrupt Period' and 'Output Pulse Period'
  TIM14->PSC = 84-1;	// Prescaler 84MHz/84 = 1MHz (1us)
  TIM14->ARR = 10000-1;	// Auto reload  : 1us * 10k = 10ms(100Hz)
  // 200Hz -> 5ms,ARR = 5k  300Hz->3.3333ms, ARR = 3k 
  // 400Hz -> 2.5ms , ARR = 2.5k  , 500Hz 0.02
  // CR1 : Up counting
  TIM14->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
  TIM14->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM14->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM14->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
  TIM14->CR1 |=  (1<<7);	// ARPE=1(ARR Preload Enable)
  TIM14->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
  TIM14->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
  // Center-aligned mode: The counter counts Up and DOWN alternatively
  
  
  
  
  ////////////////////////////////
  // Disable Tim5 Update interrupt
  
  // Define the corresponding pin by 'Output'  
  TIM14->CCER |= (1<<0);	// CC1E=1: CC1 channel Output Enable
  // OC1(TIM14_CH1) Active: �ش����� ���� ��ȣ���
  TIM14->CCER &= ~(1<<1);	// CC1P=0: CC1 channel Output Polarity (OCPolarity_High : OC1���� �������� ���)  
  TIM14->CCR1 = 0;	// TIM14 CCR1 TIM14_Pulse
  // 'Mode' Selection : Output mode, toggle  
  TIM14->CCMR1 &= ~(3<<0); 	// CC1S(CC1 channel)='0b00' : Output 
  TIM14->CCMR1 |= (1<<3); 	// OC1PE=1: Output Compare 1 preload Enable
  TIM14->CCMR1 |= (3<<4);
  TIM14->CCMR1 |= (1<<7);	// OC1CE: Output compare 1 Clear enable
  
  
  
  NVIC->ISER[1] |= (1 << (45 - 32)); 
  //TIM14->CR1 |= (1<<0);	// CEN: Enable the Tim14 Counter  					
}



void _GPIO_Init(void)
{
  // LED (GPIO G) ����
  RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
  GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
  GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
  GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
  
  // SW (GPIO H) ���� 
  
  RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							  
  GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				  
  GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
  
  ////  // Buzzer (GPIO F) ���� 
  //  RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
  //  GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
  //  GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
  //  GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
  
  //Joy Stick SW(PORT I) ����
  RCC->AHB1ENR 	|= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
  GPIOI->MODER 	&= ~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
  GPIOI->PUPDR    &= ~0x000FFC00;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)
}	

//void BEEP(void)			// Beep for 20 ms 
//{ 	
//  GPIOF->ODR |= (1<<9);	// PF9 'H' Buzzer on
//  DelayMS(20);		// Delay 20 ms
//  GPIOF->ODR &= ~(1<<9);	// PF9 'L' Buzzer off
//}

void DelayMS(unsigned short wMS)
{
  register unsigned short i;
  for (i=0; i<wMS; i++)
	DelayUS(1000);   // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
  volatile int Dly = (int)wUS*17;
  for(; Dly; Dly--);
}

void DisplayTitle(void)
{
  LCD_Clear(RGB_YELLOW);
  LCD_SetFont(&Gulim8);			//��Ʈ 
  LCD_SetBackColor(RGB_YELLOW);
  LCD_SetTextColor(RGB_BLACK);		//���ڻ�
  LCD_DisplayText(0,0,"2018130026 WHY");
  LCD_DisplayText(1,0,"EXT VOL:");	// Title
  LCD_DisplayText(2,0,"EXT TMP:");
  LCD_DisplayText(3,0,"FREQ:");
  LCD_DisplayText(6,0,"INT TMP:");
}

uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
  uint16_t key;
  key = GPIOH->IDR & 0xFF00;	// any key pressed ?
  if(key == 0xFF00)		// if no key, check key off
  {  	if(key_flag == 0)
	return key;
  else
  {	DelayMS(10);
  key_flag = 0;
  return key;
  }
  }
  else				// if key input, check continuous key
  {	if(key_flag != 0)	// if continuous key, treat as no key input
	return 0xFF00;
  else			// if new key,delay for debounce
  {	key_flag = 1;
  DelayMS(10);
  return key;
  }
  }
}
