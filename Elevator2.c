#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"
#define SW0_PUSH 0xFE00 
#define SW1_PUSH 0xFD00
#define SW2_PUSH 0xFB00
#define SW3_PUSH 0xF700
#define SW4_PUSH 0xEF00
#define SW5_PUSH 0xDF00
#define SW6_PUSH 0xBF00
#define SW7_PUSH 0x7F00
uint8_t SW7_Flag;
void _GPIO_Init(void);
void _EXTI_Init(void);
uint16_t KEY_Scan(void);
void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void DrawMoveup(void);
void DrawMovedown(void);
void DrawMovestop(void);
void countingSix(void);
void LedOn(void);
unsigned char DF= '1';  // ��ǥ�� 1������ �ʱ�ȭ
unsigned char CF= '1';   // ������ 1������ �ʱ�ȭ
int witdh = 0;   // �簢�� �׸������Ѻ���
int count=0;     // Fram ���� ����

int main(void)
{
  _GPIO_Init(); // GPIO (LED & SW) �ʱ�ȭ
  _EXTI_Init();   // EXTI �ʱ�ȭ
  LCD_Init();   // LCD ��� �ʱ�ȭ
  Fram_Init();                    // FRAM �ʱ�ȭ H/W �ʱ�ȭ
  Fram_Status_Config();   // FRAM �ʱ�ȭ S/W �ʱ�ȭ
  
  DelayMS(100);
  BEEP();
  DisplayInitScreen();
  
  GPIOG->ODR &= ~0x00FF;   // LED �ʱⰪ: LED0~7 Off  ����ODR ���� 0 ���ִ�.
  GPIOG->ODR |= 0x0002; // �ʱ���� LED 2 on
  
  
  count = Fram_Read(26);  // Fram�� ����� �� �ҷ�����
  
  if(count>0 && count <10)  // count ���� ����
  {
    LCD_SetTextColor(GET_RGB(255,0,255));  // ���ڻ�:�����
    LCD_DisplayChar(1,8, count+0x30);  
  }
  else
  {
    count = 0;
    LCD_SetTextColor(GET_RGB(255,0,255)); // ���ڻ�:�����
    LCD_DisplayChar(1,8, count+0x30);
  }
  
  while(1)
  {
    
    LedOn();
    
    switch(KEY_Scan())      
    {       
    case SW0_PUSH : //SW0
      DF = '0';
      LCD_SetTextColor(RGB_BLACK);   
      LCD_DisplayText(2,3,"B");
      GPIOG->ODR &= ~0x00FF; 
      GPIOG->ODR |= 0x0001;
      
      break;  
    case SW1_PUSH : //SW1
      DF = '1';
      LCD_SetTextColor(RGB_BLACK); 
      LCD_DisplayText(2,3,"1");
      GPIOG->ODR &= ~0x00FF; 
      GPIOG->ODR |= 0x0002;
      
      break;         
    case SW2_PUSH : //SW2
      DF = '2'; 
      LCD_SetTextColor(RGB_BLACK); 
      LCD_DisplayText(2,3,"2");
      GPIOG->ODR &= ~0x00FF; 
      GPIOG->ODR |= 0x0004;
      
      
      break; 
    case SW3_PUSH : //SW3
      DF = '3';
      LCD_SetTextColor(RGB_BLACK); 
      LCD_DisplayText(2,3,"3"); 
      GPIOG->ODR &= ~0x00FF; 
      GPIOG->ODR |= 0x0008;
      
      break;         
    case SW4_PUSH : //SW4
      DF ='4';  
      LCD_SetTextColor(RGB_BLACK); 
      LCD_DisplayText(2,3,"4");  
      GPIOG->ODR &= ~0x00FF; 
      GPIOG->ODR |= 0x0010;
      
      
      break;              
    case SW5_PUSH : //SW5
      DF = '5';  
      LCD_SetTextColor(RGB_BLACK); 
      LCD_DisplayText(2,3,"5");
      GPIOG->ODR &= ~0x00FF; 
      GPIOG->ODR |= 0x0020;
      
      break;        
    case SW6_PUSH : //SW6
      DF = '6';  
      LCD_SetTextColor(RGB_BLACK); 
      LCD_DisplayText(2,3,"6");
      GPIOG->ODR &= ~0x00FF; 
      GPIOG->ODR |= 0x0040;
      
      break;        
    }  // switch(KEY_Scan())
    
    
    if(SW7_Flag==1)
    {
      SW7_Flag=0;
      
      while(1)
      {
        if(DF>CF)// ���������� ��ǥ���� �� ���� ���������� ��� 
        {         
          GPIOG->ODR |= 0x0080;
          if(CF=='0')
          {
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(41,45,8,8);
            LCD_SetBrushColor(RGB_RED);
            LCD_DrawFillRect(42,46,7,7);  
            
            for(CF; CF<=DF; CF++)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMoveup();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(41,45,8+witdh,8);
              LCD_SetBrushColor(RGB_RED);
              LCD_DrawFillRect(42,46,7+witdh,7);          
              LedOn();
              witdh+=16;
              DelayMS(250);
            }
            witdh=0;
            CF--;  // CF�� for������ ++�Ʊ⿡ 1����
            DrawMovestop();
            countingSix();
            break;
          }
          if(CF=='1')
          { 
            
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(55,45,8,8);
            LCD_SetBrushColor(RGB_RED);
            LCD_DrawFillRect(56,46,7,7);  
            for(CF; CF<=DF; CF++)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMoveup();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(55,45,8+witdh,8);
              LCD_SetBrushColor(RGB_RED);
              LCD_DrawFillRect(56,46,7+witdh,7);                  
              LedOn();
              witdh+=16;
              DelayMS(250);
            }
            CF--; // CF�� for������ ++�Ʊ⿡ 1����
            witdh=0;
            DrawMovestop();
            countingSix();
            break;
          }
          if(CF=='2')
          {
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(71,45,8,8);
            LCD_SetBrushColor(RGB_RED);
            LCD_DrawFillRect(72,46,7,7); 
            
            for(CF; CF<=DF; CF++)
            {    
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMoveup();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(71,45,8+witdh,8);
              LCD_SetBrushColor(RGB_RED);
              LCD_DrawFillRect(72,46,7+witdh,7);                  
              
              LedOn();
              witdh+=16;
              DelayMS(250);
            }
            CF--;  // CF�� for������ ++�Ʊ⿡ 1����
            witdh=0;
            DrawMovestop();
            countingSix();
            break;
          }
          if(CF=='3')
          {
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(87,45,8,8);
            LCD_SetBrushColor(RGB_RED);
            LCD_DrawFillRect(88,46,7,7);  
            
            for(CF; CF<=DF; CF++)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMoveup();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(87,45,8+witdh,8);
              LCD_SetBrushColor(RGB_RED);
              LCD_DrawFillRect(88,46,7+witdh,7);                  
              LedOn();
              witdh+=16;
              DelayMS(250);
            }
            CF--;   // CF�� for������ ++�Ʊ⿡ 1����
            witdh=0;
            DrawMovestop();
            countingSix();
            break;
          }
          if(CF=='4')
          { 
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(103,45,8,8);
            LCD_SetBrushColor(RGB_RED);
            LCD_DrawFillRect(104,46,7,7); 
            for(CF; CF<=DF; CF++)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMoveup();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(103,45,8+witdh,8);
              LCD_SetBrushColor(RGB_RED);
              LCD_DrawFillRect(104,46,7+witdh,7);                  
              LedOn();
              witdh+=16;
              DelayMS(250);
            }
            
            CF--; // CF�� for������ ++�Ʊ⿡ 1����
            witdh=0;
            DrawMovestop();
            countingSix();
            break;
            
          }
          if(CF=='5')
          {
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(119,45,8,8);
            LCD_SetBrushColor(RGB_RED);
            LCD_DrawFillRect(120,46,7,7);  
            
            for(CF; CF<=DF; CF++)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMoveup();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(119,45,8+witdh,8);
              LCD_SetBrushColor(RGB_RED);
              LCD_DrawFillRect(120,46,7+witdh,7);                  
              LedOn();
              witdh+=16;
              DelayMS(250);
            }
            CF--; // CF�� for������ ++�Ʊ⿡ 1����
            witdh=0;
            DrawMovestop();
            countingSix();
            break;
          }
          
        }    // if(DF>CF) 
        
        
        else if(CF>DF)// �������� ��ǥ������ ũ�⿡ ���������� �ϰ�
        {         
          GPIOG->ODR |= 0x0080;
          if(CF=='6')
          {
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(135,45,8,8);
            LCD_SetBrushColor(RGB_BLUE);
            LCD_DrawFillRect(136,46,7,7);  //6�� 
            
            for(CF; CF>=DF; CF--)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMovedown();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(135-witdh,45,8+witdh,8);
              LCD_SetBrushColor(RGB_BLUE);
              LCD_DrawFillRect(136-witdh,46,7+witdh,7);  //6��           
              LedOn();
              witdh+=16;
              DelayMS(250);
            }
            witdh=0;
            CF++;  // for������ CF�� �ѹ� �� �����⿡ 1������
            DrawMovestop();
            //count++;
            break;
          }
          if(CF=='5')
          { 
            
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(119,45,8,8);
            LCD_SetBrushColor(RGB_BLUE);
            LCD_DrawFillRect(120,46,7,7); // 5��
            
            for(CF; CF>=DF; CF--)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMovedown();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(119-witdh,45,8+witdh,8);
              LCD_SetBrushColor(RGB_BLUE);
              LCD_DrawFillRect(120-witdh,46,7+witdh,7); // 5��               
              LedOn();
              
              witdh+=16;
              DelayMS(250);
            }
            CF++;// for������ CF�� �ѹ� �� �����⿡ 1������
            witdh=0;
            DrawMovestop();
            break;
          }
          if(CF=='4')
          {
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(103,45,8,8);
            LCD_SetBrushColor(RGB_BLUE);
            LCD_DrawFillRect(104,46,7,7); // 4��
            
            for(CF; CF>=DF; CF--)
            {    
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMovedown();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(103-witdh,45,8+witdh,8);
              LCD_SetBrushColor(RGB_BLUE);
              LCD_DrawFillRect(104-witdh,46,7+witdh,7); // 4��                  
              LedOn();
              witdh+=16;
              DelayMS(250);
            }
            CF++; // for������ CF�� �ѹ� �� �����⿡ 1������
            witdh=0;
            DrawMovestop();
            break;
          }
          if(CF=='3')
          {
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(87,45,8,8);
            LCD_SetBrushColor(RGB_BLUE);
            LCD_DrawFillRect(88,46,7,7);  //3�� 
            
            for(CF; CF>=DF; CF--)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMovedown();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(87-witdh,45,8+witdh,8);
              LCD_SetBrushColor(RGB_BLUE);
              LCD_DrawFillRect(88-witdh,46,7+witdh,7);  //3��                
              
              LedOn();
              witdh+=16;
              DelayMS(250);
            }
            CF++;   // for������ CF�� �ѹ� �� �����⿡ 1������
            witdh=0;
            DrawMovestop();
            break;
          }
          if(CF=='2')
          { 
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(71,45,8,8);
            LCD_SetBrushColor(RGB_BLUE);
            LCD_DrawFillRect(72,46,7,7); //2 ��
            
            for(CF; CF>=DF; CF--)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMovedown();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(71-witdh,45,8+witdh,8);
              LCD_SetBrushColor(RGB_BLUE);
              LCD_DrawFillRect(72-witdh,46,7+witdh,7); //2 ��
              LedOn();
              
              witdh+=16;
              DelayMS(250);
            }
            
            CF++; // for������ CF�� �ѹ� �� �����⿡ 1������
            witdh=0;
            DrawMovestop();
            break;
          }
          if(CF=='1')
          {
            LCD_SetBrushColor(RGB_YELLOW);
            LCD_DrawFillRect(0,45,159,10);
            LCD_SetPenColor(RGB_GREEN);
            LCD_DrawRectangle(55,45,8,8);
            LCD_SetBrushColor(RGB_BLUE);
            LCD_DrawFillRect(56,46,7,7);   //1��
            
            for(CF; CF>=DF; CF--)
            {
              LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
              LCD_DisplayText(1,0,"M");
              DrawMovedown();
              LCD_SetPenColor(RGB_GREEN);
              LCD_DrawRectangle(55-witdh,45,8+witdh,8);
              LCD_SetBrushColor(RGB_BLUE);
              LCD_DrawFillRect(56-witdh,46,7+witdh,7);   //1�� 
              LedOn();
              
              witdh+=16;  
              DelayMS(250);
            }
            CF++; // for������ CF�� �ѹ� �� �����⿡ 1������
            witdh=0;  
            DrawMovestop();
            break;
          }
        }    // if(CF>DF) 
        
        
        
        else if(CF==DF)  //�������� ��ǥ���� ������� ��, ���������� ��ǥ�� ��ư ������
        {
          break;
        }  // else if(CF==DF)
        
      }    // SW7 while(1) 
      
      GPIOG->ODR &= ~0x0080;
      
      
    } // SW7 Interrupt on
    
  }  // while(1)
  
} 


