/////////////////////////////////////////////////////////////
// 과제명: Maze Runner
// 과제개요:  시작위치(0,0)에 있는 ‘Mouse’(파란 사각형)를 조이스틱을 이용
//               하여 목표위치(4,4)에 이동시키는 게임
//  조이스틱의 움직임을 체크하는 변수들을 선언하여 LCD에 표시하는 변수를 만들어주고
//  목표위치에 도착했을때 다른기능없이 리셋만 가능하도록 하는 변수또한 추가하여 기능을 제어했다.
//  조이스틱이 움직이려면 주어진 조건을 만족시켜야하기에 if문으로 가정을 주어 제어하였다. 
// 사용한 하드웨어(기능): GPIO, Joy-stick, Switch ,LED ,GLCD 
// 제출일: 2022. 5. 25
// 제출자 클래스: 목요일반
// 학번: 2018130026
// 이름: 우하용
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
uint8_t   SW0_Flag, SW1_Flag;    // Switch 입력이 홀수번째인지? 짝수번째인지? 를 알기위한 변수 
uint8_t Xcur,Ycur; // coord . of mouse
int startflag=0; // 좌표가(4,4) 일때 SW0만 눌리게 하려는 변수
int U=0,D=0,R=0,L=0,X=0,Y=0; // UP,DOWN,RIGHT,LEFT은 조이스틱 움직인 횟수  X,Y 는 좌표표시

