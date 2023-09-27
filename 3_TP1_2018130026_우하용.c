/////////////////////////////////////////////////////////////
// ������:  ������ ����
// ��������: ���ڸ� 2���� ���ڸ� ADD(+), SUB(-), OR(#), AND(&) 
//               �����ϴ� ���⸦ ����
//               ù �ʱ�ȭ�鿡���� ������ Operand1�� �������� �����Ͽ�, '=', Joy �Է½� Error�߻��ϵ�����
//               Operand1 Ȯ�� �� ���ڳ�,'=', "Joy_Push(Operator ���þȵƴµ� Ȯ���������)" �� ������ Error����
//               Operator Ȯ�� �� Joy�Է��ϰų� '='�Է½� Error �߻�
//               Operand2 Ȯ�� �� Joy�Է��ϰų� ���ڳ� OperandȮ�� ������ Error�߻��ϵ����� 
//               ��� ��� ���� ���ڳ�,OperandȮ��, '=' , Joy ������ ������������       
// ����� �ϵ����(���): GPIO, Joy-stick, EXTI, GLCD, Key-Scan, LED, Buzzer,  
// ������: 2022. 06. 03
// ������ Ŭ����: ����Ϲ�
// �й�: 2018130026
// �̸�: ���Ͽ�
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
void GetSub(void); // ���� �Լ�
void Complement(void); // 2�Ǻ��� �Լ�
void GetAdd(void); // �����Լ�(���� ���� �Ѵٻ��)
void SetAdd(void); // ���� �� �ɸ�üũ 
void BitCheak(void); // 2�� ���� üũ
void GetOr(void); // OR�Լ�
void GetAnd(void); // AND �Լ�
void ErrorCheck(void); // Error ǥ���Լ�
void OneComlement(int array[],int Set); // 1�Ǻ��� �Լ�
void GetResultText(int array[]); //�������Լ�

uint8 Next=0; // ȭ�鿡 ���̴� 4���� 2������ operator������ operator ���� 2���� �������� �Ѿ������ ����
uint8 SW4_Flag=0,SW5_Flag=0,Joy_Flag=0,StartFlag=0,PushError=0; // �����ʱ�ȭ
int arr1[5]; //operand1 ��� ���� �迭
int arr2[5]; //operand2 ��� ���� �迭
int ResultArr[5]; // ���� ��� ��� ���� �迭 
uint8 Carry=0; //������� Carry ����
uint8 CheckOP=0; // Operator ���к���
uint8 Add=0,Sub=0,Or=0,And=0; // ����� Operator Ƚ�� ����

