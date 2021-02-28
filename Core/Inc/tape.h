/*
 * tape.h
 *
 *  Created on: Dec 22, 2020
 *      Author: David
 */

#ifndef INC_TAPE_H_
#define INC_TAPE_H_
#include "main.h"


enum{															// "POS. SEN" pin
  pos_5V,														// No tape. (Unused)
  pos_2_5V,														// Tape in, also RWD and FF position. 6.8K resistor to GND, 2.5V
  pos_1_2V,														// Transition from/to play position. 2.2K resistor to GND, 1.2V
  pos_0V,														// Play position. Signal direct to GND, 0V

  status_noPlay,												// Not in play mode. (Unused)
  status_fwdPlay,												// Playing in forward mode
  status_revPlay,												// Playing in reverse mode
};
  	  	  	  	  	  	  	  									// Delay in mS
#define posPreDelay						(uint32_t)	40			// Delay before changing position after motor is turned on
#define posPostDelay					(uint32_t)	40			// Delay before changing position if motor doesn't turn off
#define pulseDelay						(uint32_t)	10			// High/Low times for photosensor signal generation
#define longPhotoDelay					(uint32_t)	1200		// Photosensor long generation time (to prevent fault detection if too many fast skips)
#define buttonDelay						(uint32_t)	50			// Button pulse time
#define resetTimeOnPlay					(uint32_t)	2000		// Time in play mode to reset the fast skip counter

#define MIN_POS							pos_2_5V				// Min position
#define MAX_POS							pos_0V					// Max position
#define MAX_FAST_SKIP					(uint8_t)	4			// Max fast skips before adding delay, to prevent fault detection

#define H_Speed 						HAL_GPIO_ReadPin(H_SPEED_GPIO_Port, H_SPEED_Pin)
#define MT_Fwd 							HAL_GPIO_ReadPin(MT_FWD_GPIO_Port, MT_FWD_Pin)
#define MT_Rev 							HAL_GPIO_ReadPin(MT_RVS_GPIO_Port, MT_RVS_Pin)
#define L_Plus 							HAL_GPIO_ReadPin(L_plus_GPIO_Port, L_plus_Pin)
#define L_Minus 						HAL_GPIO_ReadPin(L_minus_GPIO_Port, L_minus_Pin)

#define TogglePhotoR()					HAL_GPIO_TogglePin(PHOTO_R_GPIO_Port, PHOTO_R_Pin)
#define TogglePhotoF()					HAL_GPIO_TogglePin(PHOTO_F_GPIO_Port, PHOTO_F_Pin)

#define setButtonNextHigh()				HAL_GPIO_WritePin(NEXT_GPIO_Port,NEXT_Pin,SET);
#define setButtonNextLow()				HAL_GPIO_WritePin(NEXT_GPIO_Port,NEXT_Pin,RESET);
#define setButtonPrevHigh()				HAL_GPIO_WritePin(PREV_GPIO_Port,PREV_Pin,SET)
#define setButtonPrevLow()				HAL_GPIO_WritePin(PREV_GPIO_Port,PREV_Pin,RESET)

#define setPos_2_5vHigh()				HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,SET)
#define setPos_2_5vLow()				HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,RESET)
#define setPos_1_2vHigh()				HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,SET)
#define setPos_1_2vLow()				HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,RESET)
#define setPos_0vHigh()					HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,SET)
#define setPos_0vLow()					HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,RESET)

#define setPos_5V()						setPos_2_5vHigh();	setPos_1_2vHigh();	setPos_0vHigh()
#define setPos_2_5V()					setPos_2_5vLow(); 	setPos_1_2vHigh();	setPos_0vHigh()
#define setPos_1_2V()					setPos_2_5vHigh();	setPos_1_2vLow();	setPos_0vHigh()
#define setPos_0V()						setPos_2_5vHigh();	setPos_1_2vHigh();	setPos_0vLow()

void initTape(void);
void handleTape(void);

#endif /* INC_TAPE_H_ */
