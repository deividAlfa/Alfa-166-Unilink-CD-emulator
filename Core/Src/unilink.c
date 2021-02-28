/*
 * unilink.c
 *
 *  Created on: Dec 22, 2020
 *      Author: David
 *
 *  Credits:
 *
 *  Michael Wolf
 *  https://www.mictronics.de/ (Original AVR Becker Unilink page down)
 *
 *  Sander Huiksen
 *  https://sourceforge.net/projects/alfa166unilink-git/
 *
 */

#include "unilink.h"

/****************************************************************
 * 						VARIABLES								*
 ****************************************************************/
unilink_t 		unilink;
slaveBreak_t 	slaveBreak;
spi_t 			spi;
magazine_t 		mag_data;
cdinfo_t 		cd_data[6];
uint8_t 		rawCmd[16];

/****************************************************************
 * 						STRINGS									*
 ****************************************************************/
#if (DebugLevel>=2)
char brk_str1[] = "(Bpos:0,Cnt:0,Spos:0)";
char brk_str2[] = "(Bpos:0,Cnt:0,Spos:0)";
#endif

#if (DebugLevel>=1)
char timeStr[] 				= "\t\t\t\t\t\t\t\tTIME: Disc:00, Track:00, 00m, 00s\n";
char changeStr[] 			= "\t\t\t\t\t\t\t\tChange to: Disc: 00, Track: 00\n";
char* labels[] = {
	[label_seek] 			=	"SEEK\n",
	//[label_time] 			=	"TIME\n",
	[label_magazine]		=	"MAGAZINE\n",
	[label_discinfo] 		=	"DISC_INFO\n",
	[label_status] 			=	"STATUS\n",
	[label_mode] 			=	"MODE\n",
	[label_raw] 			=	"RAW\n",
	[label_playing] 		=	"PLAYING\n",
	[label_changed] 		=	"CHANGED\n",
	[label_seeking] 		=	"SEEKING\n",
	[label_changing] 		=	"CHANGING\n",
	[label_idle] 			=	"IDLE\n",
	[label_ejecting] 		=	"EJECTING\n",
	[label_unknownStatus]	=	"UNKNOWN STATUS\n",
	[label_unknownCmd]		=	"UNKNOWN CMD\n",
	[label_busReset]		=	"\t\t\t\tBUS RESET\n",
	[label_anyone]			=	"\t\t\t\tANYONE?:",
	[label_anyoneResp]		=	"Device MSG\n",
	[label_cmd1]			=	"CMD?\n",
	[label_request1]		=	"REQUEST?\n",
	[label_power]			=	"\t\t\t\tPower status=",
	[label_setactive]		=	"Set active\n",
	[label_setidle]			=	"Set idle\n",
	[label_srcselect]		=	"\t\t\t\tSrc Select\n",
	[label_switch]			=	"\t\t\t\tSwitch to?\n",
	[label_play]			=	"\t\t\t\tPlay\n",
	[label_cmd]				=	"CMD:",
	[label_alreadysentId]	=	"Already sent ID\n",
	[label_poll]			=	"Poll:",
	[label_slavePoll]		=	"SlavePoll:",
	[label_ffwd]			=	"\t\t\t\tFast Forward\n",
	[label_frwd]			=	"\t\t\t\tFast Rewind\n",
	[label_repeat]			=	"\t\t\t\tRepeat:",
	[label_shuffle]			=	"\t\t\t\tShuffle:",
	[label_off]				=	"Off\n",
	[label_on]				=	"On\n",
	//[label_change]		=	"Change to:",
	[label_appoint]			=	"Appoint ok:",
	[label_dspdiscchange]	=	"DSP changed CD\n",
	[label_intro]			=	"End intro\n",
	[label_maginfo]			=	"Magazine info\n",
	[label_mag_slotempty]	=	"Magazine slot empty\n",
	[label_mag_removed]		=	"Magazine removed\n",
	[label_mag_inserted]	=	"Magazine inserted\n",
};
#endif


/****************************************************************
 *					initUnilink 								*
 * 																*
 * 	Init function, to be called before main() loop				*
 * 																*
 ****************************************************************/
void unilinkColdReset(void){
	unilink.ownAddr 	= Adr_Reset;									// Load default address
	unilink.groupID 	= Adr_Reset;									// Load default address
	unilinkWarmReset();
}

void unilinkWarmReset(void){
	disableSpi();
	unilink.appoint 	= 0;											// We don't want appoint  yet (only after "Anyone" command)
	unilink.rxCount		= 0;											// Reset rx counter
	unilink.txCount		= 0;											// Reset tx counter
	unilink.disc=0;
	for(uint8_t i=0;i<6;i++){											// Update magazine data
		if(cd_data[i].mins){
			if(unilink.disc==0){										// Find first cd with files
				unilink.disc=i+1;										// Set current disc ands track
				unilink.lastDisc=i+1;									// reset last disc and track
				unilink.track=1;
				unilink.lastTrack=1;
				unilink.min=0;											// reset playing time
				unilink.sec=0;
				unilink.cnt=0;
				#if(DebugLevel>=1)
				printf("Start Disc:%d Track:%d\n",unilink.disc,unilink.track);
				#endif
				break;
			}
		}
	}
	/*
	unilink.disc		= 1;											// Default disc
	unilink.track		= 1;											// Default track
	unilink.lastDisc	= 1;											// Default disc
	unilink.lastTrack	= 1;											// Default track
	unilink.min			= 0;											// Clear time
	unilink.sec			= 0;											//
	unilink.cnt			= 0;											//
	*/
	unilink.received	= 0;											// Clear received flag
	unilink.status		= unilink_idle;									// Idle status
	spi.lastTxBit		= 0;											// Reset lastTx flag
	spi.TxMode			= 0;											// Set Rx mode
	slaveBreak.BfPos	= 0;											// Clear all slavebreak stuff
	slaveBreak.SendPos 	= 0;											//
	slaveBreak.pending	= 0;											//
	slaveBreak.status	= break_idle;									//
	setDataHigh();														// Release Data line
	setDataInput();														// Set Data input
	unilink.masterinit	= 0;
	resetSpiBit();														// Reset spi bit shift counter and enable
}
char hex2ascii(uint8_t hex){
	hex&=0x0f;
	if(hex<10){
		return('0'+hex);
	}
	else{
		return('7'+hex);
	}
	return ' ';
}
#if (DebugLog>=1)
volatile bool logerr=0;
void unilinkLog(void){
	uint8_t size,count=0,i=0;
	volatile uint8_t* data;
	static char str[64];
	if(unilink.logTx){
		if(LogFormat){str[i++]='>';}
		unilink.logTx=0;
		size=unilink.txSize;
		data=&unilink.txData[0];
	}
	else if(unilink.logRx){
		if(LogFormat){str[i++]='#';}
		unilink.logRx=0;
		size=unilink.rxSize;
		data=&unilink.rxData[0];
	}
	else{return;}
//  Examples of unilink packets
//	char data[]= "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";
	        //     0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
			//    ra ta m1 m2 c1 d1 d2 d3 d4 d5 d6 d7 d8 d9 c2
			//    [          ][c][                         ][c]			// 13 byte cmd
			//    [          ][c][          ][c]						// 8 byte cmd
			//    [          ][c]										// 4 byte cmd
	while(count<size){
		if(count==0 && LogFormat){
			str[i++]='[';
		}
		str[i++]=hex2ascii((data[count])>>4);
		str[i++]=hex2ascii((data[count])&0x0f);
		count++;
		if(		(count<4)||
				(size>6 &&count>5 && count<10)||
				(size>11 && count>8 && count<15)/*||(count==size-2)*/ ){
				str[i++]=' ';
		}
		if(		(count==4||count==5)||
				//(count==6 && size!=6)||
				(count==10&&size==11)||(count==15)){
			if(LogFormat){
				str[i++]=']';
				str[i++]='[';
			}
			else{
				str[i++]=' ';
			}
		}

		if(count==size){
			if(LogFormat){
				str[i++]=']';
			}
			str[i++]='\n';
		}
	}
	str[i]=0;
	putString(&str[0]);
}
#endif
/****************************************************************
 *					SetDataDir	 								*
 * 																*
 * 	Sets the Data pin direction ( 0=output , 1=input ) 			*
 * 																*
 ****************************************************************/
