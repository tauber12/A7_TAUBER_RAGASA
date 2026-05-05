/*
 * lpuart1.h
 *
 *  Created on: May 3, 2026
 *      Author: alexm
 */

#ifndef INC_LPUART1_H_
#define INC_LPUART1_H_

#include "stm32l4xx_hal.h"
#include <stdio.h>

extern volatile uint8_t cursorRow;
extern volatile uint8_t cursorCol;
extern volatile uint8_t cursorUpdate;

#define LPUART1_PORT GPIOG

void INIT_Lpuart1( void );
void LPUART_Print( const char* message );
void LPUART_ESC_Print( const char* message );
void LPUART1_IRQHandler( void );
void Instruction_4( void );
void LPUART_PrintCharAt(uint8_t row, uint8_t col, char c);

#endif /* INC_LPUART1_H_ */
