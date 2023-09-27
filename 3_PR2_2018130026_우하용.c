//////////////////////////////////////////////////////////////////////////
// ����: PC���� �������� ���� Robot ���۸��(command)�� �޾�, 
//�ؼ��ϰ� command�� ���� �κ� ���ۿ� ���� ��ȣ�� �߻�
// USART1, TIM3,4,7,  EXTI, ADC, GPIO ���
// ������: 2022. 12. 08
// ������ Ŭ����: ����Ϲ�
// �й�: 2018130026
// �̸�: ���Ͽ�
//////////////////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"
#include "ACC.h"
#include "FRAM.h"

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
void BEEP(void);
void _ADC_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);
void TIMER4_PWM_Init(void);
void TIMER3_Init(void);
void TIMER7_Init(void);
void USART1_Init(void);
void USART_BRR_Configuration(uint32_t USART_BaudRate);
void SerialSendChar(uint8_t c);
void SerialSendString(char *s);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);



int ModeFlag; // ����Ʈ��ġ�� ��带 ��ȯ���ִ� �÷��׺���

int hour = 22; // 0x30 + 22 = F
int min = 16; //0x30 + 16 ���� �������ڸ��� Timer ���°� -1  �ʱⰪ ���� �� ,�ð� ����

int Alarm_h;
int Alarm_m; //�˶� ���� ����

int MatchAlarm = 0;  // �˶��� ����ð��� ��ġ�Ǿ����� Ȯ���ϴ� ����

double ADC2_Value,TEMP_VAL,VSENSE,Voltage,Temperature;    // ADC ���庯����

