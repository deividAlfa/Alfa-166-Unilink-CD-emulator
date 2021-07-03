/*
 * serial.c
 *
 *  Created on: Dec 23, 2020
 *      Author: David
 */
#include "serial.h"
#define SerFifoLen  8

struct{
  volatile uint16_t     LostinPutString;      // Data lost counter while trying to send through putstring (fifo full)
  volatile uint16_t     LostinCallback;       // Data lost counter while trying to send inside the callback (really strange if it happens)
  volatile uint16_t     errPutString;         // Data lost counter while trying to send through putstring (Uart error)
  volatile uint16_t     errCallback;          // Data lost counter while trying to send inside the callback (Uart error)
  volatile char*        Queue[SerFifoLen];    // Queue of pointers
  volatile uint8_t      QueueLen[SerFifoLen]; // Queue of data lengths
  volatile uint8_t      TxPos;                // Output Fifo position
  volatile uint8_t      BfPos;                // Input Fifo position
  volatile uint8_t      Pending;              // Pending elements to be sent in fifo
  UART_HandleTypeDef*   uart;                 // Pointer to uart handler
}serial;

void initSerial(UART_HandleTypeDef* huart){
  serial.uart=huart;
#ifdef DebugLevel
  #if (DebugLevel==1)
  serial.uart->Init.BaudRate = 9600;          // Lowest speed for better reading in logic analyzer
  #elif (DebugLevel==2)
  serial.uart->Init.BaudRate = 38400;         // Medium speed to not overflow the UART
  #elif (DebugLevel==3)
  serial.uart->Init.BaudRate = 57600;         // High speed to not overflow the UART
  #endif
  HAL_UART_Init(serial.uart);
#endif
}
void putString(char *str){
  uint16_t len=0;
  uint8_t tmp;
  if(*str){                                                           // If empty string, return
    //printf(str);                                                    // "Hack", printf is redirected to SWO through ITM_Send
    while(str[len]){
      len++;                                                          // Get string length
    }
    __disable_irq();                                                  // Disable interrupts to prevent errors, double-calling, etc
    tmp=HAL_UART_Transmit_DMA(serial.uart, (uint8_t *)str, len);      // Try to send data
    __enable_irq();
    if(tmp==HAL_OK){
      return;                                                         // If OK, return (Fifo empty)
    }
    if(tmp==HAL_BUSY){                                                // If busy, try to store in queue
      if(serial.Pending>(SerFifoLen-1)){                              // If queue full, discard data
        serial.LostinPutString++;                                     // Increase lost counter (For debugging)
        return;                                                       // return
      }
      serial.Queue[serial.BfPos]=str;                                 // Store string pointer
      serial.QueueLen[serial.BfPos]=len;                              // Store string length
      if(++serial.BfPos>(SerFifoLen-1)){                              // Increase Input FIFO position (Circular mode)
        serial.BfPos=0;
      }
      __disable_irq();                                                // Disable irq to prevent Pending variable corruption
      serial.Pending++;                                               // Increase pending transfers
      __enable_irq();
    }
    else if(tmp==HAL_ERROR){                                          // If error
      serial.errPutString++;                                          // Increase error counter ( For debugging)
    }
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
  uint8_t tmp;
  if(huart==serial.uart){                                             // If it's our UART
    if(serial.Pending){                                               // If data pending

      tmp=HAL_UART_Transmit_DMA(  serial.uart,                        // Try to transmit data
                                  (uint8_t *)serial.Queue[serial.TxPos],
                                  serial.QueueLen[serial.TxPos]);
      if(tmp==HAL_OK){                                                // If OK
        serial.Pending--;                                             // Decrease pending
        if(++serial.TxPos>7){                                         // Increase Output FIFO position (Circular mode)
          serial.TxPos=0;                                             
        }
      }
      else if(tmp==HAL_BUSY){                                         // If busy
        serial.LostinCallback++;                                      // Increase lost data counter (For debugging)
      }
      else if(tmp==HAL_ERROR){                                        // If error
        serial.errCallback++;                                         // Increase error counter (For debugging)
      }
    }
  }
}