void _GPIO_Init(void)
{
  // LED (GPIO G) ����
  RCC->AHB1ENR   |=  0x00000040;   // RCC_AHB1ENR : GPIOG(bit#6) Enable                     
  GPIOG->MODER    |=  0x00005555;   // GPIOG 0~7 : Output mode (0b01)                  
  GPIOG->OTYPER   &= ~0x00FF;           // GPIOG 0~7 : Push-pull  (GP8~15:reset state)   
  GPIOG->OSPEEDR    |=  0x00005555;   // GPIOG 0~7 : Output speed 25MHZ Medium speed 
  
  // SW (GPIO H) ���� 
  RCC->AHB1ENR        |=  0x00000080;           // RCC_AHB1ENR : GPIOH(bit#7) Enable                     
  GPIOH->MODER    &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state)            
  GPIOH->PUPDR    &= ~0xFFFF0000;   // GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
  
  // Buzzer (GPIO F) ���� 
  RCC->AHB1ENR   |=  0x00000020;         // RCC_AHB1ENR : GPIOF(bit#5) Enable                     
  GPIOF->MODER    |=  0x00040000;   // GPIOF 9 : Output mode (0b01)                  
  GPIOF->OTYPER    &= ~0x0200;           // GPIOF 9 : Push-pull     
  GPIOF->OSPEEDR    |=  0x00040000;   // GPIOF 9 : Output speed 25MHZ Medium speed 
  
}   


