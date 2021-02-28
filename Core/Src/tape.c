/*
 * tape.c
 *
 *  Created on: Dec 22, 2020
 *      Author: David
 */
#include "tape.h"

volatile struct{
  uint32_t 	disPhotoTimer;		// Timer for photosensor disable delay
  uint32_t 	pulseTimer;			// Timer for photosensor signal generation
  uint32_t 	changedTimer;		// Timer for skip tracks.
  uint8_t	SkipCnt;			// Counter for consecutive skips. More than 4 fast skips will make the tape think it's stuck. Wait longer this time.
  uint32_t 	startPlayTimer;		// Time when the play started
  uint8_t 	last_PlayMode;		// Last known play mode
  bool 		prevFast;			// Flag to indicate a fast mode was set before
  bool 		prevPlay;			// Flag to indicate play mode was set before
  bool		EnablePhoto;		// Flag to enable or disable the photosensor signal (For forcing RWD/FF modes to stop and resume and play mode)
}tape;

volatile struct{
  uint8_t 	OutStatus;			// Actual analog position
  uint32_t 	Timer;				// Timer for position transition delays
  uint8_t 	Status;				// For position state machine
  int8_t	Direction;			// Switching direction
}pos;

volatile struct{
  uint32_t 	Timer;				// Timer for the button push time
  bool 		pulsed;
}button;


void initTape(void){
	tape.prevFast=0;			// No previous Fast mode
	tape.prevPlay=0;			// No previous Play mode
	pos.OutStatus=pos_2_5V;		// Tape in
	pos.Status=0;				// First machine state
	button.pulsed=0;			// No button pulsed
	pos.Direction=1;			// Forward position direction
}

