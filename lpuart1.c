/*
 * lpuart1.c
 *
 *  Created on: May 3, 2026
 *      Author: alexm
 */
#include "lpuart1.h"

volatile uint8_t cursorRow = 12;
volatile uint8_t cursorCol = 40;
volatile uint8_t cursorUpdate = 0;

void INIT_Lpuart1( void ){

	PWR->CR2 |= (PWR_CR2_IOSV);              // power avail on PG[15:2] (LPUART1)
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOGEN);   // enable GPIOG clock
	RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN; // enable LPUART clock bridge
	/* USER: configure GPIOG registers MODER/PUPDR/OTYPER/OSPEEDR then
	   select AF mode and specify which function with AFR[0] and AFR[1] */
	// PG7 (TX) and PG8 (RX) initialization for LPUART functionality

	// alternate function mode 2b'10
	LPUART1_PORT->MODER &= ~(GPIO_MODER_MODE7 | GPIO_MODER_MODE8);
	LPUART1_PORT->MODER |= (GPIO_MODER_MODE7_1 | GPIO_MODER_MODE8_1);

	// tx no pull-up/pull-down, rx pull-up
	LPUART1_PORT->PUPDR &= ~(GPIO_PUPDR_PUPD7 | GPIO_PUPDR_PUPD8);
	LPUART1_PORT->PUPDR |= (GPIO_PUPDR_PUPD8_0);

	// push-pull
	LPUART1_PORT->OTYPER &= ~(GPIO_OTYPER_OT7 | GPIO_OTYPER_OT8);

	// low output speed configured - fine for 115.2 kbps baud
	LPUART1_PORT->OSPEEDR &= ~(0x3U << GPIO_OSPEEDR_OSPEED7_Pos |
										  0x3U << GPIO_OSPEEDR_OSPEED8_Pos);

	// configure alternate function 8 for PG7 and PG8 LPUART1_TX & LPUART1_RC
	LPUART1_PORT->AFR[0] &= ~(0xFU << 4*7);
	LPUART1_PORT->AFR[0] |= (0x8U << 4*7);
	LPUART1_PORT->AFR[1] &= ~(0xFU);
	LPUART1_PORT->AFR[1] |= (0x8U);

	//set 115.2kbps baud rate with 4MHz clk

   LPUART1->BRR = (256UL * 4000000UL) / 115200UL;

	LPUART1->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0); // 8-bit data
	LPUART1->CR1 |= USART_CR1_UE;                   // enable LPUART1
	LPUART1->CR1 |= (USART_CR1_TE | USART_CR1_RE);  // enable xmit & recv
	LPUART1->CR1 |= USART_CR1_RXNEIE;        // enable LPUART1 recv interrupt
	LPUART1->ISR &= ~(USART_ISR_RXNE);       // clear Recv-Not-Empty flag


	NVIC->ISER[2] = (1 << (LPUART1_IRQn & 0x1F));   // enable LPUART1 ISR
	__enable_irq();                          // enable global interrupts

}


void LPUART_Print( const char* message ) {
   uint16_t iStrIdx = 0;
   while ( message[iStrIdx] != 0 ) {
      while(!(LPUART1->ISR & USART_ISR_TXE)) // wait for empty xmit buffer
         ;
      LPUART1->TDR = message[iStrIdx];       // send this character
	iStrIdx++;                             // advance index to next char
   }
}

void LPUART_ESC_Print( const char* message ){

	while( !(LPUART1->ISR & USART_ISR_TXE) );
	LPUART1->TDR = 0x1B;
	LPUART_Print(message);
}


void LPUART1_IRQHandler( void ) {

	// hold previous state **must have static declaration
	static uint8_t escState = 0;
	// current char received on rx
   uint8_t charRecv;

   if ( LPUART1->ISR & USART_ISR_RXNE ) {
      charRecv = LPUART1->RDR;

      // use state machine to check if any arrow key pressed
      switch ( escState ){
			case 0:
				escState = (charRecv == 0x1B) ? 1:0;
				break;
			case 1:
				escState = (charRecv == '[') ? 2:0;
				break;
			case 2:
				switch (charRecv){

					case 'A': cursorRow = (cursorRow == 1) ? 24 : cursorRow - 1; //up
					break;
					case 'B': cursorRow = (cursorRow == 24) ? 1 : cursorRow + 1;  //down
					break;
					case 'C': cursorCol = (cursorCol == 80) ? 1 : cursorCol + 1;  //right
					break;
					case 'D': cursorCol = (cursorCol == 1) ? 80 : cursorCol - 1;  //left
					break;
				}
				cursorUpdate = 1; // set flag
				escState = 0;
				break;
      }

    }

}

/*
void LPUART1_IRQHandler( void ) {
   uint8_t charRecv;
   if ( LPUART1->ISR & USART_ISR_RXNE ) {
      charRecv = LPUART1->RDR;
      switch ( charRecv ) {

			case 'R': LPUART_ESC_Print("[31m");  break;
			case 'G': LPUART_ESC_Print("[32m");  break;
			case 'B': LPUART_ESC_Print("[34m");  break;
			case 'W': LPUART_ESC_Print("[37m");   break;

	   default:
	      while( !(LPUART1->ISR & USART_ISR_TXE) ); // wait for empty TX buffer
	      LPUART1->TDR = charRecv;  // echo char to terminal
	    }
   }
} */

void Instruction_4( void ){
	LPUART_ESC_Print("[2J");
	LPUART_ESC_Print("[3B");
	LPUART_ESC_Print("[5C");
	LPUART_Print("All good students read the");
	LPUART_ESC_Print("[1B");
	LPUART_ESC_Print("[21D");
	LPUART_ESC_Print("[5m");
	LPUART_Print("Reference Manual");
	LPUART_ESC_Print("[H");
	LPUART_ESC_Print("[0m");
	LPUART_Print("Input: ");
}

void SetCharCenter( void ){
	cursorRow = 12;
	cursorCol = 40;
	LPUART_Print("\x1B[12;40H");  // move cursor to center
	LPUART_Print("o");            // print your character
}

void LPUART_PrintCharAt(uint8_t row, uint8_t col, char c) {
    char buf[16];
    sprintf(buf, "\x1B[%d;%dH%c", row, col, c);
    LPUART_Print(buf);
}