void _EXTI_Init(void)
{
  RCC->AHB1ENR |= 0x00000080;   // RCC_AHB1ENR GPIOH Enable
  RCC->APB2ENR |= 0x00004000;   // Enable System Configuration Controller Clock  14��
  
  GPIOH->MODER &= ~0xFFFF0000; // GPIOH PIN8~PIN15 Input mode (reset state)             
  
  // EXTICR3(EXTICR[2])�� �̿� 
  // reset value: 0x0000   
  SYSCFG->EXTICR[3] |= 0x7000; //EXTI15<- PH15
  
  
  EXTI->FTSR |= 0x008000;           // EXTI15: Falling Trigger Enable 
  
  EXTI->IMR  |=  0x008000;        // EXTI8 ,15 ���ͷ�Ʈ mask (Interrupt Enable) ����
  
  NVIC->ISER[1] |= (1 << 8);  // Enable 'Global Interrupt EXTI 12,13,15 .,nvic�� �������̺��̴�.  EXTI 15�� ������ 40 �̱⿡   
  //NVIC->ISER[1] |=(1<<(40-32));
  
}

void EXTI15_10_IRQHandler(void)
{
  if (EXTI->PR & 0x8000)
  {
    EXTI -> PR |= 0x8000; // EXIT 15
    SW7_Flag=1;      
  }
  
}

