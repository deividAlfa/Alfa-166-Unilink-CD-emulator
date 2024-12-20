/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "unilink.h"
#include "serial.h"
#include "files.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
typedef struct{
	int8_t	 	status;//0 = no drive, 1=mounted, 2=scanned
	uint32_t	lastFsSize;
	char		lastPath[16];
	char		lastFile[16];
	uint16_t	fileCnt[6];
}system_t;

extern system_t systemStatus;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void DebugPulse(uint8_t pulses);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define CLK_Pin GPIO_PIN_14
#define CLK_GPIO_Port GPIOC
#define CLK_EXTI_IRQn EXTI15_10_IRQn
#define DATA_Pin GPIO_PIN_15
#define DATA_GPIO_Port GPIOC
#define MT_FWD_Pin GPIO_PIN_0
#define MT_FWD_GPIO_Port GPIOA
#define MT_RVS_Pin GPIO_PIN_1
#define MT_RVS_GPIO_Port GPIOA
#define H_SPEED_Pin GPIO_PIN_2
#define H_SPEED_GPIO_Port GPIOA
#define L_minus_Pin GPIO_PIN_3
#define L_minus_GPIO_Port GPIOA
#define L_plus_Pin GPIO_PIN_4
#define L_plus_GPIO_Port GPIOA
#define PHOTO_R_Pin GPIO_PIN_5
#define PHOTO_R_GPIO_Port GPIOA
#define PHOTO_F_Pin GPIO_PIN_6
#define PHOTO_F_GPIO_Port GPIOA
#define POS_2_5_Pin GPIO_PIN_7
#define POS_2_5_GPIO_Port GPIOA
#define POS_1_2_Pin GPIO_PIN_0
#define POS_1_2_GPIO_Port GPIOB
#define POS_0_Pin GPIO_PIN_1
#define POS_0_GPIO_Port GPIOB
#define VBUS_Pin GPIO_PIN_2
#define VBUS_GPIO_Port GPIOB
#define TEST_Pin GPIO_PIN_6
#define TEST_GPIO_Port GPIOB
#define BREAK_Pin GPIO_PIN_7
#define BREAK_GPIO_Port GPIOB
#define PREV_Pin GPIO_PIN_8
#define PREV_GPIO_Port GPIOB
#define NEXT_Pin GPIO_PIN_9
#define NEXT_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