void ADC_IRQHandler(void)
{
  if(ModeFlag==2) // Thermostat ����϶�
  {
    //ADC3�� ���������� ���� ��� ���� �������� ���� -> ������ ���� �µ� ������ ��ȯ-> ���ǿ� ���߾� ȭ�鿡 ���÷���
    if (ADC2->SR && ADC_SR_EOC == ADC_SR_EOC){ // ADC3 EOC int
      ADC2->SR &= ~(1<<1);		// EOC flag clear  
     
      ADC2_Value = ADC2->DR;   	// DATA REG���� ���� ����  
     
      //�Ҽ��� �Ʒ� ���ڸ��� ������ �Ǳ� ������ x10
      Voltage =ADC2_Value * (3.3 * 10) / 4093 ;   //4093���� ����  
     
      //�Ҽ��� �Ʒ� ���ڸ��� ������ �Ǳ� ������ Voltage���� *100�Ǿ����� /10 ����
      Temperature = (4.7 * Voltage * Voltage / 10 - 110);      //�µ� ���ϴ� ���� ���� ����, -10~40���� ���;���
     
      LCD_SetBrushColor(RGB_WHITE);
      LCD_DrawFillRect(45,15,156, 7);   // �� �����
     
      GPIOG->ODR &= ~0x00FF;    // LED 0~7 OFF
      TIM4->CR1	  |= (1<<0);	// CEN: Counter TIM4 disable

      if( (int)Temperature >-110 && (int)Temperature<=-100 ) // -10�� ~ -10�� �϶�
      {        
        LCD_DisplayText(1,2,"   ");        
        Temperature = fabs(Temperature); // �µ� ���밪
        LCD_DisplayChar(1,2,'-');
        LCD_DisplayChar(1,3,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,4,(int)Temperature%100/10 + 0x30);    
       
        LCD_SetBrushColor(RGB_BLUE);
        LCD_DrawFillRect(45,15, 10, 7);   // ���� �׸���	
       
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'2');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'0'); // Heator 2�ܰ�, Cooler 0�ܰ�
       
        GPIOG->ODR |= 0x0020;  // LED 5 ON
        TIM4->CCR1=2000;    // DUTY RATIO 90% ����
      }
     
      else  if( (int)Temperature > -100 && (int)Temperature<10 ) // -9�� ~ 0 ��  �϶�
      {        
        LCD_DisplayText(1,2,"   ");        
        Temperature = fabs(Temperature);
        LCD_DisplayChar(1,2,'-');
        LCD_DisplayChar(1,3,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,4,(int)Temperature%100/10 + 0x30);
        LCD_SetBrushColor(RGB_BLUE);
        LCD_DrawFillRect(45,15, 20 - (int)Temperature%100/10, 7);   // ���� �׸���	
       
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'2');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'0'); // Heator 2�ܰ�, Cooler 0�ܰ�
       
        GPIOG->ODR |= 0x0020;  // LED 5 ON
        TIM4->CCR1=2000;    // DUTY RATIO 90% ����
       
      }
      else if(( (int)Temperature>=10 && (int)Temperature<100) )  // 1�� ~ 9��
      {    
        LCD_DisplayText(1,2,"   ");        
        LCD_DisplayChar(1,2,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,3,(int)Temperature%100/10+ 0x30);
        LCD_SetBrushColor(RGB_BLUE);
        LCD_DrawFillRect(45,15,20+(int)Temperature%100/10, 7);   // ���� �׸���	
       
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'1');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'0'); // Heator 1�ܰ�, Cooler 0�ܰ�
       
        GPIOG->ODR |= 0x0020; // LED 5 ON
        TIM4->CCR1=18000;  // DUTY RATIO 10% ����
       
      }
      else if(( (int)Temperature>=100 && (int)Temperature<110) )  // 10�� ~ 10��
      {    
        LCD_DisplayText(1,2,"   ");        
        LCD_DisplayChar(1,2,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,3,(int)Temperature%100/10+ 0x30);
        LCD_SetBrushColor(RGB_BLUE);
       
        LCD_DrawFillRect(45,15,30, 7);   // ���� �׸���	
       
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'1');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'0');  // Heator 1�ܰ�, Cooler 0�ܰ�
         
        GPIOG->ODR |= 0x0020;  //LED 5 On
        TIM4->CCR1=18000;  // DUTY RATIO 10% ����
       
      }
      else if(( (int)Temperature>=110 && (int)Temperature<200) )  // 11�� ~ 19��
      {
        LCD_DisplayText(1,2,"   ");        
        LCD_DisplayChar(1,2,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,3,(int)Temperature%100/10 + 0x30);
        LCD_SetBrushColor(RGB_GREEN);
        LCD_DrawFillRect(45,15,30+(int)Temperature%100/10, 7);   // ���� �׸���
       
        // Heator & Cooler OFF
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'0');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'0');
        TIM4->CR1	&= ~(1<<0);	// CEN: Counter TIM4 disable    
      }
      else if(( (int)Temperature>=200 && (int)Temperature<210) )  // 20~20��
      {
        LCD_DisplayText(1,2,"   ");        
        LCD_DisplayChar(1,2,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,3,(int)Temperature%100/10 + 0x30);
        LCD_SetBrushColor(RGB_RED);
        LCD_DrawFillRect(45,15,40, 7);   // ���� �׸���
       
         // Heator & Cooler OFF
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'0');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'0');      
        TIM4->CR1	&= ~(1<<0);	// CEN: Counter TIM4 disable
      }
      else if(( (int)Temperature>=210 && (int)Temperature<300) )  // 21�� ~ 29 ��
      {
        LCD_DisplayText(1,2,"   ");        
        LCD_DisplayChar(1,2,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,3,(int)Temperature%100/10 + 0x30);
        LCD_SetBrushColor(RGB_RED);
        LCD_DrawFillRect(45,15,40+(int)Temperature%100/10, 7);   // ���� �׸���
       
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'0');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'1'); // Heator 0�ܰ�, Cooler 1 �ܰ�
       
        GPIOG->ODR |= 0x0040;  // LED 6 ON
        TIM4->CCR1=18000; // DUTY RATIO 10% ����
      }
      else if(( (int)Temperature>=300 && (int)Temperature<310) )  // 30�� ~ 30 ��
      {
        LCD_DisplayText(1,2,"   ");        
        LCD_DisplayChar(1,2,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,3,(int)Temperature%100/10 + 0x30);
        LCD_SetBrushColor(RGB_RED);
        LCD_DrawFillRect(45,15,50, 7);   // ���� �׸���
       
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'0');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'1'); // Heator 0�ܰ�, Cooler 1 �ܰ�
       
       GPIOG->ODR |= 0x0040;  // LED 6 ON
        TIM4->CCR1=18000; // DUTY RATIO 10% ����
      }
      else if(( (int)Temperature>=310 && (int)Temperature<400) )  // 31��~ 39��
      {	
        LCD_DisplayText(1,2,"   ");        
        LCD_DisplayChar(1,2,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,3,(int)Temperature%100/10 + 0x30);
        LCD_SetBrushColor(RGB_RED);
        LCD_DrawFillRect(45,15,50+(int)Temperature%100/10, 7);   // ���� �׸���
       
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'0');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'2'); // Heator 0�ܰ�, Cooler 2 �ܰ�
       
        GPIOG->ODR |= 0x0040; // LED 6 ON
        TIM4->CCR1=2000;   // DUTY RATIO 90% ����
      }
     
      else if(( (int)Temperature>=400 && (int)Temperature<410) )  // 40��
      {	
        LCD_DisplayText(1,2,"   ");        
        LCD_DisplayChar(1,2,(int)Temperature/100 + 0x30);
        LCD_DisplayChar(1,3,(int)Temperature%100/10 + 0x30);
        LCD_SetBrushColor(RGB_RED);
        LCD_DrawFillRect(45,15,60, 7);   // ���� �׸���
       
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayChar(2,2,'0');
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayChar(2,6,'2'); // Heator 0�ܰ�, Cooler 2 �ܰ�
       
        GPIOG->ODR |= 0x0040;   // LED 6 ON
        TIM4->CCR1=2000;   // DUTY RATIO 90% ����
      }
    }
   
  }
}