void setDataDir(bool isInput){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(isInput){
		HAL_GPIO_WritePin(BREAK_GPIO_Port, BREAK_Pin,RESET);
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	}
	else{
		HAL_GPIO_WritePin(BREAK_GPIO_Port, BREAK_Pin,SET);
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	}
	GPIO_InitStruct.Pin = DATA_Pin;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DATA_GPIO_Port, &GPIO_InitStruct);
}

/****************************************************************
 *					handleUnilink 								*
 * 																*
 * 	Main handler, called from main()  							*
 * 																*
 ****************************************************************/

void handleUnilink(void){

	if(unilink.test==2){
		unilink.test=0;
		putString("Slow\n");
	}
	if(unilink.received){
		unilink.received=0;									// do a parity check of received packet and proceed if OK
		unilink.logRx=1;
		if(unilink_checksum()){
			#if (OnlyLog==0) || !defined OnlyLog
			unilink_parse();
			#endif
		}
		else{
			#if(DebugLog>=1)
			logerr=1;
			#endif
			#if(DebugLevel>=1)
			putString("Checksum_Fail\n");
			#endif
		}
	}
	if(unilink.mag_changed && unilink.masterinit){			// usb was inserted, removed or contents changed
		static uint8_t wait=0;
		static uint32_t timer;
		if(wait==0){
			printf("Changed magazine info. Forcing ejecting status for 5s...\n");
			wait=1;
			timer=HAL_GetTick();
			mag_data.status=mag_removed;					// Set mag status=magazine removed
			addSlaveBreak(cmd_magazine);					// Send magazine list (empty)
			setStatus(unilink_ejecting);					// set mode=ejecting for 5 seconds to force the master exiting cd mode
		}

		else if (wait==1 && (HAL_GetTick()-timer>5000)){
			timer=HAL_GetTick();
			setStatus(unilink_idle);
			unilink.play=0;
			wait=2;
		}
		else if (wait==2 && (HAL_GetTick()-timer>5000)){
			wait=0;
			unilink.mag_changed=0;								// reset flag
			mag_data.status=mag_inserted;						// Set mag status=magazine inserted
			addSlaveBreak(cmd_maginfo);							// Add slot msg
			unilink.disc=0;
			for(uint8_t i=0;i<6;i++){							// Update magazine data
				if(cd_data[i].mins){
					if(unilink.disc==0){						// Find first cd with files
						unilink.disc=i+1;						// Set current disc ands track
						unilink.lastDisc=i+1;					// reset last disc and track
						unilink.track=1;
						unilink.lastTrack=1;
						unilink.min=0;							// reset playing time
						unilink.sec=0;
						#if(DebugLevel>=1)
						printf("Wait over. Start Disc:%d Track:%d\n",unilink.disc,unilink.track);
						#endif
						break;
					}
				}
			}
			unilink.play=0;										// Don't go into play mode automatically
			setStatus(unilink_idle);							// Set unilink status=changing cd
			addSlaveBreak(cmd_status);							// Add status msg
		}
	}

	#if(DebugLog>=1)
	unilinkLog();
	#endif
	#if (OnlyLog==0) || !defined OnlyLog
	handleSlaveBreak();
	#endif
	handleLed();

}

void handleLed(void){
	static uint32_t time=0;
	if(unilink.masterinit){
		if(unilink.status==unilink_idle){
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, RESET);						// If in idle state and initialized, Led on
		}
		else if(unilink.status==unilink_playing){
			if(HAL_GetTick()-time>50){
				time=HAL_GetTick();
				HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);							// If in play state, quick led blinking
			}
		}

	}
	else{
		if(!HAL_GPIO_ReadPin(LED_GPIO_Port,LED_Pin)){
			if(HAL_GetTick()-time>20){
				time=HAL_GetTick();													// If not initialized, make small pulses
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, SET);						// Indicating we're alive but not initialized
			}
		}
		else{
			if(HAL_GetTick()-time>1000){
				time=HAL_GetTick();
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, RESET);
			}
		}
	}
}
/****************************************************************
 *					unilink_tx									*
 * 																*
 * 	Process the input message, compute length and checksums,	*
 * 	and generate the final packet to be sent 					*
 * 	 															*
 * 																*
 ****************************************************************/
