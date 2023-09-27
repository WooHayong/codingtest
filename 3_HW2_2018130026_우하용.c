//////////////////////////////////////////////////////////////
// ������:  ������ �ð� ����(StopWatch ��� ǥ��)
// ��������: Timer3�� �����ϴ� �ð��ɰ� 
//               Timer4_OC Mode�� �����ϴ� StopWatch���.
//              Timer3 �� PSC Update Interrupt(Overflow) period: 100ms
//              Timer4 �� CC3 Interrupt(CC3I) period: 100ms           
// ����� �ϵ����(���): GPIO,  EXTI, GLCD, Key-Scan, LED, Buzzer, Timer 
// ������: 2022. 10. 05
// ������ Ŭ����: ����Ϲ�
// �й�: 2018130026
// �̸�: ���Ͽ�
///////////////////////////////////////////////////////////////
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

void _GPIO_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);
void TIMER3_Init(void);         // Timer3 (Counter)
void TIMER4_OC_Init(void);	// General-purpose Timer 4 (Output Compare mode)
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);

 static int stopmili=0;
  static int stopsec=0;
  static int stopsec_10=0; //stop watch ���� �ʱ�ȭ
  
int main(void)
{
	_GPIO_Init();
	_EXTI_Init();
	LCD_Init();	
	DelayMS(10);
	BEEP();
   	DisplayInitScreen();	// LCD �ʱ�ȭ��
	GPIOG->ODR &= 0xFF00;// �ʱⰪ: LED0~7 Off

	TIMER4_OC_Init();	// TIM4 Init (Output Compare mode: UI & CCI �߻�, )                  
        TIMER3_Init();		// ����Ÿ�̸�(TIM3) �ʱ�ȭ : up counting mode

	while(1)
	{
	}
}
void TIMER3_Init(void)
{
	RCC->APB1ENR |= 0x02;	// RCC_APB1ENR TIMER3 Enable

	// Setting CR1 : 0x0000 
	TIM3->CR1 &= ~(1<<4);  // DIR=0(Up counter)(reset state)
	TIM3->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
                            //  Counter Overflow/Underflow, 
                            //  Setting the UG bit Set,
                            //  Update Generation through the slave mode controller 
                            // UDIS=1 : Only Update event Enabled by  Counter Overflow/Underflow,
	TIM3->CR1 &= ~(1<<2);	// URS=0(Update Request Source  Selection):  By one of following events
                            //	Counter Overflow/Underflow, 
                            // Setting the UG bit Set,
                            //	Update Generation through the slave mode controller 
                            // URS=1 : Only Update Interrupt generated  By  Counter Overflow/Underflow,
	TIM3->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM3->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM3->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM3->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
                            // Center-aligned mode: The counter counts UP and DOWN alternatively


    // Deciding the Period
	TIM3->PSC = 840-1;	// Prescaler 84,000,000Hz/840 = 100,000 Hz (0.01ms)  (1~65536)
	TIM3->ARR = 10000-1;	// Auto reload  0.1ms * 10000 = 100ms

   	// Clear the Counter
	TIM3->EGR |= (1<<0);	// UG(Update generation)=1 
                        // Re-initialize the counter(CNT=0) & generates an update of registers   

	// Setting an UI(UEV) Interrupt 
	NVIC->ISER[0] |= (1<<29); // Enable Timer3 global Interrupt
 	TIM3->DIER |= (1<<0);	// Enable the Tim3 Update interrupt

	TIM3->CR1 |= (1<<0);	// Enable the Tim3 Counter (clock enable)   
}