void DisplayInitScreen(void)
{
  LCD_Clear(RGB_YELLOW);   // ȭ�� Ŭ����
  LCD_SetFont(&Gulim8);   // ��Ʈ : ���� 8
  
  LCD_SetBackColor(RGB_YELLOW);
  LCD_SetTextColor(RGB_BLACK); // ���ڻ�: ������
  LCD_DisplayText(0,0,"2018130026 WHY"); // 2018130026 WHYȭ�����
  LCD_DisplayText(1,0,"S");//LCD_DisplayText(1,0,"S"); 
  LCD_SetTextColor(GET_RGB(255,0,255));  // ���ڻ�: �����
  LCD_DisplayText(1,2,"O");
  LCD_SetTextColor(RGB_BLACK); // ���ڻ�: ������
  LCD_DisplayText(1,4,"6No:");
  LCD_SetTextColor(GET_RGB(255,0,255));  // ���ڻ�: �����
  LCD_DisplayText(1,8,"0");  
  LCD_SetTextColor(RGB_BLACK);// ���ڻ� : ����
  LCD_DisplayText(2,0,"FL:1 B 1 2 3 4 5 6");//LCD_DisplayText(2,0,"FL:1 B 1 2 3 4 5 6"); 
  LCD_SetPenColor(RGB_GREEN);
  LCD_DrawRectangle(55,45,8,8);
  LCD_SetBrushColor(RGB_RED);
  LCD_DrawFillRect(56,46,7,7);   
  
}