void unilink_tx(uint8_t *msg, volatile uint8_t *dest){
	uint8_t checksum = 0;                  											// local checksum
	uint8_t i = 0;                     					 							// local index
	uint8_t size;

	if(msg[2]>=0xC0){ size=16; }    												// 16 byte command (-checksum1, -checksum2, -final 0x00)
	else if(msg[2]>=0x80){ size=11; } 												// 11 byte command (-checksum1, -checksum2, -final 0x00)
	else{ size=6; }        															//  6 byte command (-checksum1, -final 0x00)

	while(i<parity1){
		checksum += msg[i];															// copy the 1st 4 bytes from msg to Tx buffer
		dest[i]=msg[i];																// and calculate checksum for it
		i++;
	}
	//4
	dest[i++] = checksum;															// copy checksum1 to Tx buffer
	//5
	while(i<(size-2)){   															// Handle rest of msg
		dest[i] = msg[i-1];   														// copy from msg to Tx data buffer
		checksum+=msg[i-1];                 										// add each byte to checksum2
		i++;
	}//
//			rad tad cmd1 cmd2 chk1 [0]d1 d2 d3 d4 chk2  [0]
//		 msg 0   1    2   3          4    5  6  7
//		 dst 0   1    2   3     4    5    6  7  8   9    10
//11	 i   1   2    3   4     5    6    7  8  9   10   11
//6	 	 i   1   2    3   4     5    6

	dest[size-2]=checksum;															// Store checksum2
	dest[size-1]=0;																	// End of msg always 0

	if(dest==&unilink.txData[0]){													// If dest is tx buffer
		unilink.txSize=size;														// Store Tx size
		setDataHigh();																// Release Data
		setDataOutput();															// DATA as output
		unilink.txCount = 0;														// set bytecount to 1 as we already sent one byte
		spi.buffer=unilink.txData[destAddr];		    							// send 1st byte to SPI, rest via SPI interrupt routine
		spi.TxMode=1;																// enable Tx mode
	}
	else{
		dest[16]=size;																// Store Tx size
	}
}


/****************************************************************
 *					unilink_parse 								*
 * 																*
 * 	Calls the correct handler for broadcast,					*
 * 	own address and appoint packets								*
 * 																*
 ****************************************************************/
void unilink_parse(void){
	if(unilink.rxData[destAddr]==Adr_Broadcast){
		unilink_broadcast();											// parse broadcast packets
		return;
	}
	if(unilink.rxData[destAddr]==unilink.ownAddr){
		unilink_myid_cmd();												// parse packets for my ID
		return;
	}
	if((unilink.rxData[destAddr]&0xF0) == unilink.groupID ){			// Appoint for us?
		unilink_appoint();												// do ID appoint procedure
		return;
	}
}


/****************************************************************
 *					Broadcast handler							*
 * 																*
 * 	Handles broadcast packets									*
 * 																*
 ****************************************************************/
void unilink_broadcast(void){
	#if (DebugLevel>=2)
	//putString(labels[label_cmd]);
	#endif

	switch(unilink.rxData[cmd1]){ 										// Switch CMD1
		case cmd_busRequest:  // 0x01 Bus requests (Broadcast)
		{
			switch(unilink.rxData[cmd2]){  								// Switch CMD2
				case cmd_busReset:										// 0x01 0x00 Bus reset
					unilinkColdReset();
					#if (DebugLevel>=1)
					putString(labels[label_busReset]);
					#endif
					break;
				case cmd_anyone:										// 0x01 0x02 Anyone?
					if(!unilink.masterinit){
						unilink.appoint = 1;							// Enable appoint
						uint8_t msg[] = send_anyoneResp;
						unilink_tx(msg,&unilink.txData[0]);                    			// send my device info string
						unilink.txCmd=cmd_anyoneResp;
						#if (DebugLevel>=1)
						putString(labels[label_anyone]);
						#endif
					}
					else{
						#if (DebugLevel>=1)
						putString(labels[label_alreadysentId]);
						#endif
					}
					break;
					default:
						#if (DebugLevel>=2)
						putString(labels[label_request1]);
						#endif
						break;
			}
			break;
		}
		case cmd_source:												// 0xF0 SRC Source select
			if (unilink.rxData[cmd2]!=unilink.ownAddr){					// check if interface is deselected
				//unilink.status = unilink_idle;                   		// set idle status on deselect
			}
			#if (DebugLevel>=1)
			putString(labels[label_srcselect]);
			#endif
			break;
		case cmd_poweroff:												// 0x87 Power Event
			#if (DebugLevel>=1)
			putString("PowerMode:");
			#endif
			if(unilink.rxData[cmd2]==cmd_idle){							// Power off
				#if (DebugLevel>=1)
				putString(labels[label_setidle]);
				#endif
				unilink.play=0;
				setStatus(unilink_idle);           				// set idle status on power off
			}
			if(unilink.rxData[cmd2]==cmd_init){
				#if (DebugLevel>=1)
				putString(labels[label_setactive]);
				#endif
				unilink.play=0;
				setStatus(unilink_idle);
			}
			break;
  }
}


/****************************************************************
 *					Command handler 							*
 * 																*
 * 	Handles the commands sent to our address					*
 * 																*
 ****************************************************************/