int main(void)
{
  LCD_Init();             // LCD ���� �Լ�
  DelayMS(10);            // LCD���� ������
  TIMER4_PWM_Init();      // TIM4 Init (PWM mode)
  _ADC_Init();            //ADC �ʱ�ȭ
  _GPIO_Init();           // LED, SW �ʱ�ȭ
  USART1_Init();          // USART1 �ʱ�ȭ
  TIMER7_Init();          // TIM7 �ʱ�ȭ
  DelayMS(10);            // LCD���� ������
  _EXTI_Init();           // EXTI �ʱ�ȭ
  DelayMS(10);            // LCD���� ������
  TIMER3_Init();          // TIM3 �ʱ�ȭ
  DelayMS(10);            // LCD���� ������

  Fram_Init();            // FRAM �ʱ�ȭ H/W �ʱ�ȭ
  Fram_Status_Config();   // FRAM �ʱ�ȭ S/W �ʱ�ȭ
 
  Alarm_h=Fram_Read(1208);
  Alarm_m=Fram_Read(1209);   // 1208,1209 �������� �˶����� ��������
 
  if(!( (Alarm_h>=0 && Alarm_h<=23)))
    Alarm_h=0;
  if(!( (Alarm_m>=0 && Alarm_m<=23)))  //�ʱ� �˶������� Fram���� ���� �� �� �������� 0�����ʱ�ȭ
    Alarm_m=0;
 
  ModeFlag=0;
  DisplayTitle();      // LCD �ʱ�ȭ�鱸�� �Լ�
 
  while(1)
  {
   
//    if(MatchAlarm == 1) // �˶��� ����ð��� ��ġ�Ǹ�
//    {
//       BEEP();
//      DelayMS(100);
//        BEEP();
//      DelayMS(100);
//       BEEP();
//      MatchAlarm=0;  
//    }
   if(ModeFlag==0)
   {
    switch(KEY_Scan())
    {
    case SW0_PUSH  : 	//SW0 'Alarm '��' ����
      Alarm_h++;
      LCD_SetTextColor(RGB_RED);
      if(Alarm_h==10)  // �˶� 9�̻�� A�� ǥ���ϵ���
        Alarm_h = 17;
      if(Alarm_h==23) // F�̻�-> 0���� �ǵ���
        Alarm_h=0;
      LCD_DisplayChar(1,6,0x30+Alarm_h);  
	         BEEP();

      break;
     
    case SW1_PUSH  : 	//SW1 'Alarm '��' ���� '
      Alarm_m++;
      LCD_SetTextColor(RGB_RED);
      if(Alarm_m==10)
        Alarm_m = 17;
      if(Alarm_m==23)
        Alarm_m=0;
      LCD_DisplayChar(1,8,0x30+Alarm_m);
	        BEEP();

      break;
     
    case SW2_PUSH  : 	//SW2 �˶��ð� FRAM(1208,1209)�� ����
      BEEP();
      DelayMS(100);
       BEEP();
      Fram_Write(1208,Alarm_h);
      Fram_Write(1209,Alarm_m); // 1208,1209���� Fram ����
      break;	
    }
   }
  }
}

///////////////////////////////////////////////////////
int Operator1=0; // OP1
int Operator2=0; // OP2
int result = 0; // ����� ��� ����
int Next = 0;  // LCD ���� �̵�����
void USART1_IRQHandler(void)  
{      
  // TX Buffer Empty Interrupt
  if ( (USART1->CR1 & (1<<7)) && (USART1->SR & USART_SR_TXE) )      // USART_SR_TXE=(1<<7)
  {
    USART1->CR1 &= ~(1<<7);   // 0x0080, TXE interrupt Enable
   
    USART1->CR1 |= (1<<7);    // TXE interrupt Enable
  }
 
  if(ModeFlag==1) // ���� ����϶�
  {
    // RX Buffer Full interrupt
    if ( (USART1->SR & USART_SR_RXNE) )      // USART_SR_RXNE=(1<<5)
    {
     
      char ch;
      LCD_SetTextColor(RGB_RED);
     
      ch = (uint16_t)(USART1->DR & (uint16_t)0x01FF);   // ���ŵ� ���� ����
     
      switch(ch)  
      {
      case '1' :   // 1 ������
        LCD_DisplayText(1,2+Next,"1");  
        if(Next==0) Operator1= 1;
        else if(Next==2) Operator2=1;
        Next=2;
        break;    
      case '2' :  // 2 ������
        LCD_DisplayText(1,2+Next,"2");  
        if(Next==0) Operator1= 2;
        else if(Next==2) Operator2=2;
        Next=2;
        break;    
      case '3' : // 3 ������
        LCD_DisplayText(1,2+Next,"3");
        if(Next==0) Operator1= 3;
        else if(Next==2) Operator2=3;
        Next=2;
        break;  
      case '4' :  //  4 ������
        LCD_DisplayText(1,2+Next,"4");
        if(Next==0) Operator1= 4;
        else if(Next==2) Operator2=4;
        Next=2;
        break;    
      case '=' :   //  ��� �� ��� ���� �ʱ�ȭ
        result= Operator1+Operator2;
        LCD_DisplayChar(1,6,0x30+result);
        Next=0;
        Operator1=0;
        Operator2=0;
        result=0;
        break;  
      }  
      // DR �� ������ SR.RXNE bit(flag bit)�� clear �ȴ�. �� clear �� �ʿ����
    }
  }
}    



