// VendingMachine.c
// Runs on LM4F120 or TM4C123
// Index implementation of a Moore finite state machine to operate
// a Pi-directional Room.
// Daniel Valvano, Jonathan Valvano
// Feb 7, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013
   Volume 1 Program 6.8, Example 6.4
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
   Volume 2 Program 3.1, Example 3.1

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// PB1 LED triggering 
// PB0 Motor triggering
// PE1 means person is about to enter a room
// PE0 means person is about to leave the room 

#include "PLL.h"
#include "SysTick.h"
#include "LCD.h"

#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))

#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define IR_INPUT   (*((volatile unsigned long *)0x4002400C))
#define SODA    (*((volatile unsigned long *)0x40005004))
#define LED  (*((volatile unsigned long *)0x40005008))

#define T10ms 800000
#define T20ms 1600000

unsigned char Persons_Count;
void FSM_Init(void){ volatile unsigned long delay;
  PLL_Init();       // 80 MHz, Program 10.1
  SysTick_Init();   // Program 10.2
  SYSCTL_RCGC2_R |= 0x12;      // 1) B E
  delay = SYSCTL_RCGC2_R;      // 2) no need to unlock
  GPIO_PORTE_AMSEL_R &= ~0x03; // 3) disable analog function on PE1-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO
  GPIO_PORTE_DIR_R &= ~0x03;   // 5) inputs on PE1-0
  GPIO_PORTE_AFSEL_R &= ~0x03; // 6) regular function on PE1-0
  GPIO_PORTE_DEN_R |= 0x03;    // 7) enable digital on PE1-0
  GPIO_PORTB_AMSEL_R &= ~0x03; // 3) disable analog function on PB1-0
  GPIO_PORTB_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO
  GPIO_PORTB_DIR_R |= 0x03;    // 5) outputs on PB1-0
  GPIO_PORTB_AFSEL_R &= ~0x03; // 6) regular function on PB1-0
  GPIO_PORTB_DEN_R |= 0x03;    // 7) enable digital on PB1-0
  SODA = 0; LED = 0; Persons_Count = 0;
}

unsigned long IR_Input(void){
  return IR_INPUT;  // PE1,0 can be 0, 1, or 2
}
void Pidirectional_None(void){
	LCD_displayStringRowColumn(0,0,"Welcome visitors");
};

void Pidirectional_IncCount(void){
	Persons_Count++;
	LCD_goToRowColumn(1,8);
	LCD_intgerToString(Persons_Count);
	
	if(Persons_Count >= 1)
  LED = 0x02;        // activate LED 
  
}

void Pidirectional_DecCount(void){
	Persons_Count--;
	LCD_goToRowColumn(1,8);
	LCD_intgerToString(Persons_Count);
	
	if(Persons_Count < 1)
	{
   LED = 0x00;        // deactivate LED 
   Persons_Count = 0;
	}
}

struct State {
  void (*CmdPt)(void);   // output function
  unsigned long Time;    // wait time, 12.5ns units
  unsigned long Next[4];}; 
typedef const struct State StateType;
#define IDLE        0
#define waitToEnter 1
#define waitToLeave 2
#define Enter       3
#define Leave	      4
StateType FSM[5]={
  {&Pidirectional_None,  T20ms,{IDLE ,waitToEnter,waitToLeave,IDLE}},      // IDLE, no one enters
  {&Pidirectional_None,  T20ms,{IDLE ,waitToEnter,Enter,IDLE}},       // waitToEnter, in case entering person is waiting between 2 IRs
  {&Pidirectional_None,  T20ms,{IDLE ,Leave,waitToLeave,IDLE}},     // waitToEnter, in case leaving person is waiting betweeen 2 IRs
  {&Pidirectional_IncCount,  T20ms,{IDLE ,IDLE,IDLE,IDLE}},    // Enter, counter is incremented and room LED is on
  {&Pidirectional_DecCount,  T20ms,{IDLE ,IDLE,IDLE,IDLE}},    // Leave, counter is decremented and room LED is off
};
unsigned long S; // index into current state     
unsigned long Input; 
int main(void){ 
  char count;	
	FSM_Init();
	PortA_Init();
	PortD_Init();
	LCD_init();
  S = IDLE;       // Initial State 
  while(1){
    (FSM[S].CmdPt)();           // call output function
    SysTick_Wait(FSM[S].Time);  // wait Program 10.2
    Input = IR_Input();       // input can be 0,1,2
    S = FSM[S].Next[Input];     // next
  }
}



