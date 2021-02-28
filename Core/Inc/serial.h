/*
 * serial.h
 *
 *  Created on: Dec 23, 2020
 *      Author: David
 */

#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_
#include "main.h"

void initSerial(UART_HandleTypeDef* huart);
void putString(char *str);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
#endif /* INC_SERIAL_H_ */