void USART1_Init(void)
{
  // USART1 : TX(PA9)
  RCC->AHB1ENR   |= (1<<0);   // RCC_AHB1ENR GPIOA Enable
  GPIOA->MODER   |= (2<<2*9);   // GPIOB PIN9 Output Alternate function mode              
  GPIOA->OSPEEDR   |= (3<<2*9);   // GPIOB PIN9 Output speed (100MHz Very High speed)
  GPIOA->AFR[1]   |= (7<<4);   // Connect GPIOA pin9 to AF7(USART1)
 
  // USART1 : RX(PA10)
  GPIOA->MODER    |= (2<<2*10);   // GPIOA PIN10 Output Alternate function mode
  GPIOA->OSPEEDR   |= (3<<2*10);   // GPIOA PIN10 Output speed (100MHz Very High speed
  GPIOA->AFR[1]   |= (7<<8);   // Connect GPIOA pin10 to AF7(USART1)
 
  RCC->APB2ENR   |= (1<<4);   // RCC_APB2ENR USART1 Enable
 
  USART_BRR_Configuration(9600); // USART Baud rate Configuration
 
  USART1->CR1   &= ~(1<<12);   // USART_WordLength 8 Data bit
  USART1->CR1   &= ~(1<<10);   // NO USART_Parity
  USART1->CR1   |= (1<<2);   // 0x0004, USART_Mode_RX Enable
  USART1->CR1   |= (1<<3);   // 0x0008, USART_Mode_Tx Enable
  USART1->CR2   &= ~(3<<12);   // 0b00, USART_StopBits_1
  USART1->CR3   = 0x0000;   // No HardwareFlowControl, No DMA
 
  USART1->CR1   |= (1<<5);   // 0x0020, RXNE interrupt Enable
  USART1->CR1   &= ~(1<<7); // 0x0080, TXE interrupt Disable   ,ó���� ������ ������ ������ �����°� ����
 
  NVIC->ISER[1]   |= (1<<(37-32));// Enable Interrupt USART1 (NVIC 37��)
 
  USART1->CR1    |= (1<<13);   //  0x2000, USART1 Enable
}
void SerialSendChar(uint8_t Ch) // 1���� ������ �Լ�
{
  while((USART1->SR & USART_SR_TXE) == RESET); // USART_SR_TXE=(1<<7), �۽� ������ ���±��� ���
  // ����ִ��� Ȯ���ϴ�
  USART1->DR = (Ch & 0x01FF);   // ���� (�ִ� 9bit �̹Ƿ� 0x01FF�� masking)
 
}