int main(void)
{
  _GPIO_Init();       // GPIO (LED, SW, Buzzer) 초기화
  LCD_Init();      // LCD 모듈 초기화
  DelayMS(100);
  BEEP();
  
  GPIOG->ODR &= ~0x00FF;   // LED 초기값: LED0~7 Off
  DisplayInitScreen();   // LCD 초기화면
  Xcur=0;  //변수초기화
  Ycur=0;  
  GPIOG->ODR |= 0x0001; // RESET 실행시 LED0 ON
  
  while(1){
    
    switch(KEY_Scan()){ 
      
    case SW0_PUSH :   // SW1  , RESET 기능
      GPIOG->ODR &= ~0x00FF;   // LED0~7 Off  
      GPIOG->ODR |= 0x0001; //  LED0 ON
      DisplayInitScreen();   // LCD 초기화면
      _GPIO_Init();  //GPIO 초기화
      
      U=0,D=0,R=0,L=0,X=0,Y=0; 
      Xcur=0;
      Ycur=0;
      startflag=0; // 변수 초기화
      
      break;
    case SW7_PUSH:   // SW 7
      if(startflag==0){  // (4,4)일때 SW0만 눌리도록 , (4,4)가 아닌상황에서는 작동 
        GPIOG->ODR |= 0x0080; // LED7 ON
        BEEP();  
        LCD_SetTextColor(RGB_RED); // 글자색 : 빨간색
        LCD_DisplayChar(5,22,'0');     
        LCD_DisplayChar(5,24,'0');   //  0,0 좌표표시
      }
      break;  
    } // Switch key
    
    switch(JOY_Scan()){ 
    case  NAVI_UP :  // Up-stick
      if((GPIOG->ODR & 0x0080) && (GPIOG->ODR & 0x0001) ){   // LED 0,7 이 켜져있는지 체크 비트 비교
        if(Ycur<=0) { // 현재 Y좌표가 0일때
          BEEP();
          DelayMS(500);
          BEEP();
          break;
        }
        BEEP();
        GPIOG->ODR |= 0x0002;  //LED1 ON
        GPIOG->ODR &=~0x001C;  //  LED2,3,4 OFF
        U++; // Up 조이스틱 횟수 증가
        if(U>9){U=0;} // 조이스틱 횟수 0->1->2->...->9->0 구현
        Y--; // Y좌표 감소
        if(Y<0){Y=0;} // Y좌표가 0보다 작아지지못하게     
        NewCount(); //LCD에 조이스틱 횟수 및 좌표 표시
        startflag=1;  // 조이스틱 움직여 시작하는 flag
        Erase_Mouse(); // Mouse 지우기 
        Ycur--;  // Up이기때문에 지운뒤에 감소시키기
        LCD_SetBrushColor(RGB_BLUE); 
        LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10);  //  Mouse그리기
      }
      break;
      
    case  NAVI_DOWN :  // Down-stick
      if((GPIOG->ODR & 0x0080) && (GPIOG->ODR & 0x0001) ) {// LED 0,7 이 켜져있는지 체크 비트 비교
        if(Ycur >= 4){  // 현재 Y좌표가 4일때
          BEEP();
          DelayMS(500);
          BEEP();
          break;
        }
        BEEP();
        GPIOG->ODR |= 0x0004; // LED2 ON
        GPIOG->ODR &=~0x001A;  // LED1,3,4 OFF
        D++; //  Down 조이스틱 횟수 증가
        if(D>9){D=0;} // 조이스틱 횟수 0->1->2->...->9->0 구현
        Y++; // Y좌표 증가
        if(Y>4){Y=4;} //Y좌표가 4보다 커지지못하게
        NewCount(); //LCD에 조이스틱 횟수 및 좌표 표시
        startflag=1;  // 조이스틱 움직여 시작하는 flag
        Erase_Mouse();// Mouse 지우기 
        Ycur++;  // Down이기때문에 지운뒤 Y증가시키기
        LCD_SetBrushColor(RGB_BLUE);
        LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10); //Mouse그리기
      }
      break;           
      
    case  NAVI_LEFT :  // Left-stick
      if((GPIOG->ODR & 0x0080) && (GPIOG->ODR & 0x0001) ) {// LED 0,7 이 켜져있는지 체크 비트 비교
        if(Xcur <= 0 ) { //현재 X좌표가 0일때
          BEEP();
          DelayMS(500);
          BEEP();
          break;
        }
        BEEP();
        GPIOG->ODR |= 0x0008; // LED3 ON
        GPIOG->ODR &=~0x0016; // LED1,2,4 OFF
        L++; //  LEFT 조이스틱 횟수증가
        if(L>9){L=0;} // 조이스틱 횟수 0->1->2->...->9->0 구현
        X--; //X좌표 감소 
        if(X<0){X=0;} //X좌표가 0보다 작아지지못하게 
        NewCount();  //LCD에 조이스틱 횟수 및 좌표 표시
        startflag=1;   // 조이스틱 움직여 시작하는 flag
        Erase_Mouse(); // Mouse 지우기
        Xcur--; // LEFT이기에 지우고 X감소
        LCD_SetBrushColor(RGB_BLUE);
        LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10);  //Mouse그리기
        
      }
      break;           
      
    case  NAVI_RIGHT :  // Right-stick
      if((GPIOG->ODR & 0x0080) && (GPIOG->ODR & 0x0001) ) {// LED 0,7 이 켜져있는지 체크 비트 비교    
        if(Xcur >= 4) { //현재 X좌표가 4일때
          BEEP();
          DelayMS(500);
          BEEP();
          break;
        }
        BEEP();
        GPIOG->ODR |= 0x0010; //LED4 ON
        GPIOG->ODR &=~0x000E;  // LED 1,2,3 OFF
        R++; //  RIGHT 조이스틱 횟수 증가
        if(R>9){R=0;} // 조이스틱 횟수 0->1->2->...->9->0 구현
        X++; // X좌표 증가
        if(X>4){X=4;} //X좌표 4초과시 4로 고정
        NewCount(); //LCD에 조이스틱 횟수 및 좌표 표시
        startflag=1;   // 조이스틱 움직여 시작하는 flag
        Erase_Mouse();  // Mouse 지우기 
        Xcur++;        //RIGHT 이기에 지우고 X증가 
        LCD_SetBrushColor(RGB_BLUE);
        LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10);  //Mouse그리기
        break;
      }
    } // swtich joy  
    StopJoy();
  } // while 
}  // main 

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) 초기 설정   */
void _GPIO_Init(void)
{
  // LED (GPIO G) 설정 : Output mode
  RCC->AHB1ENR   |=  0x00000040;   // RCC_AHB1ENR : GPIOG(bit#6) Enable                     
  GPIOG->MODER    |=  0x00005555;   // GPIOG 0~7 : Output mode (0b01)                  
  GPIOG->OTYPER   &= ~0x00FF;   // GPIOG 0~7 : Push-pull  (GP8~15:reset state)   
  GPIOG->OSPEEDR    |=  0x00005555;   // GPIOG 0~7 : Output speed 25MHZ Medium speed 
  
  // SW (GPIO H) 설정 : Input mode 
  RCC->AHB1ENR    |=  0x00000080;   // RCC_AHB1ENR : GPIOH(bit#7) Enable                     
  GPIOH->MODER    &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state)            
  GPIOH->PUPDR    &= ~0xFFFF0000;   // GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
  
  // Buzzer (GPIO F) 설정 : Output mode
  RCC->AHB1ENR   |=  0x00000020;   // RCC_AHB1ENR : GPIOF(bit#5) Enable                     
  GPIOF->MODER    |=  0x00040000;   // GPIOF 9 : Output mode (0b01)                  
  GPIOF->OTYPER    &= ~0x0200;   // GPIOF 9 : Push-pull     
  GPIOF->OSPEEDR    |=  0x00040000;   // GPIOF 9 : Output speed 25MHZ Medium speed 
  
  //Joy Stick SW(GPOI I) 설정 : Input Mode 
  RCC->AHB1ENR    |= 0x00000100;   // RCC_AHB1ENR GPIOI Enable
  GPIOI->MODER    &= ~0x000FFC00;   // GPIOI 5~9 : Input mode (reset state)
  GPIOI->PUPDR    &= ~0x000FFC00;   // GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)
}

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
  // Display Title
  LCD_Clear(RGB_WHITE);      // 화면 클리어(흰색)
  LCD_SetFont(&Gulim7);      // 폰트 : 굴림 7
  LCD_SetBackColor(RGB_YELLOW); // 글자배경색 : 노란색
  LCD_SetTextColor(RGB_BLUE);     //글자색 : 파란색
  
  LCD_DrawText(2,3,"Maze Runner"); // Title 작성 : Maze Runner
  
  Draw_Maze();
  LCD_DrawHorLine(1,2,157);
  LCD_DrawHorLine(1,125,157);
  LCD_DrawVerLine(1,2,123);
  LCD_DrawVerLine(158,2,123); // 바깥 사각형 그리기 
  
  LCD_SetBackColor(RGB_WHITE); // 글자배경색 : 하얀색 
  LCD_SetTextColor(RGB_BLACK); // 글자색 : 검은색
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