void unilink_myid_cmd(void){
	#if (DebugLevel>=1)
	//if( (unilink.rxData[cmd1]!=cmd_busRequest )&&(unilink.rxData[cmd1]!=cmd_textRequest) ){		//prueba
	//	putString(labels[label_cmd]);
	//}
	#endif
	#if(DebugLevel>=2)
	putString(labels[label_cmd]);
	#endif
	switch(unilink.rxData[cmd1]){ 										// Switch CMD1
		case cmd_busRequest:											// 0x01 Bus requests (for my ID)
		{
			switch(unilink.rxData[cmd2]){
				case cmd_timePoll:										// 0x01 0x12 Respond to time poll (PONG)
				{
					#if (DebugLevel>=2)		//prueba
					putString(labels[label_poll]);
					#endif
              		uint8_t msg[] = send_status;
					unilink_tx(msg,&unilink.txData[0]);
					unilink.txCmd=cmd_status;
              		update_status();
              		break;
				}
				case cmd_slavePoll:										// 0x01 0x13 permission to talk after poll request
				{
					#if (DebugLevel==2)			//prueba
					putString(labels[label_slavePoll]);
					#elif (DebugLevel>=2)
					putString(labels[label_slavePoll]);
					#endif
					slaveMsg();                   	 					// send slave response
					break;
				}
				default:
					#if (DebugLevel>=2)
					putString(labels[label_poll1]);
					#endif
					break;
			}
			break;
		}
		case cmd_play:													// 0x20 PLAY
		{
			#if (DebugLevel>=1)
			putString(labels[label_play]);
			#endif
			if((mag_data.status!=mag_removed)&&(unilink.status!=unilink_ejecting)){	// If magazine is present and we are not ejecting
				if(cd_data[unilink.disc-1].mins!=0){							// If current selected disc is valid
					if(unilink.track>cd_data[unilink.disc-1].tracks){			// If current track is valid
						unilink.track=1;										// Else, reset track
						unilink.lastTrack=1;
					}
					unilink.play=1;
					setStatus(unilink_changing);                        		// set changing status
				}
				else{
					for(uint8_t i=0;i<6;i++){									// Else, find the first valid disc
						if(cd_data[i].mins){
							unilink.disc=i+1;
							unilink.lastDisc=i+1;
							unilink.track=1;
							unilink.lastTrack=1;
							unilink.play=1;
							setStatus(unilink_changing);                        // set changing status
							break;
						}
					}
					if(cd_data[unilink.disc-1].mins==0){						// No valid cd was found
						setStatus(unilink_idle);                        		// set idle status
					}
				}
			}
			else{																// If we can't go into play mode
				unilink.play=0;
				addSlaveBreak(cmd_status);										// Send unilink status
				addSlaveBreak(cmd_maginfo);										// Send magazine status
				addSlaveBreak(cmd_magazine);									// Send magazine list
			}
			break;
		}
		case cmd_switch:												// 0x21 TA message start
		{
			if(unilink.rxData[cmd2] == 0x20){
				setStatus(unilink_idle);                      			// set idle status
			}
			if(unilink.rxData[cmd2] == 0x10){
			}
			#if (DebugLevel>=1)
			putString(labels[label_switch]);
			#endif
			break;
		}

		case cmd_fastFwd:												// 0x24 Fast Forward
		{
			#if (DebugLevel>=1)
			putString(labels[label_ffwd]);
			#endif
            break;
		}
		case cmd_fastRwd:												// 0x25 Fast Reverse
		{
			#if (DebugLevel>=1)
			putString(labels[label_frwd]);
			#endif
            break;
		}

		case cmd_repeat:												// 0x34 Repeat mode change
		{
			#if (DebugLevel>=1)
			putString(labels[label_repeat]);
			#endif
			switch(unilink.rxData[cmd2]){
				case 0x00:
				{
					#if (DebugLevel>=1)
					putString(labels[label_off]);
					#endif
					//repeat_mode(0);
					break;
				}
				case 0x10:
				{
					#if (DebugLevel>=1)
					putString(labels[label_on]);
					#endif
					//repeat_mode(1);
					break;
				}
			}
			addSlaveBreak(cmd_mode);									// Add command to queue
			unilink.txCmd=cmd_mode;
			break;
		}
		case cmd_shuffle:												// 0x35 Shuffle mode change
		{

			#if (DebugLevel>=1)
			putString(labels[label_shuffle]);
			#endif
			switch(unilink.rxData[cmd2]){
				case 0x00:
				{
					#if (DebugLevel>=1)
					putString(labels[label_off]);
					#endif
					//shuffle_mode(0);
					break;
				}
				case 0x10:
				{
					#if (DebugLevel>=1)
					putString(labels[label_on]);
					#endif
					//shuffle_mode(1);
					break;
				}
			}
			addSlaveBreak(cmd_mode);
			break;
		}
		case cmd_textRequest:											// 0x84 request for command
		{
			switch(unilink.rxData[cmd2]){
				case cmd_magazine:										// request magazine info
				{
					#if (DebugLevel>=1)
					putString(labels[label_magazine]);
					#endif
					if((mag_data.status!=mag_removed)&&(unilink.status!=unilink_ejecting)){
						uint8_t msg[] = send_magazine;						// send disc info
						unilink_tx(msg,&unilink.txData[0]);
					}
					else{
						uint8_t msg[] = send_magazine_empty;						// send disc info
						unilink_tx(msg,&unilink.txData[0]);
					}
					unilink.txCmd=cmd_magazine;
					break;
				}
				case cmd_discinfo:										// request disc total time and tracks
				{
					#if (DebugLevel>=1)
					putString(labels[label_discinfo]);
					#endif
					if((mag_data.status!=mag_removed)&&(unilink.status!=unilink_ejecting)){
						uint8_t msg[]=send_discinfo;
						unilink_tx(msg,&unilink.txData[0]);
					}
					else{
						uint8_t msg[]=send_discinfo_empty;
						unilink_tx(msg,&unilink.txData[0]);
					}
					unilink.txCmd=cmd_discinfo;
					break;
				}
				default:
					#if (DebugLevel>=1)
					putString(labels[label_request1]);
					#endif
					break;
			}
			break;
		}
		case cmd_goto:													// 0xB0 Direct Disc keys
		{
			bool changecd=0;
			for(uint8_t i=0;i<16;i++){
				rawCmd[i]=unilink.rxData[i];
			}
			unilink.cnt=0;
			unilink.sec=0;
			unilink.min=0;
			unilink.disc=unilink.rxData[cmd2]&0x0F;
			unilink.track=bcd2hex(unilink.rxData[d1]);
			if(unilink.track==0){ unilink.track=1; }
			if(unilink.disc==0){ unilink.disc=1; }

			#if (DebugLevel>=1)
			changeStr[25]=0x30+(unilink.disc/10);
			changeStr[26]=0x30+(unilink.disc%10);
			changeStr[36]=0x30+(unilink.track/10);
			changeStr[37]=0x30+(unilink.track%10);
			putString(changeStr);
			#endif
			if( unilink.lastDisc!=unilink.disc){ 						// check for disc change
				changecd=1;
				unilink.lastDisc=unilink.disc;
				if( unilink.lastDisc<unilink.disc){
					//prev disc stuff
				}
				else if(unilink.lastDisc>unilink.disc){
					unilink.lastDisc=unilink.disc;
					//next disc stuff
				}
			}
			if( unilink.lastTrack!=unilink.track){ 						// check for track change
					unilink.lastTrack=unilink.track;
				if( unilink.lastTrack<unilink.track){
					//prev track stuff
				}
				else if( unilink.lastTrack>unilink.track ){
					//next track stuff
				}
			}
			if(changecd){
				setStatus(unilink_changing);
				addSlaveBreak(cmd_status);									// send status changing (updates to changed)
				if((mag_data.status!=mag_removed)&&(unilink.status!=unilink_ejecting)){
					if(cd_data[unilink.disc-1].mins==0){					// Check if disc is not present
						mag_data.status=mag_slot_empty;						// Set mag status=slot empty
						addSlaveBreak(cmd_maginfo);							// Add empty slot msg
						//setStatus(unilink_changing);
						//addSlaveBreak(cmd_status);						// Add empty slot msg
						//setStatus(unilink_changing);
						//addSlaveBreak(cmd_intro);							// Intro end
						//setStatus(unilink_changing);
						//addSlaveBreak(cmd_status);						// Add empty slot msg
						//setStatus(unilink_changing);
						for(uint8_t i=0;i<6;i++){							// Skip and find next valid disc
							if(cd_data[unilink.disc-1].mins!=0){
								break;
							}
							if(++unilink.disc>6){
								unilink.disc=1;
							}
						}
						if(cd_data[unilink.disc-1].mins==0){				// No discs on system
							setStatus(unilink_idle);						// Idle state
							//addSlaveBreak(cmd_status);					//
						}
						else{
							unilink.lastDisc=unilink.disc;
							unilink.play=1;
							//addSlaveBreak(cmd_status);					// changed->playing
							//addSlaveBreak(cmd_dspdiscchange);				// Add dsp changed disc
							//addSlaveBreak(cmd_status);					// playing
						}
					}
				}
				else{
					uint8_t temp=mag_data.status;							// save status
					mag_data.status=mag_slot_empty;							// Send empty slot
					addSlaveBreak(cmd_maginfo);
					mag_data.status=temp;									// Restore status
					setStatus(unilink_idle);								// Idle state
					addSlaveBreak(cmd_status);								//
					unilink.play=0;
				}
			}
			break;
		}
		default:
			#if (DebugLevel>=1)
			putString(labels[label_cmd1]);
			#endif
			break;
	}
}

