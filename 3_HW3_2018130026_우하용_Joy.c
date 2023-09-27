/////////////////////////////////////////////////////////////
// ������: Maze Runner
// ��������:  ������ġ(0,0)�� �ִ� ��Mouse��(�Ķ� �簢��)�� ���̽�ƽ�� �̿�
//               �Ͽ� ��ǥ��ġ(4,4)�� �̵���Ű�� ����
//  ���̽�ƽ�� �������� üũ�ϴ� �������� �����Ͽ� LCD�� ǥ���ϴ� ������ ������ְ�
//  ��ǥ��ġ�� ���������� �ٸ���ɾ��� ���¸� �����ϵ��� �ϴ� �������� �߰��Ͽ� ����� �����ߴ�.
//  ���̽�ƽ�� �����̷��� �־��� ������ �������Ѿ��ϱ⿡ if������ ������ �־� �����Ͽ���. 
// ����� �ϵ����(���): GPIO, Joy-stick, Switch ,LED ,GLCD 
// ������: 2022. 5. 25
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


// NO Joy-Button : 0x03E0 : 0000 0011 1110 0000 
//Bit No      FEDC BA98 7654 3210
#define NAVI_PUSH   0x03C0  //PI5 0000 0011 1100 0000 
#define NAVI_UP      0x03A0  //PI6 0000 0011 1010 0000 
#define NAVI_DOWN       0x0360  //PI7 0000 0011 0110 0000 
#define NAVI_RIGHT   0x02E0  //PI8 0000 0010 1110 0000 
#define NAVI_LEFT   0x01E0  //PI9 0000 0001 1110 0000 

void _GPIO_Init(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);
void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void Draw_Maze(void);
void Erase_Mouse(void);
void StopJoy(void);
void NewCount(void);
uint8_t   SW0_Flag, SW1_Flag;    // Switch �Է��� Ȧ����°����? ¦����°����? �� �˱����� ���� 
uint8_t Xcur,Ycur; // coord . of mouse
int startflag=0; // ��ǥ��(4,4) �϶� SW0�� ������ �Ϸ��� ����
int U=0,D=0,R=0,L=0,X=0,Y=0; // UP,DOWN,RIGHT,LEFT�� ���̽�ƽ ������ Ƚ��  X,Y �� ��ǥǥ��

int main(void)
{
  _GPIO_Init();       // GPIO (LED, SW, Buzzer) �ʱ�ȭ
  LCD_Init();      // LCD ��� �ʱ�ȭ
  DelayMS(100);
  BEEP();
  
  GPIOG->ODR &= ~0x00FF;   // LED �ʱⰪ: LED0~7 Off
  DisplayInitScreen();   // LCD �ʱ�ȭ��
  Xcur=0;  //�����ʱ�ȭ
  Ycur=0;  
  GPIOG->ODR |= 0x0001; // RESET ����� LED0 ON
  
  while(1){
    
    switch(KEY_Scan()){ 
      
    case SW0_PUSH :   // SW1  , RESET ���
      GPIOG->ODR &= ~0x00FF;   // LED0~7 Off  
      GPIOG->ODR |= 0x0001; //  LED0 ON
      DisplayInitScreen();   // LCD �ʱ�ȭ��
      _GPIO_Init();  //GPIO �ʱ�ȭ
      
      U=0,D=0,R=0,L=0,X=0,Y=0; 
      Xcur=0;
      Ycur=0;
      startflag=0; // ���� �ʱ�ȭ
      
      break;
    case SW7_PUSH:   // SW 7
      if(startflag==0){  // (4,4)�϶� SW0�� �������� , (4,4)�� �ƴѻ�Ȳ������ �۵� 
        GPIOG->ODR |= 0x0080; // LED7 ON
        BEEP();  
        LCD_SetTextColor(RGB_RED); // ���ڻ� : ������
        LCD_DisplayChar(5,22,'0');     
        LCD_DisplayChar(5,24,'0');   //  0,0 ��ǥǥ��
      }
      break;  
    } // Switch key
    
    switch(JOY_Scan()){ 
    case  NAVI_UP :  // Up-stick
      if((GPIOG->ODR & 0x0080) && (GPIOG->ODR & 0x0001) ){   // LED 0,7 �� �����ִ��� üũ ��Ʈ ��
        if(Ycur<=0) { // ���� Y��ǥ�� 0�϶�
          BEEP();
          DelayMS(500);
          BEEP();
          break;
        }
        BEEP();
        GPIOG->ODR |= 0x0002;  //LED1 ON
        GPIOG->ODR &=~0x001C;  //  LED2,3,4 OFF
        U++; // Up ���̽�ƽ Ƚ�� ����
        if(U>9){U=0;} // ���̽�ƽ Ƚ�� 0->1->2->...->9->0 ����
        Y--; // Y��ǥ ����
        if(Y<0){Y=0;} // Y��ǥ�� 0���� �۾��������ϰ�     
        NewCount(); //LCD�� ���̽�ƽ Ƚ�� �� ��ǥ ǥ��
        startflag=1;  // ���̽�ƽ ������ �����ϴ� flag
        Erase_Mouse(); // Mouse ����� 
        Ycur--;  // Up�̱⶧���� ����ڿ� ���ҽ�Ű��
        LCD_SetBrushColor(RGB_BLUE); 
        LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10);  //  Mouse�׸���
      }
      break;
      
    case  NAVI_DOWN :  // Down-stick
      if((GPIOG->ODR & 0x0080) && (GPIOG->ODR & 0x0001) ) {// LED 0,7 �� �����ִ��� üũ ��Ʈ ��
        if(Ycur >= 4){  // ���� Y��ǥ�� 4�϶�
          BEEP();
          DelayMS(500);
          BEEP();
          break;
        }
        BEEP();
        GPIOG->ODR |= 0x0004; // LED2 ON
        GPIOG->ODR &=~0x001A;  // LED1,3,4 OFF
        D++; //  Down ���̽�ƽ Ƚ�� ����
        if(D>9){D=0;} // ���̽�ƽ Ƚ�� 0->1->2->...->9->0 ����
        Y++; // Y��ǥ ����
        if(Y>4){Y=4;} //Y��ǥ�� 4���� Ŀ�������ϰ�
        NewCount(); //LCD�� ���̽�ƽ Ƚ�� �� ��ǥ ǥ��
        startflag=1;  // ���̽�ƽ ������ �����ϴ� flag
        Erase_Mouse();// Mouse ����� 
        Ycur++;  // Down�̱⶧���� ����� Y������Ű��
        LCD_SetBrushColor(RGB_BLUE);
        LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10); //Mouse�׸���
      }
      break;           
      
    case  NAVI_LEFT :  // Left-stick
      if((GPIOG->ODR & 0x0080) && (GPIOG->ODR & 0x0001) ) {// LED 0,7 �� �����ִ��� üũ ��Ʈ ��
        if(Xcur <= 0 ) { //���� X��ǥ�� 0�϶�
          BEEP();
          DelayMS(500);
          BEEP();
          break;
        }
        BEEP();
        GPIOG->ODR |= 0x0008; // LED3 ON
        GPIOG->ODR &=~0x0016; // LED1,2,4 OFF
        L++; //  LEFT ���̽�ƽ Ƚ������
        if(L>9){L=0;} // ���̽�ƽ Ƚ�� 0->1->2->...->9->0 ����
        X--; //X��ǥ ���� 
        if(X<0){X=0;} //X��ǥ�� 0���� �۾��������ϰ� 
        NewCount();  //LCD�� ���̽�ƽ Ƚ�� �� ��ǥ ǥ��
        startflag=1;   // ���̽�ƽ ������ �����ϴ� flag
        Erase_Mouse(); // Mouse �����
        Xcur--; // LEFT�̱⿡ ����� X����
        LCD_SetBrushColor(RGB_BLUE);
        LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10);  //Mouse�׸���
        
      }
      break;           
      
    case  NAVI_RIGHT :  // Right-stick
      if((GPIOG->ODR & 0x0080) && (GPIOG->ODR & 0x0001) ) {// LED 0,7 �� �����ִ��� üũ ��Ʈ ��    
        if(Xcur >= 4) { //���� X��ǥ�� 4�϶�
          BEEP();
          DelayMS(500);
          BEEP();
          break;
        }
        BEEP();
        GPIOG->ODR |= 0x0010; //LED4 ON
        GPIOG->ODR &=~0x000E;  // LED 1,2,3 OFF
        R++; //  RIGHT ���̽�ƽ Ƚ�� ����
        if(R>9){R=0;} // ���̽�ƽ Ƚ�� 0->1->2->...->9->0 ����
        X++; // X��ǥ ����
        if(X>4){X=4;} //X��ǥ 4�ʰ��� 4�� ����
        NewCount(); //LCD�� ���̽�ƽ Ƚ�� �� ��ǥ ǥ��
        startflag=1;   // ���̽�ƽ ������ �����ϴ� flag
        Erase_Mouse();  // Mouse ����� 
        Xcur++;        //RIGHT �̱⿡ ����� X���� 
        LCD_SetBrushColor(RGB_BLUE);
        LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10);  //Mouse�׸���
        break;
      }
    } // swtich joy  
    StopJoy();
  } // while 
}  // main 

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) �ʱ� ����   */
void _GPIO_Init(void)
{
  // LED (GPIO G) ���� : Output mode
  RCC->AHB1ENR   |=  0x00000040;   // RCC_AHB1ENR : GPIOG(bit#6) Enable                     
  GPIOG->MODER    |=  0x00005555;   // GPIOG 0~7 : Output mode (0b01)                  
  GPIOG->OTYPER   &= ~0x00FF;   // GPIOG 0~7 : Push-pull  (GP8~15:reset state)   
  GPIOG->OSPEEDR    |=  0x00005555;   // GPIOG 0~7 : Output speed 25MHZ Medium speed 
  
  // SW (GPIO H) ���� : Input mode 
  RCC->AHB1ENR    |=  0x00000080;   // RCC_AHB1ENR : GPIOH(bit#7) Enable                     
  GPIOH->MODER    &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state)            
  GPIOH->PUPDR    &= ~0xFFFF0000;   // GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
  
  // Buzzer (GPIO F) ���� : Output mode
  RCC->AHB1ENR   |=  0x00000020;   // RCC_AHB1ENR : GPIOF(bit#5) Enable                     
  GPIOF->MODER    |=  0x00040000;   // GPIOF 9 : Output mode (0b01)                  
  GPIOF->OTYPER    &= ~0x0200;   // GPIOF 9 : Push-pull     
  GPIOF->OSPEEDR    |=  0x00040000;   // GPIOF 9 : Output speed 25MHZ Medium speed 
  
  //Joy Stick SW(GPOI I) ���� : Input Mode 
  RCC->AHB1ENR    |= 0x00000100;   // RCC_AHB1ENR GPIOI Enable
  GPIOI->MODER    &= ~0x000FFC00;   // GPIOI 5~9 : Input mode (reset state)
  GPIOI->PUPDR    &= ~0x000FFC00;   // GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
  // Display Title
  LCD_Clear(RGB_WHITE);      // ȭ�� Ŭ����(���)
  LCD_SetFont(&Gulim7);      // ��Ʈ : ���� 7
  LCD_SetBackColor(RGB_YELLOW); // ���ڹ��� : �����
  LCD_SetTextColor(RGB_BLUE);     //���ڻ� : �Ķ���
  
  LCD_DrawText(2,3,"Maze Runner"); // Title �ۼ� : Maze Runner
  
  Draw_Maze();
  LCD_DrawHorLine(1,2,157);
  LCD_DrawHorLine(1,125,157);
  LCD_DrawVerLine(1,2,123);
  LCD_DrawVerLine(158,2,123); // �ٱ� �簢�� �׸��� 
  
  LCD_SetBackColor(RGB_WHITE); // ���ڹ��� : �Ͼ�� 
  LCD_SetTextColor(RGB_BLACK); // ���ڻ� : ������
  LCD_DrawText(6,14,"(0,0)");  // 0,0 
  LCD_DrawText(70,14,"(4,0)");  // 4,0 
  LCD_DrawText(6,97,"(0,4)");  // 0,0 
  LCD_DrawText(70,97,"(4,4)");  // 4,0 
  
  LCD_DisplayText(10,1,"U:0,D:0,L:0,R:0");
  LCD_DisplayText(3,14,"RESET(SW0)");
  LCD_DisplayText(4,14,"RESET(SW7)");
  LCD_DisplayText(5,14,"STATUS:(X,Y)");
  
  LCD_SetTextColor(RGB_RED);
  LCD_DisplayChar(5,22,'X');
  LCD_DisplayChar(5,24,'Y');
  LCD_DisplayChar(10,3,'0');
  LCD_DisplayChar(10,7,'0');
  LCD_DisplayChar(10,11,'0');
  LCD_DisplayChar(10,15,'0');
  
  // Draw the mouse at (0,0)
  LCD_SetBrushColor(RGB_BLUE);
  LCD_DrawFillRect(10,25,10,10);
}