void TIM3_IRQHandler(void)  	// 100ms Interrupt
{
  // �ʱ⼳�� 
  static int milisec=0; 
  static int sec=7;
  static int sec_10=0;  
  //AM, PM ������
  static int Timeflag=0;
  
  TIM3->SR &= ~(1<<0);	// Interrupt flag Clear
  
  if(Timeflag==0){   //PM flag
   
    milisec++;		 // 0.1sec ����
    LCD_DisplayChar(2,12,0x30+milisec);        //  0.1�� secǥ��
    
    if(milisec>=10)	// LED3 Toggle 500ms
    {	
      milisec = 0;       // 0���� �ʱ�ȭ
      sec++;               // 1�� ����
      LCD_DisplayChar(2,12,0x30+milisec);        //  0.1�� secǥ��  
      LCD_DisplayChar(2,10, 0x30+sec);        // �� ����  ǥ�� 
    }
    
    if( sec >= 10)
    {
      sec=0;  // 0���� �ʱ�ȭ 
      sec_10++;  // 10�� ��������
      LCD_DisplayChar(2,10, 0x30+sec);        // �� ����
      LCD_DisplayChar(2,9, 0x30+sec_10);        //  10����ǥ��
    }
    if(sec_10 == 1 && sec==1 && milisec ==9)  //PM��� 11:9 ���� AM��ȯ
    {
      Timeflag = 1;
      sec_10=0; sec=0; milisec=0;
      LCD_DisplayText(2,6,"AM");        // ����PM
      LCD_DisplayChar(2,9, 0x30+sec_10);        //  10����ǥ��
      LCD_DisplayChar(2,10, 0x30+sec);        // �� ����
      LCD_DisplayChar(2,12,0x30+milisec);        //  0.1�� secǥ��

    } 
  }
  else if(Timeflag == 1){  //AM �϶�
        milisec++;		  //0.1sec ����
      LCD_DisplayChar(2,12,0x30+milisec);        //  0.1�� secǥ��
    
    if(milisec>=10)	 
    {	
      milisec = 0;    //���� �ʱ�ȭ 
      sec++;            // 1������
      LCD_DisplayChar(2,12,0x30+milisec);        //  0.1�� secǥ��
      LCD_DisplayChar(2,10, 0x30+sec);        // �� ����
    }
    
    if( sec >= 10)
    {
      sec=0;  // 1������ 
      sec_10++;   // 10�� ���� ����
      LCD_DisplayChar(2,10, 0x30+sec);        // �� ����
      LCD_DisplayChar(2,9, 0x30+sec_10);        //  10����ǥ��
    }
    
    if(sec_10 == 1 && sec==1 && milisec ==9)  // 11:9 ���Ŀ� PM���� ���� 
      LCD_DisplayText(2,6,"PM");        // ����PM
    if(sec_10 == 1 && sec==2 && milisec ==9) // 12:9����  �ٽ� 01:0 �� ����
    {
      Timeflag=0;
      sec_10=0; sec=1; milisec=0;   
        LCD_DisplayChar(2,9, 0x30+sec_10);        //  10����ǥ��
      LCD_DisplayChar(2,10, 0x30+sec);        // �� ����
      LCD_DisplayChar(2,12,0x30+milisec);        //  0.1�� secǥ��
    }
  }  
}