/* Switch가 입력되었는지를 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
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
  LCD_SetPenColor(RGB_BLACK);  // 펜 색상 : 검은색
  
  
  for(int i=0; i<5;i++)
  {
    LCD_DrawLine(15,30+i*15,75,30+i*15); // 수평선
    LCD_DrawLine(15+i*15,30,15+i*15, 90); // 수직선
  }  // 정사각형 그리기
}
void Erase_Mouse(void){
  
  // Erase the mouse at current position
  LCD_SetBrushColor(RGB_WHITE);
  if(Ycur==0){ LCD_DrawFillRect(10+Xcur*15,25,10,10);}  // Ycur이 0일때 
  else if(Xcur==0)  { LCD_DrawFillRect(10,25+Ycur*15,10,10);} // Xcur 이 0일때 
  else {  LCD_DrawFillRect(10+Xcur*15,25+Ycur*15,10,10);}  // 나머지 경우 
  Draw_Maze();
}
void StopJoy(void){
  if((X==4) && (Y==4)){ // (4,4)일때
    X=0;Y=0;
    RCC->AHB1ENR &= ~0x00000100; // JOY disable
    startflag=1; // 4,4가 도착 했다는 확인변수 
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
  LCD_DisplayChar(10,3,0x30 + U); // 아스키 코드로 LCD에 UP횟수출력
  LCD_DisplayChar(10,7,0x30 + D);  // 아스키 코드로 LCD에 DOWN횟수출력
  LCD_DisplayChar(10,11,0x30 + L); // 아스키 코드로 LCD에 LEFT횟수출력
  LCD_DisplayChar(10,15,0x30 + R);// 아스키 코드로 LCD에 RIGHT횟수출력
  LCD_DisplayChar(5,22,0x30+X);  // 아스키 코드로 LCD에 X좌표 출력
  LCD_DisplayChar(5,24,0x30+Y); // 아스키 코드로 LCD에 Y좌표 출력
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