/****************************************************************
 *					Appoint handler 							*
 * 																*
 * 	Stores the slave address from master if required			*
 * 																*
 ****************************************************************/
void unilink_appoint(void){												// respond to ID appoint
	if((unilink.rxData[cmd1]==0x02)&&unilink.appoint){					// check for previous Anyone command
		if ((unilink.rxData[destAddr]&0xF0)==unilink.groupID){			// check if packet is for my group
			if(!unilink.masterinit){									// I have no ID
				unilink.masterinit=1;
				#if (DebugLevel>=1)
				putString(labels[label_appoint]);
				#endif
				unilink.appoint=0;										// Disable appoint
				unilink.ownAddr = unilink.rxData[destAddr];   			// save my new ID
				uint8_t msg[] = send_anyoneResp;						// Generate response
				unilink_tx(msg,&unilink.txData[0]);           	// send my device info string
				unilink.txCmd=cmd_anyoneResp;
			}
		}
	}
}

/****************************************************************
 *					slaveMsg 						 			*
 * 																*
 * 	Sends the correct response after a slave poll request		*
 * 																*
 ****************************************************************/
void slaveMsg(void){
	uint8_t c=0,size;
	unilink.test=1;
	if(slaveBreak.status!=break_pending){								// If we didn't send a slave break just before receiving slave poll
		#if (DebugLevel>=2)												// (master sent the slave poll by itself)
		putString("(NoPrev)");										// Received slave poll without previous break?
		#endif															// ((master sent the slave poll by itself))
		//return;
	}
	if(slaveBreak.pending==0){											// If empty queue, generate data
		#if (DebugLevel>=2)												// (master sent the slave poll by itself)
		putString("(Self)");											// Received a slave poll without previous break and no pending breaks
		#endif
		addSlaveBreak(auto_command());									// Add new data
	}
	size=slaveBreak.tx[slaveBreak.SendPos][16];
	while(c<size){														// Copy stored slave break message to Tx Buffer
		unilink.txData[c]=slaveBreak.tx[slaveBreak.SendPos][c];
		c++;
	}
	unilink.txSize=size;												// Copy Tx size
	unilink.txCmd=slaveBreak.tx[slaveBreak.SendPos][17];								// Copy cmd
	setDataOutput();													// DATA as output
	setDataHigh();														// Release data line
	spi.buffer=unilink.txData[0];		    							// send 1st byte to SPI, rest via SPI interrupt routine
	unilink.txCount = 0;
	slaveBreak.status=break_pendingTx;
	spi.TxMode=1;														// enable Tx mode
	if(unilink.test==1){
		unilink.test=0;
	}
	#if (DebugLevel>=1)
	c=slaveBreak.tx[slaveBreak.SendPos][17];							// Copy cmd
	if(c==cmd_time){
		timeStr[19]=0x30+(unilink.disc/10);
		timeStr[20]=0x30+(unilink.disc%10);
		timeStr[29]=0x30+(unilink.track/10);
		timeStr[30]=0x30+(unilink.track%10);
		timeStr[33]=0x30+(unilink.min/10);
		timeStr[34]=0x30+(unilink.min%10);
		timeStr[38]=0x30+(unilink.sec/10);
		timeStr[39]=0x30+(unilink.sec%10);
		putString(timeStr);
	}
	else{
	//	putString(labels[cmd2label(c)]);
	}
	#endif
}

uint8_t bcd2hex(uint8_t bcd){
	uint8_t tmp;
	tmp=((bcd&0xF0)>>4)*10;
	tmp+=(bcd&0x0F);
	return tmp;
}

uint8_t hex2bcd(uint8_t hex){
	uint8_t tmp;
	tmp=(hex/10)<<4;
	tmp+=(hex%10);
	return tmp;
}
/****************************************************************
 *					auto_command	 							*
 * 																*
 * 	Function to switch between slave responses to keep the		*
 *  the master updated, rotating status-magazine-discinfo-time	*
 * 	This function returns a different response on every call	*
 ****************************************************************/
uint8_t auto_command(void){
	static uint8_t lastcmd=cmd_time;									// Initially set lastcmd=time to send status
	switch(lastcmd){													// What was the last "auto" sent command?
		case cmd_status:												// We sent Status, now send magazine
			lastcmd=cmd_magazine;
			break;
		case cmd_magazine:												// We sent magazine, now send Disc Info
			lastcmd=cmd_discinfo;
			break;
		case cmd_discinfo:												// We sent Disc Info
			if(unilink.status==unilink_playing){						// If playing:
				lastcmd=cmd_time;										// Send time
			}
			else{														// Else:
				lastcmd=cmd_status;										// Send Status
			}
			break;
		case cmd_time:													// We sent Time
		default:														// Or other
			lastcmd=cmd_status;											// Send status
			break;
	}
	return(lastcmd);
}