void TIMER4_OC_Init(void)
{
// PB8: TIM4_CH3
// PB8�� ��¼����ϰ� Alternate function(TIM4_CH3)���� ��� ����
	RCC->AHB1ENR	|= (1<<1);	// 0x08, RCC_AHB1ENR GPIOB Enable : AHB1ENR.1

	GPIOB->MODER    |= (2<<16);	// 0x00020000(MODER.(17,16)=0b10), GPIOB PIN8 Output Alternate function mode 					
	GPIOB->OSPEEDR 	|= (3<<16);	// 0x00030000(OSPEEDER.(17,16)=0b11), GPIOB PIN8 Output speed (100MHz High speed)
	GPIOB->OTYPER	&= ~(1<<8);	// ~0x0100, GPIOB PIN8  Output type push-pull (reset state)
	GPIOB->PUPDR    |= (1<<16); 	// 0x10000000, GPIOB PIN8  Pull-up
  					// PB8 ==> TIM4_CH3
	GPIOD->AFR[1]	|= 0x00000002;  // (AFR[1].(19~16)=0b0010): Connect TIM4 pins(PB8) to AF2(TIM3..5)
 
// Time base ����
	RCC->APB1ENR |= (1<<2);	// 0x04, RCC_APB1ENR TIMER4 Enable
        
        
	// Setting CR1 : 0x0000 
	TIM4->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
	TIM4->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
                            //  Counter Overflow/Underflow, 
                            //  Setting the UG bit Set,
                            //  Update Generation through the slave mode controller 
                            // UDIS=1 : Only Update event Enabled by  Counter Overflow/Underflow,
	TIM4->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
                            //	Counter Overflow/Underflow, 
                            // Setting the UG bit Set,
                            //	Update Generation through the slave mode controller 
                            // URS=1 : Only Update Interrupt generated  By  Counter Overflow/Underflow,
	TIM4->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM4->CR1 |= (1<<7);	        // ARPE=1(ARR is buffered): ARR Preload Enalbe 
	TIM4->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM4->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 : Edge-aligned mode(reset state)

	// Setting the Period
	TIM4->PSC = 16800-1;	// Prescaler=16800 , 84MHz/16800 = 5000Hz (0.2ms)
	TIM4->ARR = 500-1;	// Auto reload  : 0.2ms * 500 =  100ms(period) : ���ͷ�Ʈ�ֱ⳪ ��½�ȣ�� �ֱ� ����

	// Update(Clear) the Counterfl
	TIM4->EGR |= (1<<0);    // UG: Update generation    

// Output Compare ����
       // TIM4->CCMR1 &= ~0xFFFF;
	// CCMR2(Capture/Compare Mode Register 2) : Setting the MODE of Ch3 or Ch4
	TIM4->CCMR2 &= ~(3<<0); // CC3S(CC3 channel) = '0b00' : Output 
	TIM4->CCMR2 &= ~(1<<2); // OC3FE=0: Output Compare 1 Fast disable 
	TIM4->CCMR2 &= ~(1<<3); // OC3PE=0: Output Compare 1 preload disable(CCR3�� �������� ���ο� ���� loading ����) 
	TIM4->CCMR2 |= (3<<4);	// OC3M=0b011 (Output Compare 1 Mode : toggle)
				// OC3REF toggles when CNT = CCR1
				
	// CCER(Capture/Compare Enable Register) : Enable "Channel 1" 
	TIM4->CCER |= (1<<8);	// CC3E=1: CC3 channel Output Enable
				// OC1(TIM4_CH3) Active: �ش���(167��)�� ���� ��ȣ���
	TIM4->CCER &= ~(1<<9);	// CC3P=0: CC3 channel Output Polarity (OCPolarity_High : OC1���� �������� ���)  

	// CC3I(CC ���ͷ�Ʈ) ���ͷ�Ʈ �߻��ð� �Ǵ� ��ȣ��ȭ(���)�ñ� ����: ��ȣ�� ����(phase) ����
	// ���ͷ�Ʈ �߻��ð�(500 �޽�)�� 100%(500) �ð����� compare match �߻�
	TIM4->CCR3 = 500;	// TIM4 CCR3 TIM4_Pulse

	TIM4->DIER |= (1<<3);	// CC3IE: Enable the Tim4 CC3 interrupt
	 NVIC->ISER[0] 	|= (1<<30);	// Enable Timer4 global Interrupt on NVIC

	//TIM4->CR1 |= (1<<0);	// CEN: Enable the Tim4 Counter  					
}

void TIM4_IRQHandler(void)      //RESET: 0
{
 
  if((TIM4->SR & 0x08) != RESET)  {
		TIM4->SR &= ~(1<<3);	// CC 3 Interrupt Claer
             
                stopmili++;		
                LCD_DisplayChar(3,7,0x30+stopmili);        //  0.1�� secǥ��
              
                if(stopmili>=10)	// LED3 Toggle 500ms
                {	
                  stopmili = 0;     
                  stopsec++;
                  LCD_DisplayChar(3,7,0x30+stopmili);        //  0.1�� secǥ��  
                  LCD_DisplayChar(3,5, 0x30+stopsec);        // �� ����   
                }
                
                if( stopsec >= 10)
                {
                  stopsec=0;
                  stopsec_10++;
                  LCD_DisplayChar(3,5, 0x30+stopsec);        // �� ����
                  LCD_DisplayChar(3,4, 0x30+stopsec_10);        //  10����ǥ��
                }
                
                if(stopsec_10 == 9 && stopsec==9 && stopmili ==9) // 99:9 ���� 00:0���� �ʱ�ȭ
                {
                  stopsec_10=0; stopsec=0; stopmili=0;
                  LCD_DisplayChar(3,4, 0x30+stopsec_10);        //  10����ǥ��
                  LCD_DisplayChar(3,5, 0x30+stopsec);        // �� ����
                  LCD_DisplayChar(3,7,0x30+stopmili);        //  0.1�� secǥ��
                }
  }
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

	// Buzzer (GPIO F) ���� 
    	RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
}	