/* Switch�� �ԷµǾ������� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */ 
uint8_t key_flag = 0;
uint16_t KEY_Scan(void)   // input key SW0 - SW7 
{ 
  uint16_t key;
  key = GPIOH->IDR & 0xFF00;   // any key pressed ?
  if(key == 0xFF00)      // if no key, check key off
  {   if(key_flag == 0)
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

/* Buzzer: Beep for 30 ms */
void BEEP(void)         
{    
  GPIOF->ODR |=  0x0200;   // PF9 'H' Buzzer on
  DelayMS(30);      // Delay 30 ms
  GPIOF->ODR &= ~0x0200;   // PF9 'L' Buzzer off
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

void Draw_Maze(void)
{
  // Making The Maze
  LCD_SetPenColor(RGB_BLACK);  // �� ���� : ������
  
  
  for(int i=0; i<5;i++)
  {
    LCD_DrawLine(15,30+i*15,75,30+i*15); // ����
    LCD_DrawLine(15+i*15,30,15+i*15, 90); // ������
  }  // ���簢�� �׸���
}
void Erase_Mouse(void){
  
  // Erase the mouse at current position
  LCD_SetBrushColor(RGB_WHITE);
  if(Ycur==0){ LCD_DrawFillRect(10+Xcur*15,25,10,10);}  // Ycur�� 0�϶� 
  else if(Xcur==0)  { LCD_DrawFillRect(10,25+Ycur*15,10,10);} // Xcur �� 0�϶� 
  else {  LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10);}  // ������ ��� 
  Draw_Maze();
}
void StopJoy(void){
  if((X==4) && (Y==4)){ // (4,4)�϶�
    X=0;Y=0;
    RCC->AHB1ENR &= ~0x00000100; // JOY disable
    startflag=1; // 4,4�� ���� �ߴٴ� Ȯ�κ��� 
    DelayMS(1000);
    BEEP();
    DelayMS(500);
    BEEP();
    DelayMS(500);  
    BEEP();
    DelayMS(500);
  }
}
void NewCount(void){
  LCD_DisplayChar(10,3,0x30 + U); // �ƽ�Ű �ڵ�� LCD�� UPȽ�����
  LCD_DisplayChar(10,7,0x30 + D);  // �ƽ�Ű �ڵ�� LCD�� DOWNȽ�����
  LCD_DisplayChar(10,11,0x30 + L); // �ƽ�Ű �ڵ�� LCD�� LEFTȽ�����
  LCD_DisplayChar(10,15,0x30 + R);// �ƽ�Ű �ڵ�� LCD�� RIGHTȽ�����
  LCD_DisplayChar(5,22,0x30+X);  // �ƽ�Ű �ڵ�� LCD�� X��ǥ ���
  LCD_DisplayChar(5,24,0x30+Y); // �ƽ�Ű �ڵ�� LCD�� Y��ǥ ���
}
uint8_t joy_flag = 0;
uint16_t JOY_Scan(void)   // input joy stick NAVI_* 
{ 
  uint16_t key;
  key = GPIOI->IDR & 0x03E0;   // any key pressed ? : 5~9
  if(key == 0x03E0)      // if no key, check key off
  {     if(joy_flag == 0)
    return key;
  else
  {
    DelayMS(10);
    joy_flag = 0;
    return key;
  }
  }
  else            // if key input, check continuous key
  {   
    if(joy_flag != 0)   // if continuous key, treat as no key input
      return 0x03E0;
    else         // if new key,delay for debounce
    {
      joy_flag = 1;
      DelayMS(10);
      return key;
    }
  }
}