uint8_t key_flag = 0;
uint16_t KEY_Scan(void)   // input key SW0 - SW7 
{ 
  uint16_t key;
  key = GPIOH->IDR & 0xFF00;   // any key pressed ?
  if(key == 0xFF00)      // if no key, check key off
  {     if(key_flag == 0)
    return key;
  else
  {   DelayMS(10);
  key_flag = 0;
  return key;
  }
  }
  else            // if key input, check continuous key
  {   if(key_flag != 0)   // if continuous key, treat as no key input
    return 0xFF00;
  else         // if new key,delay for debounce
  {   key_flag = 1;
  DelayMS(10);
  return key;
  }
  }
}

void BEEP(void)         /* beep for 30 ms */
{    
  GPIOF->ODR |=  0x0200;   // PF9 'H' Buzzer on
  DelayMS(30);      // Delay 30 ms
  GPIOF->ODR &= ~0x0200;   // PF9 'L' Buzzer off
}
void DelayMS(unsigned short wMS)
{
  register unsigned short i;
  for (i=0; i<wMS; i++)
    DelayUS(1000);               // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
  volatile int Dly = (int)wUS*17;
  for(; Dly; Dly--);
}

void DrawMoveup(void)
{
  LCD_DisplayText(1,2," "); 
  LCD_SetPenColor(RGB_RED);
  LCD_DrawHorLine(15,18,10);
  LCD_DrawLine(21,14,25,18);
  LCD_DrawLine(21,22,25,18);
}

void DrawMovedown(void)
{
  LCD_DisplayText(1,2," "); 
  LCD_SetPenColor(RGB_BLUE);
  LCD_DrawHorLine(15,18,10);
  LCD_DrawLine(19,14,15,18);
  LCD_DrawLine(19,22,15,18);
}

void DrawMovestop(void)
{
  LCD_SetPenColor(RGB_YELLOW);
  LCD_DrawHorLine(15,18,10);
  LCD_DrawLine(21,14,25,18);
  LCD_DrawLine(21,22,25,18);
  LCD_DrawLine(19,14,15,18);
  LCD_DrawLine(19,22,15,18);
  
  LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
  LCD_DisplayText(1,0,"S");
  LCD_SetTextColor(GET_RGB(255,0,255)); //���ڻ�: �����
  LCD_DisplayText(1,2,"O");
  LCD_SetTextColor(RGB_BLACK);  // ���ڻ�: ������
}

void countingSix(void)
{
  if(CF=='6')
  {
    count++;
    if(count==10)
    {
      count=0;
    }
  }
  
  Fram_Write(26,count);
  LCD_SetTextColor(GET_RGB(255,0,255));
  LCD_DisplayChar(1,8,0x30+count);
  
}

void LedOn(void)
{
  if(CF=='0') {GPIOG->ODR |= 0x0001;}
  if(CF=='1') {GPIOG->ODR |= 0x0002;}
  if(CF=='2') {GPIOG->ODR |= 0x0004;}
  if(CF=='3') {GPIOG->ODR |= 0x0008;}
  if(CF=='4') {GPIOG->ODR |= 0x0010;}
  if(CF=='5') {GPIOG->ODR |= 0x0020;}
  if(CF=='6') {GPIOG->ODR |= 0x0040;}
}