void BEEP(void)			// Beep for 20 ms
{ 	
  GPIOF->ODR |= 0x0200;	// PF9 'H' Buzzer on
DelayMS(20);		// Delay 20 ms
GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void SerialSendString(char *str) // �������� ������ �Լ�
{
  while (*str != '\0') // ���Ṯ�ڰ� ������ ������ ����, ���Ṯ�ڰ� �����Ŀ��� ������ �޸� ���� �߻����ɼ� ����.
  {
    SerialSendChar(*str);   // �����Ͱ� ����Ű�� ���� �����͸� �۽�
    str++;          // ������ ��ġ ����
  }
}
void USART_BRR_Configuration(uint32_t USART_BaudRate)
{
  uint32_t tmpreg = 0x00;
  uint32_t APB2clock = 84000000;   //PCLK2_Frequency
  uint32_t integerdivider = 0x00;
  uint32_t fractionaldivider = 0x00;
 
  // Determine the integer part
  if ((USART1->CR1 & USART_CR1_OVER8) != 0) // USART_CR1_OVER8=(1<<15)
  {                                         // USART1->CR1.OVER8 = 1 (8 oversampling)
    // Computing 'Integer part' when the oversampling mode is 8 Samples
    integerdivider = ((25 * APB2clock) / (2 * USART_BaudRate));    
  }
  else  // USART1->CR1.OVER8 = 0 (16 oversampling)
  {   // Computing 'Integer part' when the oversampling mode is 16 Samples
    integerdivider = ((25 * APB2clock) / (4 * USART_BaudRate));    
  }
  tmpreg = (integerdivider / 100) << 4;
 
  // Determine the fractional part
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));
 
  // Implement the fractional part in the register
  if ((USART1->CR1 & USART_CR1_OVER8) != 0)   // 8 oversampling
  {
    tmpreg |= (((fractionaldivider * 8) + 50) / 100) & (0x07);
  }
  else          // 16 oversampling
  {
    tmpreg |= (((fractionaldivider * 16) + 50) / 100) & (0x0F);
  }
 
  // Write to USART BRR register
  USART1->BRR = (uint16_t)tmpreg;
}
void _ADC_Init(void)
{   	// ADC2 �ʱ� ����          
  RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;	// (1<<9) ENABLE ADC2 CLK (stm32f4xx.h ����)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   // (1<<0) ENABLE GPIOA CLK (stm32f4xx.h ����) �������׻��
 
  GPIOA->MODER |= (3<<2*1);      // CONFIG GPIOA PIN(PA1) TO ANALOG IN MODE
 
  ADC->CCR &= ~(0x1F<<0);		// MULTI[4:0]: ADC���: ADC_Mode_Independent
  ADC->CCR |=  (1<<16); 		        // ADCPRE:ADC_Prescaler_Div4 (ADC MAX Clock 36MHz, 84Mhz(APB2)/4 = 21MHz)
  ADC->CCR |=  (1<<23); 		        //�µ� ���� Enable
 
  //ADC3: ���������� �ܺοµ� ������ ���, TIM3_CH1 CC event ���
  ADC2->CR1 |=  (1<<5);		// EOCIE=1: Interrupt enable for EOC
  ADC2->CR1 &= ~(1<<8);		// SCAN=0 : ADC_ScanCovMode Disable	
  ADC2->CR1 &= ~(3<<24);		// RES[1:0]=0b00 : 12bit Resolution	
  ADC2->CR2 &= ~(1<<1);		// CONT=0: ADC_Continuous ConvMode Disable
  ADC2->CR2 |= (7<<24);    	// EXSEL[[3:0](27~24): ADC_External event select for regular group_Timer 3 CC1 event
  ADC2->CR2 |=  (2<<28);		// EXTEN[1:0]: ADC_ExternalTrigConvEdge_Enable(Falling Edge)	
  ADC2->CR2 &= ~(1<<11);		// ALIGN=0: ADC_DataAlign_Right
  ADC2->CR2 &= ~(1<<10);		// EOCS=0: The EOC bit is set at the end of each sequence of regular conversions
  ADC2->SQR1 &= ~(0xF<<20);	// L[3:0]: ADC Regular channel sequece length 					 	
  ADC2->SQR3 |= (1<<0);		// SQ1[4:0]=0b0001 : CH1
  ADC2->SMPR2 |= (7<<(3*1));	// ADC2_CH1 Sample TIme_480Cycles (3*Channel_1)
 
  NVIC->ISER[0] |= (1<<18);   //���ͷ�Ʈ NVIC����            
 
  //ADC2->CR2 |= (1<<0);      // ADC2 ON
}

void TIMER4_PWM_Init(void)
{  
  // TIM4 CH1 : PB6
  // Clock Enable : GPIOB & TIMER4
  RCC->AHB1ENR	|= (1<<1);	// GPIOB CLOCK Enable
  RCC->APB1ENR |= (1<<2);	// TIMER4 CLOCK Enable
 
  // PB6�� ��¼����ϰ� Alternate function(TIM4_CH1)���� ��� ���� : PWM ���
  GPIOB->MODER 	|= (2<<12);	//  PB6 Output Alternate function mode					
  GPIOB->OSPEEDR 	|= (3<<12);	//  PB6 Output speed (100MHz High speed)
  GPIOB->OTYPER	&= ~(1<<6);	// PB6 Output type push-pull (reset state)
  GPIOB->PUPDR	&= ~(1<<12);	//  PB6 Pull-Down
  GPIOB->AFR[0]	|= (2<<24);	//  (AFR[0].(3~0)=0b0010): Connect TIM4 pins(PB6) to AF2(TIM3..5)
 
  // TIM4 Channel 1: PWM 1 mode
  // Assign 'PWM Pulse Period'
  TIM4->PSC	= 8400-1;	// Prescaler 84,000,000Hz/8400 = 10,000 Hz(0.1ms)  (1~65536)
  TIM4->ARR	= 20000-1;	// Auto reload  (0.1ms * 20000 = 2sec : PWM Period)
 
  // Setting CR1 : 0x0000 (Up counting)
  TIM4->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
  TIM4->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled)
  TIM4->CR1 |= (1<<7);	// ARPE=1(ARR is buffered): ARR Preload Enable
  TIM4->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
  TIM4->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 : Edge-aligned mode(reset state)
 
  // Define the corresponding pin by 'Output'  
  // CCER(Capture/Compare Enable Register) : Enable "Channel 1"
  TIM4->CCER	|= (1<<0);	// CC1E=1: OC1(TIM4_CH1) Active(Capture/Compare 1 output enable)
  // �ش���(167��)�� ���� ��ȣ���
  TIM4->CCER	&= ~(1<<1);	// CC3P=0: CC3 Output Polarity (OCPolarity_High : OC3���� �������� ���)
 
  // Duty Ratio
  TIM4->CCR3	= 10000;		// CCR3 value : DR =50 %
 
  // 'Mode' Selection : Output mode, PWM 1
  // CCMR1(Capture/Compare Mode Register 1) : Setting the MODE of Ch1 or Ch2
  TIM4->CCMR1 &= ~(3<<0); // CC1S(CC1 channel)= '0b00' : Output
  TIM4->CCMR1 |= (1<<3); 	// OC1PE=1: Output Compare 1 preload Enable
  TIM4->CCMR1	|= (6<<4);	// OC1M=0b110: Output compare 1 mode: PWM 1 mode
  TIM4->CCMR1	|= (1<<7);	// OC1CE=1: Output compare 1 Clear enable
 
  //Counter TIM4 enable
}

