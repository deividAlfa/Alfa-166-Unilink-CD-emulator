/*
 * serial.c
 *
 *  Created on: Dec 23, 2020
 *      Author: David
 */
#include "serial.h"
#define SerFifoLen	8

struct{
	volatile uint16_t 	LostinPutString;
	volatile uint16_t 	LostinCallback;
	volatile uint16_t 	errPutString;
	volatile uint16_t 	errCallback;
	volatile char* 		Queue[SerFifoLen];
	volatile uint8_t 	QueueLen[SerFifoLen];
	volatile uint8_t 	TxPos;
	volatile uint8_t 	BfPos;
	volatile uint8_t 	Pending;
	UART_HandleTypeDef* uart;
}serial;

void initSerial(UART_HandleTypeDef* huart){
	serial.uart=huart;
#ifdef DebugLevel
	#if (DebugLevel==1)
	serial.uart->Init.BaudRate = 9600;		// Lowest speed for better reading in logic analyzer
	#elif (DebugLevel==2)
	serial.uart->Init.BaudRate = 38400;		// Medium speed to not overflow the UART
	#elif (DebugLevel==3)
	serial.uart->Init.BaudRate = 57600;		// High speed to not overflow the UART
	#endif
	HAL_UART_Init(serial.uart);
#endif
}
void putString(char *str){
	uint16_t len=0;
	uint8_t tmp;
	if(*str){																		// If empty string, return
		printf(str);
		//HAL_UART_DMAPause(serial.uart);												// Pause DMA,  the interrupt could write at the same time and cause data loss
		//__HAL_DMA_DISABLE_IT(serial.uart->hdmatx,DMA_IT_TC);							// Disable DMA interrupt
		while(str[len]){
			len++;																	// Get string length
		}
		tmp=HAL_UART_Transmit_DMA(serial.uart, (uint8_t *)str, len);					// Try to send data
		if(tmp==HAL_OK){
			return;																	// If OK, return
		}
		if(tmp==HAL_BUSY){															// If busy, try to store in queue
			if(serial.Pending>(SerFifoLen-1)){										// If queue full, discard data
				serial.LostinPutString++;												// Increase lost counter (debugging)
				return;																// return
			}
			serial.Queue[serial.BfPos]=str;											// Store string pointer
			serial.QueueLen[serial.BfPos]=len;										// Store string length
			if(++serial.BfPos>(SerFifoLen-1)){										// Increase Buffer position(circular mode)
				serial.BfPos=0;														// If reached end, go to start
			}
			serial.Pending++;														// Increase pending transfers
		}
		else if(tmp==HAL_ERROR){													// If error
			serial.errPutString++;													// Increase error counter (debugging)
		}
		//__HAL_DMA_ENABLE_IT(serial.uart->hdmatx,DMA_IT_TC);								// Enable DMA interrupt
		//HAL_UART_DMAResume(serial.uart);												// Resume DMA
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	uint8_t tmp;
	if(huart==serial.uart){																// If it's our UART
		if(serial.Pending){															// If data pending

			tmp=HAL_UART_Transmit_DMA(	serial.uart,									// Transmit data
										(uint8_t *)serial.Queue[serial.TxPos],
										serial.QueueLen[serial.TxPos]);
			if(tmp==HAL_OK){														// If OK
				serial.Pending--;													// Decrease pending
				if(++serial.TxPos>7){
					serial.TxPos=0;													// Increase TxPos
				}
			}
			else if(tmp==HAL_BUSY){													// If busy
				serial.LostinCallback++;												// Increase lost data counter
			}
			else if(tmp==HAL_ERROR){												// If error
				serial.errCallback++;												// Increase error counter
			}
		}
	}
}