/****************************************************************
 *					update_status 								*
 * 																*
 * 	Switches between states on every call						*
 * 	except on stable states (playing, idle) 					*
 * 																*
 ****************************************************************/
void update_status(void){
	#if (DebugLevel>=1)
	putString("				Status=");
	putString(labels[status2label(unilink.status)]);
	#endif
	if(unilink.statusTimer){ return; }			// Wait 100mS between changes
	//unilink.statusTimer=100;					// Load 100mS
	switch(unilink.status){
		case unilink_playing:
		case unilink_idle:
		case unilink_ejecting:
			break;
		case unilink_changed:
			if(unilink.play==1){
				unilink.status = unilink_seeking;
			}
			else{
				unilink.status = unilink_idle;
			}
			break;
		case unilink_seeking:
			unilink.status = unilink_playing;
			break;
		case unilink_changing:
			unilink.status = unilink_changed;
			break;
		default:
			unilink.status = unilink_idle;
			break;
	}
}
void setStatus(uint8_t status){
	//unilink.statusTimer=100;										// Load 100mS
	unilink.status=status;
}
/****************************************************************
 *					unilink_cheksum								*
 * 																*
 * 	Computes and verifies the checksum for received packets		*
 *  Returns 1 if correct, 0 if wrong							*
 ****************************************************************/
bool unilink_checksum(void){             								// check parity of complete Unilink packet
	uint8_t count=0;      												// local byte counter
    uint8_t size=unilink.rxSize-2;										// local rxsize, minus 1 checksum and 1 end byte
    uint8_t checksum=0;													// local checksum

    while(count<4){   													// calculate checksum for byte 1-4
    	checksum+=unilink.rxData[count++];								// add to checksum
    }
    if(checksum==unilink.rxData[parity1]){								// verify the 1st checksum, skip rest if is invalid
    	if(count==size){         										// check if short packet
    		if(unilink.rxData[count+1]==0){
    			return 1;      											// return with true if checksum 1 is ok AND end byte is 0x00 (short)
    		}
    	}
    }
    else{
    	return 0;             											// if checksum 1 or end byte is invalid, return false
    }
    count++;            												// skip byte 4
    while(count<size){													// calculate checksum for all other bytes
    	checksum+=unilink.rxData[count++];								// add to checksum
    }
    if(count==size){ 													// check for medium or long packet
    	if(checksum==unilink.rxData[count]){
    		if(unilink.rxData[count+1] == 0x00){
    			return 1;  												// return with true if checksum 2 is ok AND end byte is 0x00 (medium or long)
    		}
    	}
    }
    return 0;             												// if checksum 2 or end byte is invalid, return false
}
#if(DebugLevel>=1)
/****************************************************************
 *					cmd2label 									*
 *	Returns label index for certain unilink command				*
 *	input param: unilink command								*
 * 	return: label index 										*
 * 																*
 ****************************************************************/
uint8_t cmd2label(uint8_t cmd){
	switch(cmd){
		case cmd_anyoneResp:
			return(label_anyoneResp);
		case cmd_seek:
			return(label_seek);
		case cmd_magazine:
			return(label_magazine);
		case cmd_discinfo:
			return(label_discinfo);
		case cmd_mode:
			return(label_mode);
		case cmd_time:
			return(label_time);
		case cmd_status:
			return(label_status);
		case cmd_intro:
			return(label_intro);
		case cmd_dspdiscchange:
			return(label_dspdiscchange);
		case cmd_maginfo:
			return(label_maginfo);
		case cmd_unknown:
			return(label_unknownCmd);
		case cmd_raw:
			return(label_raw);
		default:
			return(label_unknownCmd);
	}
}

/****************************************************************
 *					cmd2label 									*
 *	Returns label index for certain unilink status				*
 *	input param: unilink status								*
 * 	return: label index 										*
 * 																*
 ****************************************************************/
uint8_t status2label(uint8_t status){
	switch(status){
		case unilink_playing:
			return(label_playing);
		case unilink_changed:
			return(label_changed);
		case unilink_seeking:
			return(label_seeking);
		case unilink_changing:
			return(label_changing);
		case unilink_idle:
			return(label_idle);
		case unilink_ejecting:
			return(label_ejecting);
		default:
			return(label_unknownStatus);
	}
}
#endif
/****************************************************************
 *					addSlaveBreak 								*
 * 																*
 * 	Adds a slave break to the queue  							*
 * 																*
 ****************************************************************/
void addSlaveBreak(uint8_t command){
	uint8_t pos=slaveBreak.BfPos;
	if(slaveBreak.pending>=BrkSiz){
		slaveBreak.lost++;
		return;
	}
	switch(command){
		case cmd_magazine:												// magazine info
		{
			if(mag_data.status==mag_removed){
				uint8_t msg[]=send_magazine_empty;
				unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			}
			else{
				uint8_t msg[]=send_magazine;
				unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			}
			break;
		}
		case cmd_discinfo:												// disc total time and tracks
		{
			//txDiscinfo();
			if(mag_data.status==mag_removed){
				uint8_t msg[]=send_discinfo_empty;
				unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			}
			else{
				uint8_t msg[]=send_discinfo;
				unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			}
			break;
		}
		case cmd_mode:													// mode
		{
			uint8_t msg[]=send_mode;
			unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			break;
		}
		case cmd_time:													// time info update
		{
			uint8_t msg[]=send_time;
			unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			break;
		}
		case cmd_status:												// status
		{
			uint8_t msg[]=send_status;
			unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			break;
		}
		case cmd_maginfo:											// status
		{
			uint8_t msg[]=send_maginfo;
			unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			break;
		}
		case cmd_dspdiscchange:											// status
		{
			uint8_t msg[]=send_dspchange;
			unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			break;
		}
		case cmd_intro:													// status
		{
			uint8_t msg[]=send_introend;
			unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			break;
		}
		default:														// Unknown
		{
			uint8_t msg[]=send_status;
			unilink_tx(msg,(uint8_t*)&slaveBreak.tx[pos][0]);
			//slaveBreak.commands[slaveBreak.pos]=cmd_status;
			command=cmd_unknown;
			break;
		}
	}
	if(command==cmd_status){
		update_status();
	}
	slaveBreak.tx[slaveBreak.BfPos][17]=command;						// Copy cmd for debugging
	#if (DebugLevel>=2)
	brk_str1[6] = slaveBreak.BfPos+'0';						// Debug current pending breaks number
	brk_str1[12] = slaveBreak.pending+'0';
	brk_str1[19] = slaveBreak.SendPos+'0';
	putString(brk_str1);
	putString("Add:");
	putString(labels[cmd2label(command)]);
	#endif

	if(++slaveBreak.BfPos>(BrkSiz-1)){
		slaveBreak.BfPos=0;
	}
	slaveBreak.pending++;
}