void handleTape(void){
 /*********************************************************************************************************************
	 *		Tape position sensor. L+ and L- signals set the motor polarity.
	 *
	 *		We don't care for the direction.
	 *		L+==0 && L-==0 ==> Tape not active (Radio or CD mode)(Both inactive)
	 *		L+==1  ^ L-==1 ==> Changing position (Only one active)
	 *		L+==1 && L-==1 ==> Active in a stable position (Both active)
	 *
	 *		As long as the motor spins, we keep switching between the analog voltages with a small delay.
	 *		2.5V <> 1.2V <> 0V
	 *
	 *	When the tape controller finds the correct position, it stops the motor, so we do.
	 *********************************************************************************************************************/
	  if(L_Plus ^ L_Minus ){														// Only L+ or L-. Both 0 or 1 = do nothing
		  switch(pos.Status){
				case 0:
					pos.Timer=HAL_GetTick();
					pos.Status=1;
					break;
				case 1:
					if((HAL_GetTick()-pos.Timer)>posPreDelay){						// Small delay before changing output
						pos.Status=2;
					}
					break;
				case 2:
					pos.OutStatus += pos.Direction;
					if(pos.OutStatus==MIN_POS){										// Change positions
						pos.Direction=1;

					}
					if(pos.OutStatus==MAX_POS){
						pos.Direction=-1;
					}
					pos.Status=3;
					pos.Timer=HAL_GetTick();
					break;
				case 3:
					if((HAL_GetTick()-pos.Timer)>posPostDelay){						// Small delay before next change
						pos.Status=0;												// First machine state
					}
					break;
		  }
	  }
	  if(!L_Plus && !L_Minus ){														// Not in tape mode
		 // pos.OutStatus = pos_2_5V;													// Reset everything
		  tape.prevFast=0;
		  tape.prevPlay=0;
		  tape.EnablePhoto=1;
	  }
	  /*********************************************************************************************************************
	   * 	Check Forward/Reverse inputs
	   *********************************************************************************************************************/
	  if(L_Plus && L_Minus){														// Active in some position
		  if(MT_Rev || MT_Fwd){
			  if(((HAL_GetTick()-tape.pulseTimer)>pulseDelay)&&tape.EnablePhoto){	// Simulate the photosensor pulses if active flag
				  tape.pulseTimer=HAL_GetTick();
				  TogglePhotoR();													// Toggle both sensors
				  TogglePhotoF();
			  }
			  if(!H_Speed){															// Not in in fast speed (revPlay or fwdPlay mode)
				  if(!tape.prevPlay){												// Wasn't on play mode before?
					  tape.prevPlay=1;												// Set flag
					  tape.prevFast=0;												// Reset flag
					  tape.EnablePhoto=1;											// Enable photosensor
					  tape.startPlayTimer=HAL_GetTick();							// Time when started playing after a fast mode

				  }
				  if(MT_Rev){
					  tape.last_PlayMode=status_revPlay;							// In reverse play mode
				  }
				  if(MT_Fwd){
					  tape.last_PlayMode=status_fwdPlay;							// In forward play mode
				  }
			  }
			  else{																	// In high speed mode (RWD or FF)
				  if(MT_Fwd^MT_Rev){												// Any direction?
					  if(!tape.prevFast){											// Wasn't in Fast mode before?
						  tape.SkipCnt++;											// Increase skipped count
						  tape.disPhotoTimer=HAL_GetTick();							// Update photosensor disable timer
						  button.Timer=HAL_GetTick();						  		// Button timer
						  button.pulsed=1;											// Button pulsed flag
						  tape.prevFast=1;											// Set flag
						  tape.prevPlay=0;											// Reset flag
						  if(MT_Fwd){												// In FF mode
							  if(tape.last_PlayMode==status_revPlay){
								  setButtonPrevHigh();								// Send pulse to BT Prev track
							  }
							  if(tape.last_PlayMode==status_fwdPlay){
								  setButtonNextHigh();								// Send pulse to BT Next track
							  }
						  }
						  if(MT_Rev){												// In RWD mode
							  if(tape.last_PlayMode==status_revPlay){
								  setButtonNextHigh();								// Send pulse to BT Next track
							  }
							  if(tape.last_PlayMode==status_fwdPlay){
								  setButtonPrevHigh();								// Send pulse to BT Prev track
							  }
						  }
					  }
				  }
			  }
		  }
	  }
	  if(tape.prevFast){
		  if(tape.SkipCnt>=MAX_FAST_SKIP){
			  if((HAL_GetTick()-tape.disPhotoTimer)>longPhotoDelay){				// After 3 quick skips, add delay to avoid tape error triggering
				  tape.SkipCnt=0;													// Reset count
			  }
		  }
		  else{																		// No delay for the first 3 skips or
			  tape.EnablePhoto=0;  	  	  	  	  	  	  	  	  	  	  	 		// Disable photosensor pulses
		  }
	  }																				// Then disable pulses to simulate end of tape (back to play mode)
	  if(tape.prevPlay && (HAL_GetTick()-tape.startPlayTimer)>resetTimeOnPlay){
		  tape.SkipCnt=0;															// Reset if playing for >2 seconds
	  }

	  if(button.pulsed && (HAL_GetTick()-button.Timer)>buttonDelay){				// Button timer
		  button.pulsed=0;
		  setButtonPrevLow();														// Reset BT buttons
		  setButtonNextLow();
	  }
	  /*********************************************************************************************************************
	   * 	Set the outputs for the analog sensor based on the position status
	   * 	SET = open drain, pin Hi-Z.
	   * 	RESET = Set to Gnd
	   *********************************************************************************************************************/
	  switch(pos.OutStatus){														// Set the analog position
		  case pos_5V:
			  setPos_5V();															// Position = 5V
			  break;
		  case pos_2_5V:
			  setPos_2_5V();														// Position = 2.5V
			  break;
		  case pos_1_2V:
			  setPos_1_2V();														// Position = 1.2V
			  break;
		  case pos_0V:
			  setPos_0V();															// Position = 0V
			  break;
	  }
}