void TIMER3_Init(void)
{
 
  //TIM3_CH1 (PB4) : 400ms ���ͷ�Ʈ �߻�
  // Clock Enable : GPIOI
  RCC->AHB1ENR |= (1<<1);	// GPIOB Enable
  RCC->APB1ENR |= (1<<1);	// TIMER3 Enable
 
  //PB4�� ��¼����ϰ� Alternate function(TIM3_CH1)���� ��� ����
  GPIOB->MODER |= (1<<2*8);	// PB4 Output Alternate function mode				
  GPIOB->OSPEEDR |= (1<<2*8); // PB4 Output speed (100MHz High speed)
  GPIOB->OTYPER &= ~(1<<15); // PB4 Output type push-pull (reset state)
  GPIOB->AFR[0]|= (2<<16); // (AFR[0].()=0b0010): Connect TIM3 pins(PB4) to AF2(TIM3..5)
  // PB4 ==> TIM3_CH1                          
 
  //400mSec ���� ���ߴ� ����
  TIM3->PSC = 840-1;	// Prescaler  84MHz/840 = 0.1MHz (10us)
  TIM3->ARR = 40000-1;	// Auto reload  : 10us * 40K =  400ms(period)
 
  // CR1 : Up counting
  TIM3->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
  TIM3->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
  //	- Counter Overflow/Underflow,
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller
  TIM3->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
  //	- Counter Overflow/Underflow,
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller
  TIM3->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
  TIM3->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
  TIM3->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
  TIM3->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
  // Center-aligned mode: The counter counts Up and DOWN alternatively  
 
  // Define the corresponding pin by 'Output'  
  TIM3->CCER |= (1<<0);	// CC1E=1: CC1 channel Output Enable				
  TIM3->CCER &= ~(1<<1);	// CC1P=0: CC1 channel Output Polarity (OCPolarity_High : OC3���� �������� ���)  
 
  // 'Mode' Selection : Output mode, toggle  
  TIM3->CCMR1 &= ~(3<<0);        
  TIM3->CCMR1 &= ~(1<<3);	// OC1M=0b010: Output Compare 1 Mode : toggle				
  TIM3->CCMR1 |= (3<<4);	// OC1M=0b010: Output Compare 1 Mode : toggle				
  TIM3->CCR1 = 2000;	
 
  TIM3->DIER |= (1<<1); // CC1IE:"ENABLE THE TIM4 CC1 INTERRUPT
 
  NVIC->ISER[0] = (1<<29); //TIM3 Global Interrupt Mask
 
  //TIM3->CR1 |= (1<<0);	// CEN: Enable the Tim1 Counter  					
}
void TIM3_IRQHandler(void)
{  
  TIM3->SR &= ~(1<<1);   //Interrupt flag Clear
 
}