void _EXTI_Init(void)
{
    	RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
	
	GPIOH->MODER 	&= 0x0000FFFF;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
	
	SYSCFG->EXTICR[3] |= 0x0077; 	// EXTI12,13�� ���� �ҽ� �Է��� GPIOH�� ���� (EXTICR4) (reset value: 0x0000)	
	
	EXTI->FTSR |= 0x003000;		// Falling Trigger Enable  (EXTI12:PH12) (EXTI13:PH13) 
    	EXTI->IMR  |= 0x003000;  	// EXTI12,13 ���ͷ�Ʈ mask (Interrupt Enable)
		
	NVIC->ISER[1] |= (1<<8);   	// Enable Interrupt EXTI 12,13 Vector table Position ����
}

void EXTI15_10_IRQHandler(void) // 10~15 �ڵ鷯 
{
  if(EXTI->PR & 0x1000) 	// EXTI12 �߻�����
  {
    EXTI->PR &= 0x1000; 	    // Pending bit Clear
    GPIOG->ODR ^= 0x0010;    // LED 4 Toggle
    GPIOG->ODR |= 0x0020;   // LED 5 0N
    BEEP();
    TIM4->CR1 ^= (1<<0);	// CEN: Toggle the Tim4 Counter  					
  }
 
  else if(EXTI->PR & 0x2000) 	// EXTI13 �߻�����
  {            
    EXTI->PR &= 0x2000; 	// Pending bit Clear
     if( !(GPIOG->ODR & 0x0010) && (GPIOG->ODR & 0x0020) ){  // LED4 OFF , LED5 ON �ϰ��(�ߴܻ���)�϶��� �����ϰ�
     BEEP();
     GPIOG->ODR &= ~0x0030;  // LED4 ,5 OFF 
     stopmili=0;
     stopsec=0;
     stopsec_10=0;
     LCD_DisplayChar(3,4, 0x30+stopsec_10);        //  10����ǥ��
     LCD_DisplayChar(3,5, 0x30+stopsec);        // �� ����
     LCD_DisplayChar(3,7,0x30+stopmili);        //  0.1�� secǥ��
     TIM4->CR1 &= ~(1<<0);	// CEN: Disable the Tim4 Counter  	
     }
  }
}

void BEEP(void)			// Beep for 20 ms 
{ 	GPIOF->ODR |= 0x0200;	// PF9 'H' Buzzer on
	DelayMS(20);		// Delay 20 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

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

void DisplayInitScreen(void)
{
    LCD_Clear(RGB_YELLOW);                   // ȭ�� Ŭ����
    LCD_SetFont(&Gulim8);                   // ��Ʈ : ���� 8
    LCD_SetBackColor();      // ���ڹ��� : Yellow
  
    LCD_SetPenColor(RGB_BLACK);         // �� ��: Black
    LCD_DrawRectangle(5, 1, 120, 23);     //  Title ������ �׵θ� �׸���
 
    LCD_SetTextColor(RGB_BLUE);           // ���ڻ� : Blue
    LCD_DisplayText(0,1,"Digital Watch");  
    LCD_DisplayText(2,1,"TIME");             // TIME �׸���
    LCD_DisplayText(3,1,"SW");               // SW �׸���

    LCD_SetTextColor(RGB_GREEN);        // ���ڻ� : Green
    LCD_DisplayText(1,1,"2018130026 WHY");  
   
    LCD_SetTextColor(RGB_BLACK);       // ���ڻ� : Black
    LCD_DisplayText(2,6,"PM 07:0");        // TIME �ʱⰪ �׸���
    LCD_DisplayText(3,4,"00:0");              // SW �ʱⰪ �׸���
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
		{   	key_flag = 1;		
			DelayMS(10);
			return key;
		}
	}
}
