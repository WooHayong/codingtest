/////////////////////////////////////////////////////////////
// 과제명:  이진수 계산기
// 과제개요: 네자리 2진수 숫자를 ADD(+), SUB(-), OR(#), AND(&) 
//               연산하는 계산기를 제작
//               첫 초기화면에서는 무조건 Operand1이 눌리도록 설정하여, '=', Joy 입력시 Error발생하도록함
//               Operand1 확정 후 숫자나,'=', "Joy_Push(Operator 선택안됐는데 확정누른경우)" 가 눌리면 Error동작
//               Operator 확정 후 Joy입력하거나 '='입력시 Error 발생
//               Operand2 확정 후 Joy입력하거나 숫자나 Operand확정 누르면 Error발생하도록함 
//               계산 결과 이후 숫자나,Operand확정, '=' , Joy 누르면 에러나도록함       
// 사용한 하드웨어(기능): GPIO, Joy-stick, EXTI, GLCD, Key-Scan, LED, Buzzer,  
// 제출일: 2022. 06. 03
// 제출자 클래스: 목요일반
// 학번: 2018130026
// 이름: 우하용
///////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"
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
void _EXTI_Init(void);
void DisplayInitScreen(void);
uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);
void BEEP(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void GetSub(void); // 뺄셈 함수
void Complement(void); // 2의보수 함수
void GetAdd(void); // 덧셈함수(뺄셈 덧셈 둘다사용)
void SetAdd(void); // 덧셈 후 케리체크 
void BitCheak(void); // 2진 숫자 체크
void GetOr(void); // OR함수
void GetAnd(void); // AND 함수
void ErrorCheck(void); // Error 표시함수
void OneComlement(int array[],int Set); // 1의보수 함수
void GetResultText(int array[]); //결과출력함수

uint8 Next=0; // 화면에 보이는 4개의 2진수를 operator지정시 operator 다음 2진수 지정으로 넘어가기위한 변수
uint8 SW4_Flag=0,SW5_Flag=0,Joy_Flag=0,StartFlag=0,PushError=0; // 변수초기화
int arr1[5]; //operand1 담기 위한 배열
int arr2[5]; //operand2 담기 위한 배열
int ResultArr[5]; // 연산 결과 담기 위한 배열 
uint8 Carry=0; //연산과정 Carry 변수
uint8 CheckOP=0; // Operator 구분변수
uint8 Add=0,Sub=0,Or=0,And=0; // 사용한 Operator 횟수 변수

int main(void){
  
  LCD_Init();   // LCD 모듈 초기화
  DelayMS(100);
  _GPIO_Init();   // GPIO 초기화
  DelayMS(100);
  BEEP(); 
  _EXTI_Init();   // EXTI 초기화
  Fram_Init();                    // FRAM 초기화 H/W 초기화
  Fram_Status_Config();   // FRAM 초기화 S/W 초기화
  
  Add=Fram_Read(525);  // 525번지에 저장된 값 Add에 읽어오기
  Sub=Fram_Read(526);  // 526번지에 저장된 값 Add에 읽어오기
  Or=Fram_Read(527);   // 527번지에 저장된 값 Add에 읽어오기
  And=Fram_Read(528); // 528번지에 저장된 값 Add에 읽어오기
  
  DisplayInitScreen();   // LCD 초기화면
  GPIOG->ODR &= ~0x00FF;   // 초기값: LED0~7 Off
  
  while(1)
  {
	switch(KEY_Scan()){  
	case SW0_PUSH:      
	  if(StartFlag==0){
		if(GPIOG->ODR & 0x0001){// LED0 ON일때
		  GPIOG->BSRRH = 0x0001; //LED0 OFF        
		}
		else {
		  GPIOG->BSRRL = 0x0001; // LED0 ON
		}
		break;
	  }
	  else { ErrorCheck(); break;} // StartFlag변수를 SW4(Operand확정)에서 조작하여, 이외 입력에는 Error가 되도록함
	  break;
	case SW1_PUSH:
	  if(StartFlag==0){
		if(GPIOG->ODR & 0x0002){// LED1 ON일때
		  GPIOG->BSRRH=0x0002;  //LED1 OFF 
		}
		else {
		  GPIOG->BSRRL = 0x0002; // LED1 ON    
		}
		break;
	  }
	  else { ErrorCheck(); break;} // StartFlag변수를 SW4(Operand확정)에서 조작하여, 이외 입력에는 Error가 되도록함
	  break;
	case SW2_PUSH:
	  if(StartFlag==0){
		if(GPIOG->ODR & 0x0004){// LED2 ON일때
		  GPIOG->BSRRH=0x0004;  //LED2 OFF  
		}   
		else {   
		  GPIOG->BSRRL = 0x0004; //LED2 ON
		}
		break;
	  }
	  else { ErrorCheck(); break;} // StartFlag변수를 SW4(Operand확정)에서 조작하여, 이외 입력에는 Error가 되도록함
	  break;
	case SW3_PUSH:
	  if(StartFlag==0){
		if(GPIOG->ODR & 0x0008){  // LED3 ON일때     
		  GPIOG->BSRRH=0x0008;   //LED3 OFF
		}
		else {
		  GPIOG->BSRRL = 0x0008; //LED3 ON
		}
		break;
	  }
	  else { ErrorCheck(); break; } // StartFlag변수를 SW4(Operand확정)에서 조작하여, 이외 입력에는 Error 발생하도록함
	  break;
	
	} //switch key
	switch (JOY_Scan())
	{
	case NAVI_PUSH:
	  if(Joy_Flag != 1 || PushError==0 ) { ErrorCheck(); } // Operand1이 결정된 이후 (SW4가 눌린이후) 에만 사용후 나머지 경우는 Error발생하도록함
	  break;
	}//Switch Joy_Scan()
  } //while 
}// main

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
  LCD_Clear(RGB_YELLOW);          // 화면 노란색으로 클리어
  LCD_SetFont(&Gulim7);          // 폰트 : 굴림 7
  
  //초기 화면 초록사각형 틀 그리기
  LCD_SetPenColor(RGB_GREEN);      //  펜색상 : Green   
  LCD_DrawRectangle(3, 3, 152, 120);   //  큰 테두리사각형 그리기
  LCD_DrawRectangle(5, 40, 148, 20);   //  2진수 계산 사각형 그리기
  
  //영문 이니셜 
  LCD_DrawRectangle(7, 5, 93, 20);   //  영문 이니셜 사각형 그리기
  LCD_SetBackColor(RGB_BLUE);   // 글자배경색 : BLUE
  LCD_SetTextColor(RGB_WHITE);   // 글자색 : WHITE
  LCD_SetBrushColor(RGB_BLUE); // 브러쉬 : BLUE
  LCD_DrawFillRect(8,6,92,19);   // 영문이니셜 사각형 파란색으로 채우기
  LCD_DrawText(9,12,"WHY Calculator");  //  WHY Calculator  화면표시
  //Error와 사각형
  LCD_DrawRectangle(140, 12, 9, 9);  // Error 표시박스
  LCD_SetBrushColor(GET_RGB(195,195,195)); // 브러쉬 : gray
  LCD_DrawFillRect(141,13,8,8);   // Error 사각형 회색으로 채우기
  LCD_SetTextColor(RGB_BLACK);   // 글자색 : BLACK
  LCD_SetBackColor(RGB_YELLOW);   // 글자배경색 : YELLOW
  LCD_DrawText(105, 12, "Error");     //  Error 화면표시
  
  LCD_DisplayChar(4, 1 ,'>'); // 계산기 사각형 > 표시
  
  LCD_DisplayText(7,1,"+:0, -:0, #:0, &:0"); //연산자 표시
  LCD_SetTextColor(RGB_RED);
  // DisplayInitScreen 을 Fram_Read로 변수에 담아 둔 뒤 사용하여(main문 초기), 화면에 Fram에서 가져온 값들이 표시되도록함
  LCD_DisplayChar(7,3,0x30 + Add);  
  LCD_DisplayChar(7,8,0x30 + Sub);
  LCD_DisplayChar(7,13,0x30 + Or);
  LCD_DisplayChar(7,18,0x30 + And); // Operator 사용횟수 표시하기 
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) 초기 설정   */
void _GPIO_Init(void)
{  // LED (GPIO G) 설정 : Output mode
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

void _EXTI_Init(void)
{
  RCC->AHB1ENR    |= 0x00000100; // (1<<8)   RCC_AHB1ENR GPIOI Enable
  RCC->AHB1ENR   |=  0x00000080;   // (1<<7) RCC_AHB1ENR : GPIOH(bit#7) Enable    
  RCC->APB2ENR    |= 0x00004000;   // (1<<14)Enable System Configuration Controller Clock
  
  GPIOI->MODER    &= ~0x000FFC00;   // GPIOI 5~9 : Input mode (reset state)
  GPIOH->MODER    &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state) 
  
  SYSCFG->EXTICR[1] |= 0x8880;   // EXTI5<-PI5,EXTI6<-PI6,EXTI7 <-PI7 , NAVI_PUSH, NAVI_UP, NAVI_DOWN 인터럽트 설정
  SYSCFG->EXTICR[2] |= 0x0088; // EXTI8 <-PI8, EXTI9<-PI9,NAVI_RIGHT,NAVI_LEFT 인터럽트설정
  SYSCFG->EXTICR[3] |= 0x7777; // EXTI 12 <-PH12,EXTI 13 <-PH13,EXTI 14 <-PH14,EXTI 15 <-PH15, SW4,SW5,SW6,SW7 인터럽트설정
  
  EXTI->FTSR |=  0xF3E0; //  EXTI5~9, EXTI 12~15 , 스위치RTSR , 조이스틱이 움직이자마자 반응해야하기에 RTSR설정
  EXTI->IMR  |= 0x73C0; //   초기 EXTI 12 (SW4) ,EXTI 14(SW6) 만 mask Enable해주기위해 
  
  NVIC->ISER[0] |= ( 1<<23 );   // 0x00800000 Enable 'Global Interrupt EXTI9_5'
  NVIC->ISER[1] |= ( 1<<8 );    // (1<<(40-32)  'Global Interrupt EXTI15_10' , 
}

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{      
  if(EXTI->PR & 0x0020) { // EXTI5 Interrupt Pending(발생) 여부? NAVI_PUSH
	EXTI->PR &= 0x0020; // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생) 기록지 지울려면 1사용.
	
	SW4_Flag=0; // SW4_Flag를 0 으로 만들어 OP2 확정가능하게함
	StartFlag=0; // StartFlag를 0으로 만들어 OP2 입력가능하게함
	EXTI->IMR &=~0x0020; // 한번 PUSH 사용후 disable 하여 push 사용 못하도록함 , main에서 joy scan으로 Error 받을 수 있게함
	Joy_Flag++; // SW4와 Joy_Push에서 변수 조작을 통하여 SW4가 한번 눌렸을때만 Error발생안하도록하고 이후엔 다 Error발생
	BEEP(); 
	LCD_SetTextColor(RGB_RED); 
	if(CheckOP==1)  {// #, OR 
	  Or++; // Or 연산 사용횟수 증가
	  if(Or==10) {Or=0;} // Or 연산 횟수가 10회 넘어가면 다시 0회
	  Fram_Write(527,Or); // 527번지에 Or변수 값 저장
	  LCD_DisplayChar(7,13,0x30 + Or);  // Or 사용횟수 보이기
	}
	else if(CheckOP==2) { // & , AND
	  And++; // And 연산 사용횟수 증가
	  if(And==10) {And=0;} // And 연산 횟수가 10회 넘어가면 다시 0회로
	  Fram_Write(528,And); // 528번지에 And 변수 값 저장
	  LCD_DisplayChar(7,18,0x30 + And); // And 사용횟수 보이기
	}
	else if(CheckOP==3){  // - ,SUB
	  Sub++; // Sub 연산 사용횟수 증가
	  if(Sub==10) {Sub=0;} // Sub 연산 횟수 10회 넘어가면 다시 0회
	  Fram_Write(526,Sub); // 526번지에 Sub변수 값 저장
	  LCD_DisplayChar(7,8,0x30 + Sub);  // Sub 사용횟수 보이기
	} 
	else if(CheckOP==4){  // + ,ADD
	  Add++;  // Add 연산 사용횟수 증가
	  if(Add==10) {Add=0;} // And 연산 횟수 10회 넘어가면 다시 0회
	  Fram_Write(525,Add); // 525번지에 Add 변수 값 저장하기
	  LCD_DisplayChar(7,3,0x30 + Add);  // Add 사용횟수 보이기
	}
  }
  else if(EXTI->PR & 0x0040){ // EXTI6 Interrupt Pending(발생) 여부? NAVI_UP
	EXTI->PR &= 0x0040; // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생) 기록지 지울려면 1사용.
	if(Joy_Flag==1){ // SW4가 1번 눌리면 Joy_Flag가 1이됨으로 SW4가 한번 눌린 후에만 Operator설정가능, Joy_Push 에 Joy_Flag증가시켜 Push이후에 사용불가
	  LCD_DisplayChar(4, 6 ,'#'); //  # Operator
	  EXTI->IMR |= 0x0020; // Joy_Push가 Operator 선택을 해야 Enable되도록 통제
	  CheckOP=1; // SW5에서 어떤 연산할지 알려주는 변수
	  PushError++;
	}
	else if(Joy_Flag != 1) { ErrorCheck(); } // Operand1이 결정된 이후 (SW4가 눌린이후) 에만 사용후 나머지 경우는 Error발생하도록함
  }
  else if(EXTI->PR & 0x0080) { // EXTI7 Interrupt Pending(발생) 여부? NAVI_DOWN
	EXTI->PR &= 0x0080; // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생) 기록지 지울려면 1사용.
	if(Joy_Flag==1){// SW4가 1번 눌리면 Joy_Flag가 1이됨으로 SW4가 한번 눌린 후에만 Operator설정가능, Joy_Push 에 Joy_Flag증가시켜 Push이후에 사용불가
	  LCD_DisplayChar(4, 6 ,'&'); //  & Operator
	  EXTI->IMR |= 0x0020;  // Joy_Push가 Operator 선택을 해야 Enable되도록 통제
	  CheckOP=2;// SW5에서 어떤 연산할지 알려주는 변수
	  PushError++;
	}
	else if(Joy_Flag != 1) { ErrorCheck(); } // Operand1이 결정된 이후 (SW4가 눌린이후) 에만 사용후 나머지 경우는 Error발생하도록함
	
  }
  else if(EXTI->PR & 0x0100) {    // EXTI8 Interrupt Pending(발생) 여부? NAVI_RIGHT
	EXTI->PR &= 0x0100;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생) 기록지 지울려면 1사용.
	if(Joy_Flag==1){// SW4가 1번 눌리면 Joy_Flag가 1이됨으로 SW4가 한번 눌린 후에만 Operator설정가능, Joy_Push 에 Joy_Flag증가시켜 Push이후에 사용불가
	  LCD_DisplayChar(4, 6 ,'-'); //  - Operator
	  EXTI->IMR |= 0x0020;  // Joy_Push가 Operator 선택을 해야 Enable되도록 통제
	  CheckOP=3;// SW5에서 어떤 연산할지 알려주는 변수
	  PushError++;
	}
	else if(Joy_Flag != 1) { ErrorCheck(); } // Operand1이 결정된 이후 (SW4가 눌린이후) 에만 사용후 나머지 경우는 Error발생하도록함
	
  }
  else if(EXTI->PR & 0x0200){    // EXTI9 Interrupt Pending(발생) 여부? NAVI_LEFT
	EXTI->PR |= 0x0200;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
	if(Joy_Flag==1){// SW4가 1번 눌리면 Joy_Flag가 1이됨으로 SW4가 한번 눌린 후에만 Operator설정가능, Joy_Push 에 Joy_Flag증가시켜 Push이후에 사용불가
	  LCD_DisplayChar(4, 6 ,'+'); //  + Operator
	  EXTI->IMR |= 0x0020; // Joy_Push가 Operator 선택을 해야 Enable되도록 통제
	  CheckOP=4;// SW5에서 어떤 연산할지 알려주는 변수
	  PushError++;
	}
	else if(Joy_Flag != 1) { ErrorCheck(); } // Operand1이 결정된 이후 (SW4가 눌린이후) 에만 사용후 나머지 경우는 Error발생하도록함
  }
}