/****************************************************************
 *					handleSlaveBreak 							*
 * 																*
 * 	Continually checks for breaks stored in the queue			*
 * 	and reads the data line conditions to generate the break	*
 * 	If the queue is empty it will automatically generate a new	*
 * 	break, to keep the master updated							*
 * 																*
 ****************************************************************/
void handleSlaveBreak(void){
	static uint8_t status=0;
	static bool startTimeout=0;
	if(spi.TxMode || !unilink.masterinit){ return;}												// Exit if in Tx mode or no initialized

	if( ( (slaveBreak.status==break_pending) || (slaveBreak.status==break_idle) )){
		startTimeout=0;
	}

	// Set 100mS timeout for the response to be sent once the master sent a Slave poll or actually retrieving slave data
	if( ( (slaveBreak.status==break_sending) || (slaveBreak.status==break_pendingTx) )){
		if(!startTimeout){
			startTimeout=1;
			slaveBreak.Timeout=100;
		}
		if(startTimeout && slaveBreak.Timeout==0){	// If timeout elapsed and didn't finish, cancel the the transmission and reset back to Rx mode
			startTimeout=0;
			slaveBreak.status=break_idle;
			#if(DebugLevel>=2)
			putString("Brk_Timeout ");
			#endif
			unilinkWarmReset();
		}
		status=0;
		return;
	}

	switch(status){
		case 0:
		{
			if(isDataLow()){
				slaveBreak.dataTime=7;									// Wait for DATA line 7mS in low state
				status++;
			}
			break;
		}
		case 1:
		{
			if(slaveBreak.dataTime){
				if(isDataHigh()){
					status=0;											// If DATA goes high before the timer expires, reset state
				}
			}
			else{
				#if (DebugLevel>=2)
				putString("Brk_Start ");
				#endif
				status++;
			}
			break;
		}
		case 2:
		{
			if(isDataHigh()){											// Wait for DATA high
				slaveBreak.dataTime=3;
				status++;
			}
			break;
		}
		case 3:
		{
			if(slaveBreak.dataTime){									// Wait for DATA line 3mS in high state
				if(isDataLow()){
					return;												// If DATA goes low before the timer expires, reset to state 0
				}
			}
			else{
				setDataLow();											// Force DATA low
				setDataOutput();										// Set DATA as output
				slaveBreak.dataTime=3;
				status++;
			}
			break;
		}
		case 4:
		{
			if(!slaveBreak.dataTime){									// Wait 3mS
				setDataHigh();											// Release DATA
				setDataInput();											// Set DATA as input
				slaveBreak.status=break_pending;						// We send the break, now we need clock from master
				#if (DebugLevel>=2)
				putString("Brk_End\n");
				#endif
				status=0;												// Reset status
				return;
			}
			break;
		}
	}
}
/****************************************************************
 *					spiTimer									*
 * 																*
 * 	Called from sysTick() every 1mS								*
 * 	Used for slave break timing, spi reset timeouts				*
 * 	and keeps running a clock for telling the master			*
 * 																*
 ****************************************************************/
uint8_t discSizes[] = { 0,12,24,48,64,72,96 };
uint8_t discStart[] = { 0,8,16,40,56,61,88 };
void spiTimer(void){
	if(unilink.status==unilink_playing){					// Simulates play time
		if(++unilink.cnt>999){
			unilink.cnt=0;
			if(++unilink.sec>59){
				unilink.sec=0;
				if(++unilink.min>59){
					unilink.min=0;
				}
			}
		}
	}
	if(unilink.masterinit){										// If >200mS with no clock from master (100uS timebase)
		if(++spi.lastClockTime>2000){
			unilinkWarmReset();									// Warm reset (don't reset our ID)
			#if (DebugLevel>=2)
			putString("Master_Timeout ");
			#endif
		}
	}
	if(slaveBreak.dataTime){											// For sampling data high/low time
		slaveBreak.dataTime--;
	}
	if(slaveBreak.Timeout){												// For break timeout if master doesn't send poll
		slaveBreak.Timeout--;
	}
	if(unilink.statusTimer){											// For delaying status changes
		unilink.statusTimer--;
	}
}
/****************************************************************
 *					resetClockTimer 							*
 * 																*
 * 	Resets the spi clock timeout timer							*
 * 																*
 ****************************************************************/
void resetClockTimer(void){
	spi.lastClockTime=0;												// For resetting unilink if master doesn't send clock for a long time
	HAL_TIM_Base_Stop(unilink.clockTimer);
	__HAL_TIM_SET_COUNTER(unilink.clockTimer,0);						// For resetting spi if the clock delays too much in the middle of a byte
	__HAL_TIM_CLEAR_FLAG(unilink.clockTimer,TIM_FLAG_UPDATE);
	HAL_TIM_Base_Start(unilink.clockTimer);
}

/****************************************************************
 *					clockTimerExpired 							*
 * 																*
 * 	Returns 1 if the clock timer expired						*
 * 																*
 ****************************************************************/
bool clockTimerExpired(void){
	return (__HAL_TIM_GET_FLAG(unilink.clockTimer,TIM_FLAG_UPDATE));
}

/****************************************************************
 *					initClockTimer 								*
 * 																*
 * 	Stores the timer handler and initializes it					*
 * 																*
 ****************************************************************/
void initUnilink(TIM_HandleTypeDef* timer){
	unilink.clockTimer=timer;
	unilink.clockTimer->Init.Prescaler = 9599;							// 96MHz/9600= 100uS Timebase
	unilink.clockTimer->Init.CounterMode = TIM_COUNTERMODE_UP;
	unilink.clockTimer->Init.Period = SpiClkTimeout;					// Timer period = 100uS*Period
	unilink.clockTimer->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	unilink.clockTimer->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(unilink.clockTimer);
	unilinkColdReset();
	unilink.hwinit=1;
	mag_data.cmd2=0;
	mag_data.CD1=1;
	cd_data[0].tracks=88;
	cd_data[0].mins=88;
	cd_data[0].secs=8;

	for(uint8_t i=1;i<6;i++){
	  cd_data[i].tracks=99;
	  cd_data[i].mins=0;
	  cd_data[i].secs=0;
	}

}
/****************************************************************
 *					unilink_int 								*
 * 																*
 * 	Unilink byte handler, called from the interrupt after		*
 * 	a byte was received or sent									*
 * 																*
 ****************************************************************/