void TIMER7_Init(void)   // ���ӵ����� ���� �ֱ� ����: 1sec
{
  // Timer7 basic TImer Only Counter mode  
 
  RCC->APB1ENR |= (1<<5);   // TIMER7 Clock Enable
 
  TIM7->CR1 &= ~(1<<4);   // DIR=0(Up counter)(reset state)  
  TIM7->CR1 &= ~(1<<1);   // UDIS=0(Update event Enabled): By one of following events
  //  Counter Overflow/Underflow,
  //  Setting the UG bit Set,
  //  Update Generation through the slave mode controller
  // UDIS=1 : Only Update event Enabled by  Counter Overflow/Underflow,
  TIM7->CR1 &= ~(1<<2);   // URS=0(Update event source Selection): one of following events
  //   Counter Overflow/Underflow,
  // Setting the UG bit Set,
  //   Update Generation through the slave mode controller
  // URS=1 : Only Update Interrupt generated  By  Counter Overflow/Underflow,
 
  TIM7->CR1 &= ~(1<<3);   // OPM=0(The counter is NOT stopped at update event) (reset state)
  TIM7->CR1 &= ~(1<<7);   // ARPE=1(ARR is buffered): ARR Preload Enalbe
  TIM7->CR1 &= ~(3<<8);    // CKD(Clock division)=00(reset state)
  TIM7->CR1 &= ~(3<<5);    // CMS(Center-aligned mode Sel)=00 : Edge-aligned mode(reset state)  
 
 
  TIM7->PSC = 8400-1;      //Prescaler 84MHz/8400 = 10kHz (0.1ms)  
  TIM7->ARR = 10000-1;      //Auto reload  0.1ms * 10000   = 1s  
 
  TIM7->EGR |= (1<<0);          //  UG(Update genration = 1)
  NVIC->ISER[1] |= ( 1 << (55-32) );   //Enable Timer7 global Interrupt
  TIM7->DIER |= (1<<0);          //Update Interrupt Enable Timer7
  TIM7->CR1 |= (1<<0);      //Enable Tim7 Counter    
}

void TIM7_IRQHandler(void)   // 1s int
{
  TIM7->SR &= ~(1<<0);   //Interrupt flag Clear
 
  min++; // 1�ʸ��� �� ����
 
  if(min == 10)
    min = 17; // 9 -> A�� ������ ����
 
  if(min==23) // F�̻��϶�  hour 1 ����
  {
    min = 0;
    hour++;
    if(hour == 10)
      hour = 17;
    if(hour == 23)
      hour = 0;
  }
  LCD_SetTextColor(RGB_BLUE);
  LCD_DisplayChar(0,14, 0x30+hour);
  LCD_DisplayChar(0,15,':');
  LCD_DisplayChar(0,16, 0x30+min);  
 
  if((min == Alarm_m)&& (hour==Alarm_h))  // ����ð��� �˶��ð��� ������ ��ġ
    MatchAlarm = 1;
 
 
}

void _GPIO_Init(void)
{
  // LED (GPIO G) ����
  RCC->AHB1ENR   |=  0x00000040;   // RCC_AHB1ENR : GPIOG(bit#6) Enable                    
  GPIOG->MODER    |=  0x00005555;   // GPIOG 0~7 : Output mode (0b01)                  
  GPIOG->OTYPER   &= ~0x00FF;   // GPIOG 0~7 : Push-pull  (GP8~15:reset state)  
  GPIOG->OSPEEDR    |=  0x00005555;   // GPIOG 0~7 : Output speed 25MHZ Medium speed
 
  // SW (GPIO H) ����
  RCC->AHB1ENR    |=  0x00000080;   // RCC_AHB1ENR : GPIOH(bit#7) Enable                    
  GPIOH->MODER    &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state)            
  GPIOH->PUPDR    &= ~0xFFFF0000;   // GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
 
  // Buzzer (GPIO F) ����
  RCC->AHB1ENR   |=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable                    
  GPIOF->MODER    |=  0x00040000;   // GPIOF 9 : Output mode (0b01)                  
  GPIOF->OTYPER    &= ~0x0200;   // GPIOF 9 : Push-pull    
  GPIOF->OSPEEDR    |=  0x00040000;   // GPIOF 9 : Output speed 25MHZ Medium speed
 
  GPIOG->ODR &= 0x00;   // LED0~7 Off
}

void _EXTI_Init(void)
{
  RCC->AHB1ENR   |=  0x00000080;   // (1<<7) RCC_AHB1ENR : GPIOH(bit#7) Enable    
  RCC->APB2ENR    |= 0x00004000;   // (1<<14)Enable System Configuration Controller Clock
 
  GPIOH->MODER    &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state)
 
  SYSCFG->EXTICR[3] |= 0x7000; // EXTI 15 <-PH15, SW7 ���ͷ�Ʈ����
 
  EXTI->FTSR |=  0x8000; //  EXTI15 (SW7) FTSR����
 
  EXTI->IMR  |= 0x8000; //    EXTI 15 (SW7) enable mask Enable
 
  NVIC->ISER[1] |= ( 1<<8 );    // (1<<(40-32)  'Global Interrupt EXTI15_10' ,
}


void EXTI15_10_IRQHandler(void)
{
  if(EXTI->PR & 0x8000)         // EXTI 15 Interrupt Pending(�߻�) ����? SW7 (Enter)
  {
    EXTI->PR |= 0x8000;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�) ����� ������� 1���.
   
    ModeFlag++;
    if(ModeFlag>=3)
      ModeFlag=0;
    DisplayTitle();   // SW7 ������ ���� ��� ����Ǿ� ȭ�� �ʱ�ȭ��
   
  }
}