void EXTI15_10_IRQHandler(void)
{
  if(EXTI->PR & 0x8000)         // EXTI 15 Interrupt Pending(발생) 여부? SW7 (Enter)
  {
	EXTI->PR |= 0x8000;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생) 기록지 지울려면 1사용.
	for(int i=2; i<17; i++){
	  LCD_DisplayChar(4, i,' '); // 연산식 쓰는곳 Clear
	}
	SW4_Flag=0,SW5_Flag=0,Joy_Flag=0,StartFlag=0,Next=0; //변수 전부 초기설정
	GPIOG->ODR &= ~0x00FF;// LED0~7 OFF
  }
  else if(EXTI->PR & 0x4000)      // EXTI14 Interrupt Pending(발생) 여부? SW6
  {
	EXTI->PR |= 0x4000;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)  
	for(int i=0; i<4;i++){
	  Fram_Write(525+i,0);
	} // 525~528번지에 0 대입
	LCD_SetTextColor(RGB_RED); // 글자색 : 빨간색 
	LCD_DisplayChar(7,3,0x30 +Fram_Read(525)); 
	LCD_DisplayChar(7,8,0x30 + Fram_Read(526));
	LCD_DisplayChar(7,13,0x30 + Fram_Read(527));
	LCD_DisplayChar(7,18,0x30 + Fram_Read(528)); // 0 표시하기
  }
  else if(EXTI->PR & 0x2000) {      // EXTI13 Interrupt Pending(발생) 여부? SW5
	EXTI->PR |= 0x2000;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)   
	if(SW5_Flag==2){ // SW4에서 ++시킴을 통해 SW4가 두번 눌린 이후에만 SW5사용가능하게함 
	  LCD_SetTextColor(RGB_BLACK);  // 글자색 : 검정색
	  LCD_DisplayChar(4, 11 ,'='); // ' = ' 연산자 화면에 표시
	  if(CheckOP==1)  {// #, OR  
		GetOr();  // OR연산 함수사용
		CheckOP=0; // 변수 초기화
	  }
	  else if(CheckOP==2) { // & , AND
		GetAnd(); //AND연산 함수사용
		CheckOP=0; // 변수초기화
	  }
	  else if(CheckOP==3){  // - ,SUB
		GetSub();  // SUB연산 함수사용
		CheckOP=0; //변수초기화
	  } 
	  else if(CheckOP==4){  // - ,ADD
		GetAdd(); // Add연산 함수사용 
		CheckOP=0; // 변수초기화
	  }
	  SW5_Flag = 0; // SW5가 한번 사용 한 뒤 다시 사용하지 못하도록함
	}
	else { ErrorCheck(); } // SW5는 operand2를 받고나서만 사용하도록함.즉, SW4가 두번 사용 되면 SW5사용가능 ,나머지 경우는 Error발생하도록함

  }
  else if(EXTI->PR & 0x1000)      // EXTI12 Interrupt Pending(발생) 여부? SW4
  {
	EXTI->PR |= 0x1000;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
	if(SW4_Flag==0){  //SW4_Flag가 0일때만 들어오도록 통제, 해당 코딩에선 2번밖에 입력 불가하도록 코딩함
	  BEEP(); 
	  BitCheak(); // Operand1인지 Operand2인지 구분하여 저장해두는 함수
	  GPIOG->ODR &= ~0x00FF; // LED0~7 OFF
	  EXTI->IMR |= 0xF3C0; // 인터럽트 EXTI 15~12, EXTI 6~9 enable 
	  SW4_Flag = 1; // SW4_Flag 중복방지
	  SW5_Flag++; // SW4 인터럽트는 총 2번밖에 안온다. 따라서 SW5_Flag의 최종값은 2가된다.따라서 SW5_Flag를 2일때만 체크.
	  Joy_Flag++; // Joy는 SW4가 1번 한 이후에는 다 Error이다. 따라서 한번 눌러 1 증가시켜 1이외에는 Error상황
	  StartFlag=1; // StartFlag를다시 1로만들어 Opernad입력못하게함
	  Next=5; // Opernad1,2구분하기위한 변수 설정
	}  
	else if(SW4_Flag==1){ ErrorCheck(); } // SW4_Flag를 Handler에서 조작하여, SW4(Operand확정)가 눌리는 경우 제외하고 나머지눌리면 Error발생하도록함

  }
}
/* Switch가 입력되었는지 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
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
/* Buzzer: Beep for 30 ms */
void BEEP(void)         
{    
  GPIOF->ODR |=  0x0200;   // PF9 'H' Buzzer on
  DelayMS(1);      // Delay 30 ms
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
void BitCheak(void)
{
  LCD_SetTextColor(RGB_BLACK); // 글자색 : 검정색
  if(Next==0){ // Operand1 결정
	if(GPIOG->IDR & 0x0001) {LCD_DisplayChar(4, 2 ,'1');  arr1[1]=1; }  //  LED0 ON이면, 1표시하도록하고, 배열에 1넣기
	else {LCD_DisplayChar(4, 2 ,'0');  arr1[1]=0;  }  // LED0 OFF이면, 0표시하도록하고, 배열에 0 넣기
	
	if(GPIOG->IDR & 0x0002) {LCD_DisplayChar(4, 3,'1');  arr1[2]=1; }  //  LED1 ON이면, 1표시하도록하고, 배열에 1넣기
	else {LCD_DisplayChar(4, 3 ,'0');  arr1[2]=0; } // LED1 OFF이면, 0표시하도록하고, 배열에 0 넣기
	
	if(GPIOG->IDR & 0x0004) {LCD_DisplayChar(4, 4 ,'1'); arr1[3]=1; }  //  LED2 ON이면, 1표시하도록하고, 배열에 1넣기
	else {LCD_DisplayChar(4, 4 ,'0'); arr1[3]=0; } // LED2 OFF이면, 0표시하도록하고, 배열에 0 넣기
	
	if(GPIOG->IDR & 0x0008) {LCD_DisplayChar(4, 5,'1'); arr1[4]=1; }  //  LED3 ON이면, 1표시하도록하고, 배열에 1넣기
	else {LCD_DisplayChar(4, 5 ,'0');  arr1[4]=0; } // LED3 OFF이면, 0표시하도록하고, 배열에 0 넣기
  }
  else if(Next==5){// Operand2 결정
	if(GPIOG->IDR & 0x0001) {LCD_DisplayChar(4, 2+Next ,'1'); arr2[1]=1; }  //  LED0 ON이면, 1표시하도록하고, 배열에 1넣기
	else {LCD_DisplayChar(4, 2+Next ,'0'); arr2[1]=0; } // LED0 OFF이면, 0표시하도록하고, 배열에 0 넣기
	
	if(GPIOG->IDR & 0x0002) {LCD_DisplayChar(4, 3+Next ,'1'); arr2[2]=1; } //  LED1 ON이면, 1표시하도록하고, 배열에 1넣기
	else {LCD_DisplayChar(4, 3+Next ,'0');  arr2[2]=0; } // LED1 OFF이면, 0표시하도록하고, 배열에 0 넣기
	
	if(GPIOG->IDR & 0x0004) {LCD_DisplayChar(4, 4+Next ,'1'); arr2[3]=1; } //  LED2 ON이면, 1표시하도록하고, 배열에 1넣기
	else {LCD_DisplayChar(4, 4+Next ,'0'); arr2[3]=0; } // LED2 OFF이면, 0표시하도록하고, 배열에 0 넣기
	
	if(GPIOG->IDR & 0x0008) {LCD_DisplayChar(4, 5+Next,'1'); arr2[4]=1; } //  LED3 ON이면, 1표시하도록하고, 배열에 1넣기
	else {LCD_DisplayChar(4, 5+Next ,'0');  arr2[4]=0; } // LED3 OFF이면, 0표시하도록하고, 배열에 0 넣기
  } 
}
void SetAdd(void)
{
  Carry = 0 ; // 초기에는 두 2진수만 더해야하기때문에 Carry값 강제 0 
  for(int i=4; i>=0; i--){
	ResultArr[i]= arr1[i]+arr2[i]+Carry; // 두 이진수를 더하고 캐리 발생시 캐리까지 더해줌
	if(ResultArr[i]==0) { ResultArr[i]=0; Carry=0;  } //    0 일경우 해당 2진수 0,  carry0
	else if(ResultArr[i]==1) { ResultArr[i]=1; Carry=0;}  // 1일 경우 해당 2진수 1, Carry 0
	else if(ResultArr[i]==2) { ResultArr[i]=0, Carry=1;  } // 2일경우 해당2진수 0, Carry 1
	else if(ResultArr[i]==3) { ResultArr[i]=1, Carry=1;  } // 3일경우 해당 2진수 1, Carry 1
  }  // 두 2진수를 더하는 과정
}
void GetResultText(int array[])  // 계산결과 LCD에 표현하는 함수 
{n
  for(int i=1; i<5; i++){
	LCD_DisplayChar(4, 12+i ,0x30+array[i]);  // 받아온 결과값을 차례대로 LCD에 표시
  }
}
void GetAdd(void)
{
  LCD_SetTextColor(RGB_BLACK); //글자색 : 검정색 
  SetAdd();  //더하기 함수
  
  if(ResultArr[0]==0){
	GetResultText(ResultArr);
  } // 캐리 발생없으면 그대로 4bit표시
  else if(ResultArr[0]==1) {
	LCD_DisplayChar(4, 12 ,0x30+ResultArr[0]); // 5bit이기에 최상위 비트 출력
	GetResultText(ResultArr);
  } // 최상위 비트에 캐리발생시 1표시후 나머지 2진결과 표시 5bit
}
void Complement(void)
{
  OneComlement(arr2,1); // 1의 보수  
  Carry=1; // 1더하기를 위해 초기 Carry 1로 설정
  for(int i = 4; i >= 0; i--){
	arr2[i]+=Carry; 
	if(arr2[i]==0) { arr2[i]=0,Carry=0;} // 0이면, 해당 2진수 0, Carry 0
	else if(arr2[i]==1) {arr2[i]=1, Carry=0;} // 1이면, 해당 2진수 1 , Carry 0
	else if(arr2[i]==2) {arr2[i]=0 ,Carry=1;} // 2가되면, 해당 2진수 0 , Carry 1
	
  }  // 1더하기 과정 
}
void OneComlement(int array[],int Set) 
{
  for(int i=4; i>=Set; i--)
  {
	if(array[i]==0){array[i]=1;}
	else if(array[i]==1){array[i]=0;}
  } // 1의 보수 , 0이면 1로 1이면 0으로 바꿔주는 과정
}
void GetSub(void)
{
  LCD_SetTextColor(RGB_BLACK); // 글자색 : 검은색
  Complement(); //2의보수
  SetAdd(); // 2개의 받아온거 덧셈
  if(ResultArr[0]==1) // 최상위 캐리 발생시 양수이기에 최상위 캐리 버리고 4자리만 사용
  { 
	GetResultText(ResultArr); // 결과 출력 함수 
  }
  else if(ResultArr[0]==0) // 최상위 값이 0이면 무조건 음수, 따라서 결과값에 2의보수 
  {
	// 2의보수 과정
	OneComlement(ResultArr,0); // 1의보수     
	Carry=1; //1을 더해야하기에  Carry변수 1설정
	for(int j=4; j > 0; j--){
	  ResultArr[j]+=Carry; 
	  if( ResultArr[j]==0) { ResultArr[j]=0,Carry=0;} // 0이면 2진수 0, Carry0
	  else if(ResultArr[j]==1) {ResultArr[j]=1, Carry=0;} // 1이면 2진수 1, Carry 0
	  else if(ResultArr[j]==2) {ResultArr[j]=0 ,Carry=1;} //  2이면 2진수 0, Carry 1 
	} // 1의 보수 이후 +1 해주는과정,  최상위 캐리발생은 무시
	LCD_DisplayChar(4, 12 ,'-'); // 결과가 음수이기에 - 붙여줌
	GetResultText(ResultArr);   // 결과값 출력
  }
}
void GetOr(void)
{
  for(int i =1; i<5 ; i++) {
	if(arr1[i] | arr2[i]) { ResultArr[i]=1; } 
	else { ResultArr[i]=0; }
  } // 2진수 각 자리끼리 Or 비교후, 1이면 결과 1, 0이면 결과 0
  LCD_SetTextColor(RGB_BLACK); // 글자색 : 검은색
  GetResultText(ResultArr); // 결과 비트 출력함수
}
void GetAnd(void) // AND 연산 함수
{
  for(int i =1; i<5;i++){
	if(arr1[i] & arr2[i]) { ResultArr[i]=1; }
	else { ResultArr[i]=0; }
  } // 2진수의 각 자리끼리 AND 비교후, 1이면 결과 1,  0이면 결과 0으로  
  LCD_SetTextColor(RGB_BLACK); // 글자색 : 검은색 
  GetResultText(ResultArr); // 결과 비트 출력함수 
}
void ErrorCheck(void)
{
  LCD_SetBrushColor(RGB_RED); // 브러쉬 : RED
  LCD_DrawFillRect(141,13,8,8);   // Error 사각형 회색으로 채우기
  DelayMS(2000);
  LCD_SetBrushColor(GET_RGB(195,195,195)); // 브러쉬 : GRAY 
  LCD_DrawFillRect(141,13,8,8);   // Error 사각형 회색으로 채우기
  BEEP();
  DelayMS(500);
  BEEP();
}