void unilink_int(void){
	if (!spi.TxMode){                            						// If in receive mode
		unilink.rxData[unilink.rxCount] = spi.buffer;					// store last received byte
		spi.buffer=0;													// Clear spi buffer
		if(unilink.rxCount==0){											// If first byte
			if(unilink.rxData[0]==0){									// skip if 1st byte of packet is 0x00
				return;
			}															// If first valid byte
			DebugPulse(1);												// Debug pulse to indicate start of reception
			#if (DebugLevel>=3)
			putString("RX ");
			#endif
		}
		if(unilink.rxCount<cmd1){										// if didn't receive cmd1 yet
			unilink.rxSize=6; 											// set packet size to short
		}
		else if(unilink.rxCount==cmd1){									// if cmd1 received
			if(unilink.rxData[cmd1]>=0xC0){ unilink.rxSize=16; } 		// set packet size to long
			else if(unilink.rxData[cmd1]>=0x80){ unilink.rxSize=11; } 	// set packet size to medium
			else{ unilink.rxSize=6; }									// set packet size to short
		}
		if (++unilink.rxCount>=unilink.rxSize){     					// Increase counter, if packet complete
			unilink.rxCount=0;                  						// reset counter
			unilink.received=1;          								// set RX complete status flag
			DebugPulse(2);												// Debug pulse to indicate end of reception
			#if (DebugLevel>=3)
			putString("!RX ");											// Debug
			#endif
		}
  }
	else{                                            					// If in transmit mode
		if(unilink.txCount==0){
			DebugPulse(1);												// Debug pulse to indicate start of transmission
			#if (DebugLevel>=3)
			putString("TX ");
			#endif
			if(slaveBreak.status==break_pendingTx){						// Was this the first byte for a slave poll answer?
				slaveBreak.status=break_sending;						// OK, we are sending a slave response
			}
		}
		if(++unilink.txCount<unilink.txSize){     						// check if bytes left
			spi.buffer=unilink.txData[unilink.txCount];				// output next byte
		}
		else{															// All bytes sent
			spi.buffer=0;												// Clear buffer for next reception
			spi.TxMode = 0;       		        						// disable Tx mode
			DebugPulse(3);												// Debug pulse to indicate end of transmission
			#if (DebugLevel>=3)
			putString("!TX ");
			#endif
			if(slaveBreak.status==break_sending){						// Were we sending a slave response?
				#if (DebugLevel>=2)
				putString("(Brk)");
				#endif
				slaveBreak.status=break_idle;							// Done
				if(slaveBreak.pending){									// We should have something here
					slaveBreak.pending--;								// Decrease
					if(++slaveBreak.SendPos>(BrkSiz-1)){;				// Increase sendPos
						slaveBreak.SendPos=0;
					}
				}
				#if (DebugLevel>=2)
				else{
					putString("(NoPrev)");								// Sent with no previous request (master sent the slave poll by itself)
				}
				brk_str2[6] = slaveBreak.BfPos+'0';						// Debug current pending breaks number
				brk_str2[12] = slaveBreak.pending+'0';
				brk_str2[19] = slaveBreak.SendPos+'0';
				putString(brk_str2);
				#endif
			}
			#if (DebugLevel>=2)
			putString("Sent:");											// We successfully sent a response
			putString( (char*)labels[cmd2label(unilink.txCmd)]);		// We sent this cmd
			#endif

		}
	}
}

/****************************************************************
 *					HAL_GPIO_EXTI_Callback 						*
 * 																*
 * 	HAL EXTI Callback, called when a change occurs				*
 * 	in the EXTI lines 											*
 * 	Checks the clock status and sends or receive data bits		*
 * 	Also resets the receive state if the clock timer expired	*
 * 																*
 ****************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin==CLK_Pin){												// If change was in clock pin
		bool d=isDataHigh();
		if(unilink.test==1){
			unilink.test=2;
		}

		if(!unilink.hwinit || isSpiDisabled()){							// if SPI receive disabled or not initialized
			return;														// return
		}
		if(clockTimerExpired() && !spi.TxMode){							// If Rx mode and >1.5mS since last clock change
			unilink.rxCount=0;											// Clear Rx counter
			resetSpiBit();												// Reset bit shift
			spi.buffer=0;												// Clear input buffer
			#if (DebugLevel>=3)
			putString("(R)\n");
			#endif
		}
		resetClockTimer();
		if(isSpiClockActive()){											// If clock low (active level)
			if(spi.lastTxBit){											// Was this the last clock falling edge, part of the previous Tx byte?
				spi.lastTxBit=0;										// Clear flag
				if(!spi.TxMode){										// If no longer in Tx mode (End of transmission)
					unilink.logTx=1;
					setDataInput();										// Set Data input
					setDataHigh();										// Release data line
				}
				return;													// return
			}
			if(!spi.TxMode){											// If in receive mode
				if(!d){													// Read Data pin
					spi.buffer |= 1<<spi.shift;							// Store 1 if 0 (Input inverted)
				}
				spi.shift--;											// Decrease shift counter
			}
		}
		else{															// If clock high (idle level)
			if(spi.TxMode){												// If in transmit mode
				if( (1<<spi.shift)&(spi.buffer) ){						// Check data bit and set data level before clock falling edge
					setDataLow();										// If 1, set 0 (Output inverted)
				}
				else{
					setDataHigh();										// If 0, set 1 (Output inverted)
				}
				spi.shift--;											// Decrease shift counter
			}
		}
		if(isLastSpiBit()){												// Byte complete
			resetSpiBit();												// Reset spi bit shift counter
			#if(DebugSpi>=1)
			//if(spi.TxMode && (unilink.txData[cmd1]==0x90)){
				spi.debug[spi.debugPos]=spi.buffer;							// Store byte  to Debug data
				if(spi.debugPos<(sizeof(spi.debug)-1)){						// Stop if debug buffer full
					spi.debugPos++;
				}
			//}
			#endif
			if(spi.TxMode){												// If in Tx mode, we finsihed while the clock was idle
				spi.lastTxBit=1;										// Ignore next clock falling edge
			}
			unilink_int();												// Call unilink interruption to handle data
		}
	}
}