void DelayMS(unsigned short wMS)
{
  register unsigned short i;
 
  for (i=0; i<wMS; i++)
    DelayUS(1000);      //1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
  volatile int Dly = (int)wUS*17;
  for(; Dly; Dly--);
}
void DisplayTitle(void)
{
  LCD_Clear(RGB_WHITE);
  LCD_SetFont(&Gulim7);      //��Ʈ
  LCD_SetBackColor(RGB_YELLOW);
  LCD_SetTextColor(RGB_BLACK);    //���ڻ�
 
  LCD_DisplayText(0,0,"X-Y Robot Control : WHY");  // Title
  
  LCD_SetBackColor(RGB_WHITE);
  LCD_DisplayText(1,0,"SD=2, RPM=");  // Title

  LCD_DisplayText(2,0,"GX=");
  LCD_DisplayChar(2,5,',');
  LCD_DisplayText(2,6,"GY=");
  LCD_DisplayChar(2,11,',');
  LCD_DisplayText(2,12,"DX=");
  LCD_DisplayChar(2,17,',');
  LCD_DisplayText(2,18,"DY=");

  LCD_DisplayText(3,0,"TPX=");
  LCD_DisplayChar(3,6,',');
  LCD_DisplayText(3,7,"TPY=");

  LCD_DisplayText(4,0,"CX=");
  LCD_DisplayChar(4,5,',');
  LCD_DisplayText(4,6,"CY=");

  LCD_DisplayText(5,0,"CPX=");
  LCD_DisplayChar(5,6,',');
  LCD_DisplayText(5,7,"CPY=");

  LCD_SetTextColor(RGB_BLUE);
  LCD_DisplayText(1,10,"3(PC)");
  LCD_DisplayText(2,3,"00");
  LCD_DisplayText(2,9,"00");
  LCD_DisplayText(2,15,"00");
  LCD_DisplayText(2,22,"00");
  
  LCD_SetTextColor(RGB_RED);
  LCD_DisplayText(4,3,"00");
  LCD_DisplayText(4,9,"00");

  LCD_DisplayText(5,4,"00");
  LCD_DisplayText(5,11,"00");
  
  
//  if(ModeFlag==0)  // �˶��ð� ���
//  {
//	
//	ADC2->CR2 &= ~(1<<0);      // ADC2 ON
//  	TIM3->CR1 &= ~(1<<0);
//	TIM4->CR1 &= ~(1<<0);
//	GPIOG->ODR &= ~0x00FF;
//    LCD_DisplayText(0,0,"1.ALARM(HGD)");  // Title
//    LCD_DisplayText(1,0,"Alarm");   //X AXIS
//   
//    LCD_SetTextColor(RGB_BLUE);
//    LCD_DisplayText(0,14,"F:A");  // ����ð�
//   
//    LCD_SetTextColor(RGB_BLUE);
//    LCD_DisplayChar(0,14, 0x30+hour);
//    LCD_DisplayChar(0,15,':');
//    LCD_DisplayChar(0,16, 0x30+min);
//   
//    LCD_SetTextColor(RGB_RED);
//    LCD_DisplayChar(1,6,0x30+Alarm_h);
//    LCD_DisplayChar(1,7,':');
//    LCD_DisplayChar(1,8,0x30+Alarm_m);  // �˶� �� ����ð� ǥ��
//  }
//  else if(ModeFlag==1)  // Calculator ���
//  {
//	
//    LCD_DisplayText(0,0,"2.Calculator");  // Title
//    LCD_DisplayChar(1,3,'+');
//    LCD_DisplayChar(1,5,'=');
//    LCD_SetTextColor(RGB_RED);    //���ڻ�
//    LCD_DisplayChar(1,2,'0');
//    LCD_DisplayChar(1,4,'0');
//    LCD_DisplayChar(1,6,'0');
//   
//    LCD_SetTextColor(RGB_BLUE);
//    LCD_DisplayChar(0,14, 0x30+hour);
//    LCD_DisplayChar(0,15,':');
//    LCD_DisplayChar(0,16, 0x30+min);  //����ð�ǥ��
//  }
//  else if(ModeFlag==2)  // Thermostat ���
//  {
//  	ADC2->CR2 |= (1<<0);      // ADC2 ON
//  	TIM3->CR1 |= (1<<0);
//	TIM4->CR1 |= (1<<0);
//	
//	LCD_DisplayText(0,0,"3.Thermostat");  // Title
//    LCD_DisplayText(1,0,"T:");
//    LCD_DisplayText(2,0,"H:");
//    LCD_DisplayText(2,4,"C:");
//   
//    LCD_SetTextColor(RGB_BLUE);
//    LCD_DisplayChar(0,14, 0x30+hour);
//    LCD_DisplayChar(0,15,':');
//    LCD_DisplayChar(0,16, 0x30+min);  //����ð� ǥ��
//  }
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