int main(void){
  
  LCD_Init();   // LCD ��� �ʱ�ȭ
  DelayMS(100);
  _GPIO_Init();   // GPIO �ʱ�ȭ
  DelayMS(100);
  BEEP(); 
  _EXTI_Init();   // EXTI �ʱ�ȭ
  Fram_Init();                    // FRAM �ʱ�ȭ H/W �ʱ�ȭ
  Fram_Status_Config();   // FRAM �ʱ�ȭ S/W �ʱ�ȭ
  
  Add=Fram_Read(525);  // 525������ ����� �� Add�� �о����
  Sub=Fram_Read(526);  // 526������ ����� �� Add�� �о����
  Or=Fram_Read(527);   // 527������ ����� �� Add�� �о����
  And=Fram_Read(528); // 528������ ����� �� Add�� �о����
  
  DisplayInitScreen();   // LCD �ʱ�ȭ��
  GPIOG->ODR &= ~0x00FF;   // �ʱⰪ: LED0~7 Off
  
  while(1)
  {
	switch(KEY_Scan()){  
	case SW0_PUSH:      
	  if(StartFlag==0){
		if(GPIOG->ODR & 0x0001){// LED0 ON�϶�
		  GPIOG->BSRRH = 0x0001; //LED0 OFF        
		}
		else {
		  GPIOG->BSRRL = 0x0001; // LED0 ON
		}
		break;
	  }
	  else { ErrorCheck(); break;} // StartFlag������ SW4(OperandȮ��)���� �����Ͽ�, �̿� �Է¿��� Error�� �ǵ�����
	  break;
	case SW1_PUSH:
	  if(StartFlag==0){
		if(GPIOG->ODR & 0x0002){// LED1 ON�϶�
		  GPIOG->BSRRH=0x0002;  //LED1 OFF 
		}
		else {
		  GPIOG->BSRRL = 0x0002; // LED1 ON    
		}
		break;
	  }
	  else { ErrorCheck(); break;} // StartFlag������ SW4(OperandȮ��)���� �����Ͽ�, �̿� �Է¿��� Error�� �ǵ�����
	  break;
	case SW2_PUSH:
	  if(StartFlag==0){
		if(GPIOG->ODR & 0x0004){// LED2 ON�϶�
		  GPIOG->BSRRH=0x0004;  //LED2 OFF  
		}   
		else {   
		  GPIOG->BSRRL = 0x0004; //LED2 ON
		}
		break;
	  }
	  else { ErrorCheck(); break;} // StartFlag������ SW4(OperandȮ��)���� �����Ͽ�, �̿� �Է¿��� Error�� �ǵ�����
	  break;
	case SW3_PUSH:
	  if(StartFlag==0){
		if(GPIOG->ODR & 0x0008){  // LED3 ON�϶�     
		  GPIOG->BSRRH=0x0008;   //LED3 OFF
		}
		else {
		  GPIOG->BSRRL = 0x0008; //LED3 ON
		}
		break;
	  }
	  else { ErrorCheck(); break; } // StartFlag������ SW4(OperandȮ��)���� �����Ͽ�, �̿� �Է¿��� Error �߻��ϵ�����
	  break;
	
	} //switch key
	switch (JOY_Scan())
	{
	case NAVI_PUSH:
	  if(Joy_Flag != 1 || PushError==0 ) { ErrorCheck(); } // Operand1�� ������ ���� (SW4�� ��������) ���� ����� ������ ���� Error�߻��ϵ�����
	  break;
	}//Switch Joy_Scan()
  } //while 
}// main

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
  LCD_Clear(RGB_YELLOW);          // ȭ�� ��������� Ŭ����
  LCD_SetFont(&Gulim7);          // ��Ʈ : ���� 7
  
  //�ʱ� ȭ�� �ʷϻ簢�� Ʋ �׸���
  LCD_SetPenColor(RGB_GREEN);      //  ����� : Green   
  LCD_DrawRectangle(3, 3, 152, 120);   //  ū �׵θ��簢�� �׸���
  LCD_DrawRectangle(5, 40, 148, 20);   //  2���� ��� �簢�� �׸���
  
  //���� �̴ϼ� 
  LCD_DrawRectangle(7, 5, 93, 20);   //  ���� �̴ϼ� �簢�� �׸���
  LCD_SetBackColor(RGB_BLUE);   // ���ڹ��� : BLUE
  LCD_SetTextColor(RGB_WHITE);   // ���ڻ� : WHITE
  LCD_SetBrushColor(RGB_BLUE); // �귯�� : BLUE
  LCD_DrawFillRect(8,6,92,19);   // �����̴ϼ� �簢�� �Ķ������� ä���
  LCD_DrawText(9,12,"WHY Calculator");  //  WHY Calculator  ȭ��ǥ��
  //Error�� �簢��
  LCD_DrawRectangle(140, 12, 9, 9);  // Error ǥ�ùڽ�
  LCD_SetBrushColor(GET_RGB(195,195,195)); // �귯�� : gray
  LCD_DrawFillRect(141,13,8,8);   // Error �簢�� ȸ������ ä���
  LCD_SetTextColor(RGB_BLACK);   // ���ڻ� : BLACK
  LCD_SetBackColor(RGB_YELLOW);   // ���ڹ��� : YELLOW
  LCD_DrawText(105, 12, "Error");     //  Error ȭ��ǥ��
  
  LCD_DisplayChar(4, 1 ,'>'); // ���� �簢�� > ǥ��
  
  LCD_DisplayText(7,1,"+:0, -:0, #:0, &:0"); //������ ǥ��
  LCD_SetTextColor(RGB_RED);
  // DisplayInitScreen �� Fram_Read�� ������ ��� �� �� ����Ͽ�(main�� �ʱ�), ȭ�鿡 Fram���� ������ ������ ǥ�õǵ�����
  LCD_DisplayChar(7,3,0x30 + Add);  
  LCD_DisplayChar(7,8,0x30 + Sub);
  LCD_DisplayChar(7,13,0x30 + Or);
  LCD_DisplayChar(7,18,0x30 + And); // Operator ���Ƚ�� ǥ���ϱ� 
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) �ʱ� ����   */
void _GPIO_Init(void)
{  // LED (GPIO G) ���� : Output mode
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

void _EXTI_Init(void)
{
  RCC->AHB1ENR    |= 0x00000100; // (1<<8)   RCC_AHB1ENR GPIOI Enable
  RCC->AHB1ENR   |=  0x00000080;   // (1<<7) RCC_AHB1ENR : GPIOH(bit#7) Enable    
  RCC->APB2ENR    |= 0x00004000;   // (1<<14)Enable System Configuration Controller Clock
  
  GPIOI->MODER    &= ~0x000FFC00;   // GPIOI 5~9 : Input mode (reset state)
  GPIOH->MODER    &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state) 
  
  SYSCFG->EXTICR[1] |= 0x8880;   // EXTI5<-PI5,EXTI6<-PI6,EXTI7 <-PI7 , NAVI_PUSH, NAVI_UP, NAVI_DOWN ���ͷ�Ʈ ����
  SYSCFG->EXTICR[2] |= 0x0088; // EXTI8 <-PI8, EXTI9<-PI9,NAVI_RIGHT,NAVI_LEFT ���ͷ�Ʈ����
  SYSCFG->EXTICR[3] |= 0x7777; // EXTI 12 <-PH12,EXTI 13 <-PH13,EXTI 14 <-PH14,EXTI 15 <-PH15, SW4,SW5,SW6,SW7 ���ͷ�Ʈ����
  
  EXTI->FTSR |=  0xF3E0; //  EXTI5~9, EXTI 12~15 , ����ġRTSR , ���̽�ƽ�� �������ڸ��� �����ؾ��ϱ⿡ RTSR����
  EXTI->IMR  |= 0x73C0; //   �ʱ� EXTI 12 (SW4) ,EXTI 14(SW6) �� mask Enable���ֱ����� 
  
  NVIC->ISER[0] |= ( 1<<23 );   // 0x00800000 Enable 'Global Interrupt EXTI9_5'
  NVIC->ISER[1] |= ( 1<<8 );    // (1<<(40-32)  'Global Interrupt EXTI15_10' , 
}

/* EXTI5~9 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{      
  if(EXTI->PR & 0x0020) { // EXTI5 Interrupt Pending(�߻�) ����? NAVI_PUSH
	EXTI->PR &= 0x0020; // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�) ����� ������� 1���.
	
	SW4_Flag=0; // SW4_Flag�� 0 ���� ����� OP2 Ȯ�������ϰ���
	StartFlag=0; // StartFlag�� 0���� ����� OP2 �Է°����ϰ���
	EXTI->IMR &=~0x0020; // �ѹ� PUSH ����� disable �Ͽ� push ��� ���ϵ����� , main���� joy scan���� Error ���� �� �ְ���
	Joy_Flag++; // SW4�� Joy_Push���� ���� ������ ���Ͽ� SW4�� �ѹ� ���������� Error�߻����ϵ����ϰ� ���Ŀ� �� Error�߻�
	BEEP(); 
	LCD_SetTextColor(RGB_RED); 
	if(CheckOP==1)  {// #, OR 
	  Or++; // Or ���� ���Ƚ�� ����
	  if(Or==10) {Or=0;} // Or ���� Ƚ���� 10ȸ �Ѿ�� �ٽ� 0ȸ
	  Fram_Write(527,Or); // 527������ Or���� �� ����
	  LCD_DisplayChar(7,13,0x30 + Or);  // Or ���Ƚ�� ���̱�
	}
	else if(CheckOP==2) { // & , AND
	  And++; // And ���� ���Ƚ�� ����
	  if(And==10) {And=0;} // And ���� Ƚ���� 10ȸ �Ѿ�� �ٽ� 0ȸ��
	  Fram_Write(528,And); // 528������ And ���� �� ����
	  LCD_DisplayChar(7,18,0x30 + And); // And ���Ƚ�� ���̱�
	}
	else if(CheckOP==3){  // - ,SUB
	  Sub++; // Sub ���� ���Ƚ�� ����
	  if(Sub==10) {Sub=0;} // Sub ���� Ƚ�� 10ȸ �Ѿ�� �ٽ� 0ȸ
	  Fram_Write(526,Sub); // 526������ Sub���� �� ����
	  LCD_DisplayChar(7,8,0x30 + Sub);  // Sub ���Ƚ�� ���̱�
	} 
	else if(CheckOP==4){  // + ,ADD
	  Add++;  // Add ���� ���Ƚ�� ����
	  if(Add==10) {Add=0;} // And ���� Ƚ�� 10ȸ �Ѿ�� �ٽ� 0ȸ
	  Fram_Write(525,Add); // 525������ Add ���� �� �����ϱ�
	  LCD_DisplayChar(7,3,0x30 + Add);  // Add ���Ƚ�� ���̱�
	}
  }
  else if(EXTI->PR & 0x0040){ // EXTI6 Interrupt Pending(�߻�) ����? NAVI_UP
	EXTI->PR &= 0x0040; // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�) ����� ������� 1���.
	if(Joy_Flag==1){ // SW4�� 1�� ������ Joy_Flag�� 1�̵����� SW4�� �ѹ� ���� �Ŀ��� Operator��������, Joy_Push �� Joy_Flag�������� Push���Ŀ� ���Ұ�
	  LCD_DisplayChar(4, 6 ,'#'); //  # Operator
	  EXTI->IMR |= 0x0020; // Joy_Push�� Operator ������ �ؾ� Enable�ǵ��� ����
	  CheckOP=1; // SW5���� � �������� �˷��ִ� ����
	  PushError++;
	}
	else if(Joy_Flag != 1) { ErrorCheck(); } // Operand1�� ������ ���� (SW4�� ��������) ���� ����� ������ ���� Error�߻��ϵ�����
  }
  else if(EXTI->PR & 0x0080) { // EXTI7 Interrupt Pending(�߻�) ����? NAVI_DOWN
	EXTI->PR &= 0x0080; // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�) ����� ������� 1���.
	if(Joy_Flag==1){// SW4�� 1�� ������ Joy_Flag�� 1�̵����� SW4�� �ѹ� ���� �Ŀ��� Operator��������, Joy_Push �� Joy_Flag�������� Push���Ŀ� ���Ұ�
	  LCD_DisplayChar(4, 6 ,'&'); //  & Operator
	  EXTI->IMR |= 0x0020;  // Joy_Push�� Operator ������ �ؾ� Enable�ǵ��� ����
	  CheckOP=2;// SW5���� � �������� �˷��ִ� ����
	  PushError++;
	}
	else if(Joy_Flag != 1) { ErrorCheck(); } // Operand1�� ������ ���� (SW4�� ��������) ���� ����� ������ ���� Error�߻��ϵ�����
	
  }
  else if(EXTI->PR & 0x0100) {    // EXTI8 Interrupt Pending(�߻�) ����? NAVI_RIGHT
	EXTI->PR &= 0x0100;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�) ����� ������� 1���.
	if(Joy_Flag==1){// SW4�� 1�� ������ Joy_Flag�� 1�̵����� SW4�� �ѹ� ���� �Ŀ��� Operator��������, Joy_Push �� Joy_Flag�������� Push���Ŀ� ���Ұ�
	  LCD_DisplayChar(4, 6 ,'-'); //  - Operator
	  EXTI->IMR |= 0x0020;  // Joy_Push�� Operator ������ �ؾ� Enable�ǵ��� ����
	  CheckOP=3;// SW5���� � �������� �˷��ִ� ����
	  PushError++;
	}
	else if(Joy_Flag != 1) { ErrorCheck(); } // Operand1�� ������ ���� (SW4�� ��������) ���� ����� ������ ���� Error�߻��ϵ�����
	
  }
  else if(EXTI->PR & 0x0200){    // EXTI9 Interrupt Pending(�߻�) ����? NAVI_LEFT
	EXTI->PR |= 0x0200;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
	if(Joy_Flag==1){// SW4�� 1�� ������ Joy_Flag�� 1�̵����� SW4�� �ѹ� ���� �Ŀ��� Operator��������, Joy_Push �� Joy_Flag�������� Push���Ŀ� ���Ұ�
	  LCD_DisplayChar(4, 6 ,'+'); //  + Operator
	  EXTI->IMR |= 0x0020; // Joy_Push�� Operator ������ �ؾ� Enable�ǵ��� ����
	  CheckOP=4;// SW5���� � �������� �˷��ִ� ����
	  PushError++;
	}
	else if(Joy_Flag != 1) { ErrorCheck(); } // Operand1�� ������ ���� (SW4�� ��������) ���� ����� ������ ���� Error�߻��ϵ�����
  }
}

void EXTI15_10_IRQHandler(void)
{
  if(EXTI->PR & 0x8000)         // EXTI 15 Interrupt Pending(�߻�) ����? SW7 (Enter)
  {
	EXTI->PR |= 0x8000;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�) ����� ������� 1���.
	for(int i=2; i<17; i++){
	  LCD_DisplayChar(4, i,' '); // ����� ���°� Clear
	}
	SW4_Flag=0,SW5_Flag=0,Joy_Flag=0,StartFlag=0,Next=0; //���� ���� �ʱ⼳��
	GPIOG->ODR &= ~0x00FF;// LED0~7 OFF
  }
  else if(EXTI->PR & 0x4000)      // EXTI14 Interrupt Pending(�߻�) ����? SW6
  {
	EXTI->PR |= 0x4000;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)  
	for(int i=0; i<4;i++){
	  Fram_Write(525+i,0);
	} // 525~528������ 0 ����
	LCD_SetTextColor(RGB_RED); // ���ڻ� : ������ 
	LCD_DisplayChar(7,3,0x30 +Fram_Read(525)); 
	LCD_DisplayChar(7,8,0x30 + Fram_Read(526));
	LCD_DisplayChar(7,13,0x30 + Fram_Read(527));
	LCD_DisplayChar(7,18,0x30 + Fram_Read(528)); // 0 ǥ���ϱ�
  }
  else if(EXTI->PR & 0x2000) {      // EXTI13 Interrupt Pending(�߻�) ����? SW5
	EXTI->PR |= 0x2000;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)   
	if(SW5_Flag==2){ // SW4���� ++��Ŵ�� ���� SW4�� �ι� ���� ���Ŀ��� SW5��밡���ϰ��� 
	  LCD_SetTextColor(RGB_BLACK);  // ���ڻ� : ������
	  LCD_DisplayChar(4, 11 ,'='); // ' = ' ������ ȭ�鿡 ǥ��
	  if(CheckOP==1)  {// #, OR  
		GetOr();  // OR���� �Լ����
		CheckOP=0; // ���� �ʱ�ȭ
	  }
	  else if(CheckOP==2) { // & , AND
		GetAnd(); //AND���� �Լ����
		CheckOP=0; // �����ʱ�ȭ
	  }
	  else if(CheckOP==3){  // - ,SUB
		GetSub();  // SUB���� �Լ����
		CheckOP=0; //�����ʱ�ȭ
	  } 
	  else if(CheckOP==4){  // - ,ADD
		GetAdd(); // Add���� �Լ���� 
		CheckOP=0; // �����ʱ�ȭ
	  }
	  SW5_Flag = 0; // SW5�� �ѹ� ��� �� �� �ٽ� ������� ���ϵ�����
	}
	else { ErrorCheck(); } // SW5�� operand2�� �ް����� ����ϵ�����.��, SW4�� �ι� ��� �Ǹ� SW5��밡�� ,������ ���� Error�߻��ϵ�����

  }
  else if(EXTI->PR & 0x1000)      // EXTI12 Interrupt Pending(�߻�) ����? SW4
  {
	EXTI->PR |= 0x1000;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
	if(SW4_Flag==0){  //SW4_Flag�� 0�϶��� �������� ����, �ش� �ڵ����� 2���ۿ� �Է� �Ұ��ϵ��� �ڵ���
	  BEEP(); 
	  BitCheak(); // Operand1���� Operand2���� �����Ͽ� �����صδ� �Լ�
	  GPIOG->ODR &= ~0x00FF; // LED0~7 OFF
	  EXTI->IMR |= 0xF3C0; // ���ͷ�Ʈ EXTI 15~12, EXTI 6~9 enable 
	  SW4_Flag = 1; // SW4_Flag �ߺ�����
	  SW5_Flag++; // SW4 ���ͷ�Ʈ�� �� 2���ۿ� �ȿ´�. ���� SW5_Flag�� �������� 2���ȴ�.���� SW5_Flag�� 2�϶��� üũ.
	  Joy_Flag++; // Joy�� SW4�� 1�� �� ���Ŀ��� �� Error�̴�. ���� �ѹ� ���� 1 �������� 1�̿ܿ��� Error��Ȳ
	  StartFlag=1; // StartFlag���ٽ� 1�θ���� Opernad�Է¸��ϰ���
	  Next=5; // Opernad1,2�����ϱ����� ���� ����
	}  
	else if(SW4_Flag==1){ ErrorCheck(); } // SW4_Flag�� Handler���� �����Ͽ�, SW4(OperandȮ��)�� ������ ��� �����ϰ� ������������ Error�߻��ϵ�����

  }
}
/* Switch�� �ԷµǾ����� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */ 
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
  LCD_SetTextColor(RGB_BLACK); // ���ڻ� : ������
  if(Next==0){ // Operand1 ����
	if(GPIOG->IDR & 0x0001) {LCD_DisplayChar(4, 2 ,'1');  arr1[1]=1; }  //  LED0 ON�̸�, 1ǥ���ϵ����ϰ�, �迭�� 1�ֱ�
	else {LCD_DisplayChar(4, 2 ,'0');  arr1[1]=0;  }  // LED0 OFF�̸�, 0ǥ���ϵ����ϰ�, �迭�� 0 �ֱ�
	
	if(GPIOG->IDR & 0x0002) {LCD_DisplayChar(4, 3,'1');  arr1[2]=1; }  //  LED1 ON�̸�, 1ǥ���ϵ����ϰ�, �迭�� 1�ֱ�
	else {LCD_DisplayChar(4, 3 ,'0');  arr1[2]=0; } // LED1 OFF�̸�, 0ǥ���ϵ����ϰ�, �迭�� 0 �ֱ�
	
	if(GPIOG->IDR & 0x0004) {LCD_DisplayChar(4, 4 ,'1'); arr1[3]=1; }  //  LED2 ON�̸�, 1ǥ���ϵ����ϰ�, �迭�� 1�ֱ�
	else {LCD_DisplayChar(4, 4 ,'0'); arr1[3]=0; } // LED2 OFF�̸�, 0ǥ���ϵ����ϰ�, �迭�� 0 �ֱ�
	
	if(GPIOG->IDR & 0x0008) {LCD_DisplayChar(4, 5,'1'); arr1[4]=1; }  //  LED3 ON�̸�, 1ǥ���ϵ����ϰ�, �迭�� 1�ֱ�
	else {LCD_DisplayChar(4, 5 ,'0');  arr1[4]=0; } // LED3 OFF�̸�, 0ǥ���ϵ����ϰ�, �迭�� 0 �ֱ�
  }
  else if(Next==5){// Operand2 ����
	if(GPIOG->IDR & 0x0001) {LCD_DisplayChar(4, 2+Next ,'1'); arr2[1]=1; }  //  LED0 ON�̸�, 1ǥ���ϵ����ϰ�, �迭�� 1�ֱ�
	else {LCD_DisplayChar(4, 2+Next ,'0'); arr2[1]=0; } // LED0 OFF�̸�, 0ǥ���ϵ����ϰ�, �迭�� 0 �ֱ�
	
	if(GPIOG->IDR & 0x0002) {LCD_DisplayChar(4, 3+Next ,'1'); arr2[2]=1; } //  LED1 ON�̸�, 1ǥ���ϵ����ϰ�, �迭�� 1�ֱ�
	else {LCD_DisplayChar(4, 3+Next ,'0');  arr2[2]=0; } // LED1 OFF�̸�, 0ǥ���ϵ����ϰ�, �迭�� 0 �ֱ�
	
	if(GPIOG->IDR & 0x0004) {LCD_DisplayChar(4, 4+Next ,'1'); arr2[3]=1; } //  LED2 ON�̸�, 1ǥ���ϵ����ϰ�, �迭�� 1�ֱ�
	else {LCD_DisplayChar(4, 4+Next ,'0'); arr2[3]=0; } // LED2 OFF�̸�, 0ǥ���ϵ����ϰ�, �迭�� 0 �ֱ�
	
	if(GPIOG->IDR & 0x0008) {LCD_DisplayChar(4, 5+Next,'1'); arr2[4]=1; } //  LED3 ON�̸�, 1ǥ���ϵ����ϰ�, �迭�� 1�ֱ�
	else {LCD_DisplayChar(4, 5+Next ,'0');  arr2[4]=0; } // LED3 OFF�̸�, 0ǥ���ϵ����ϰ�, �迭�� 0 �ֱ�
  } 
}
void SetAdd(void)
{
  Carry = 0 ; // �ʱ⿡�� �� 2������ ���ؾ��ϱ⶧���� Carry�� ���� 0 
  for(int i=4; i>=0; i--){
	ResultArr[i]= arr1[i]+arr2[i]+Carry; // �� �������� ���ϰ� ĳ�� �߻��� ĳ������ ������
	if(ResultArr[i]==0) { ResultArr[i]=0; Carry=0;  } //    0 �ϰ�� �ش� 2���� 0,  carry0
	else if(ResultArr[i]==1) { ResultArr[i]=1; Carry=0;}  // 1�� ��� �ش� 2���� 1, Carry 0
	else if(ResultArr[i]==2) { ResultArr[i]=0, Carry=1;  } // 2�ϰ�� �ش�2���� 0, Carry 1
	else if(ResultArr[i]==3) { ResultArr[i]=1, Carry=1;  } // 3�ϰ�� �ش� 2���� 1, Carry 1
  }  // �� 2������ ���ϴ� ����
}
void GetResultText(int array[])  // ����� LCD�� ǥ���ϴ� �Լ� 
{n
  for(int i=1; i<5; i++){
	LCD_DisplayChar(4, 12+i ,0x30+array[i]);  // �޾ƿ� ������� ���ʴ�� LCD�� ǥ��
  }
}
void GetAdd(void)
{
  LCD_SetTextColor(RGB_BLACK); //���ڻ� : ������ 
  SetAdd();  //���ϱ� �Լ�
  
  if(ResultArr[0]==0){
	GetResultText(ResultArr);
  } // ĳ�� �߻������� �״�� 4bitǥ��
  else if(ResultArr[0]==1) {
	LCD_DisplayChar(4, 12 ,0x30+ResultArr[0]); // 5bit�̱⿡ �ֻ��� ��Ʈ ���
	GetResultText(ResultArr);
  } // �ֻ��� ��Ʈ�� ĳ���߻��� 1ǥ���� ������ 2����� ǥ�� 5bit
}
void Complement(void)
{
  OneComlement(arr2,1); // 1�� ����  
  Carry=1; // 1���ϱ⸦ ���� �ʱ� Carry 1�� ����
  for(int i = 4; i >= 0; i--){
	arr2[i]+=Carry; 
	if(arr2[i]==0) { arr2[i]=0,Carry=0;} // 0�̸�, �ش� 2���� 0, Carry 0
	else if(arr2[i]==1) {arr2[i]=1, Carry=0;} // 1�̸�, �ش� 2���� 1 , Carry 0
	else if(arr2[i]==2) {arr2[i]=0 ,Carry=1;} // 2���Ǹ�, �ش� 2���� 0 , Carry 1
	
  }  // 1���ϱ� ���� 
}
void OneComlement(int array[],int Set) 
{
  for(int i=4; i>=Set; i--)
  {
	if(array[i]==0){array[i]=1;}
	else if(array[i]==1){array[i]=0;}
  } // 1�� ���� , 0�̸� 1�� 1�̸� 0���� �ٲ��ִ� ����
}
void GetSub(void)
{
  LCD_SetTextColor(RGB_BLACK); // ���ڻ� : ������
  Complement(); //2�Ǻ���
  SetAdd(); // 2���� �޾ƿ°� ����
  if(ResultArr[0]==1) // �ֻ��� ĳ�� �߻��� ����̱⿡ �ֻ��� ĳ�� ������ 4�ڸ��� ���
  { 
	GetResultText(ResultArr); // ��� ��� �Լ� 
  }
  else if(ResultArr[0]==0) // �ֻ��� ���� 0�̸� ������ ����, ���� ������� 2�Ǻ��� 
  {
	// 2�Ǻ��� ����
	OneComlement(ResultArr,0); // 1�Ǻ���     
	Carry=1; //1�� ���ؾ��ϱ⿡  Carry���� 1����
	for(int j=4; j > 0; j--){
	  ResultArr[j]+=Carry; 
	  if( ResultArr[j]==0) { ResultArr[j]=0,Carry=0;} // 0�̸� 2���� 0, Carry0
	  else if(ResultArr[j]==1) {ResultArr[j]=1, Carry=0;} // 1�̸� 2���� 1, Carry 0
	  else if(ResultArr[j]==2) {ResultArr[j]=0 ,Carry=1;} //  2�̸� 2���� 0, Carry 1 
	} // 1�� ���� ���� +1 ���ִ°���,  �ֻ��� ĳ���߻��� ����
	LCD_DisplayChar(4, 12 ,'-'); // ����� �����̱⿡ - �ٿ���
	GetResultText(ResultArr);   // ����� ���
  }
}
void GetOr(void)
{
  for(int i =1; i<5 ; i++) {
	if(arr1[i] | arr2[i]) { ResultArr[i]=1; } 
	else { ResultArr[i]=0; }
  } // 2���� �� �ڸ����� Or ����, 1�̸� ��� 1, 0�̸� ��� 0
  LCD_SetTextColor(RGB_BLACK); // ���ڻ� : ������
  GetResultText(ResultArr); // ��� ��Ʈ ����Լ�
}
void GetAnd(void) // AND ���� �Լ�
{
  for(int i =1; i<5;i++){
	if(arr1[i] & arr2[i]) { ResultArr[i]=1; }
	else { ResultArr[i]=0; }
  } // 2������ �� �ڸ����� AND ����, 1�̸� ��� 1,  0�̸� ��� 0����  
  LCD_SetTextColor(RGB_BLACK); // ���ڻ� : ������ 
  GetResultText(ResultArr); // ��� ��Ʈ ����Լ� 
}
void ErrorCheck(void)
{
  LCD_SetBrushColor(RGB_RED); // �귯�� : RED
  LCD_DrawFillRect(141,13,8,8);   // Error �簢�� ȸ������ ä���
  DelayMS(2000);
  LCD_SetBrushColor(GET_RGB(195,195,195)); // �귯�� : GRAY 
  LCD_DrawFillRect(141,13,8,8);   // Error �簢�� ȸ������ ä���
  BEEP();
  DelayMS(500);
  BEEP();
}