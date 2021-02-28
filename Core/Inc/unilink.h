/*
 * unilink.h
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

#ifndef __UNILINK_H
#define __UNILINK_H
															//				  ___________________________________________________________________
#include "main.h"											//			     V																	 |
															// Byte bits	[7]  ->	[6]  ->	[5]	 ->	[4]	 ->	[3]	 ->	[2]	 ->	[1]	 ->	[0]	 ->	[-1=End]
#define SpiLastBit			-1								// shift value 	 7		 6		 5		 4		 3		 2		 1		 0
#define SpiDisabled			-2								// -2 to totally disable the spi (all activity is ignored)
#define SpiMaxBit			7								// shift reset value
#define resetSpiBit() 		spi.shift = SpiMaxBit
#define disableSpi()		spi.shift = SpiDisabled
#define isLastSpiBit()		spi.shift == SpiLastBit
#define isSpiDisabled()		spi.shift == SpiDisabled

#define isSpiClockActive()	!HAL_GPIO_ReadPin(CLK_GPIO_Port, CLK_Pin)
#define isSpiClockIdle()	HAL_GPIO_ReadPin(CLK_GPIO_Port, CLK_Pin)
#define isDataHigh()		HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin)
#define isDataLow()			!HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin)
#define setDataHigh()		HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin,SET)
#define setDataLow()		HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin,RESET)

#define setDataInput()		setDataDir(1)
#define setDataOutput()		setDataDir(0)

#define debugHere()			setDataOutput();setDataLow()
#define SpiClkTimeout		15	// Timeout *100uS without spi clock from master to reset spi (1.5mS)
#define DebugLevel			1	//(0=No debug, 3=Max debug)
#define DebugSpi			0	//(0=No debug, 1=Store debug data)
#define OnlyLog				0	//(0=Normal, 1=passive operation)
#define DebugLog			1	//(0=No debug, 1=Show log)
#define LogFormat			0	//(0=No brackets, 1=Show brackets)

typedef struct {
	TIM_HandleTypeDef* 	clockTimer;				// Stores the address of the clock timer handler (for clock timeout)
	volatile uint8_t 	rxData[16];				// Stores received packet
	volatile uint8_t 	rxCount;				// Counter for received bytes
	volatile uint8_t 	rxSize;					// Stores expected rx packet size
	volatile bool 	 	logRx;					// Indicates rx buffer ready for logging
	volatile uint8_t	txData[17];				// Stores packet to be transmitted
	volatile uint8_t 	txCount;				// Counter for transmitted bytes
	volatile uint8_t 	txCmd;					// Counter for transmitted bytes
	volatile uint8_t 	txSize;					// Stores expected tx packet size
	volatile bool 		logTx;					// Indicates tx buffer ready for logging
	volatile uint8_t 	status;					// Stores unilink status
	volatile bool		play;					// Stores play status from master
	volatile uint8_t 	statusTimer;			// Stores unilink status timer
	volatile uint8_t 	ownAddr;				// Stores device address
	volatile uint8_t 	groupID;				// Stores group ID
	volatile uint8_t 	disc;					// Stores current disc
	volatile uint8_t 	lastDisc;				// Stores previous disc after a disc change
	volatile uint8_t 	track;					// Stores current track
	volatile uint8_t 	lastTrack;				// Stores previous track after a track change
	volatile uint8_t 	min;					// Stores current minute
	volatile uint8_t 	sec;					// Stores current second
	volatile uint16_t	cnt;					// Counter increased with timer, for generating time
	volatile bool		mag_changed;			// Flag, set if usb was changed
	volatile bool 	 	received; 				// Flag, set when packet received
	volatile bool 	 	appoint;				// Flag, set if we did appoint
	volatile bool 	 	hwinit;					// Flag, set if unilink hw is initialized
	volatile bool 	 	masterinit;				// Flag, set if unilink was initialized by master
	volatile uint8_t 	test;					// test
}unilink_t;


#define BrkSiz			9
typedef struct{
	volatile uint32_t 	Timeout;			// Timeout for waiting for the master to send the slave poll request after sending a slave break
	volatile uint8_t 	dataTime;			// For measuring data high/low states
	volatile uint16_t 	lost;				// Counter for discarded slavebreaks if the queue was full
	volatile uint8_t 	pending;			// Counter of slavebreaks pending to be send
	volatile uint8_t 	BfPos;				// Input buffer position
	volatile uint8_t 	SendPos;			// Sending buffer position
	#if (DebugLevel>=1)
	volatile char* 		str[BrkSiz];				// string for debugging
	#endif
	volatile uint8_t 	tx[BrkSiz][18];			// tx buffer queue	//tx[0-15] data, tx[16]byte count, tx[17]cmd
	volatile uint8_t 	lastAutoCmd;		// last self-generated break command (when no requested from master)
	volatile uint8_t 	status;				// status
}slaveBreak_t;


typedef struct{
	volatile bool 		lastTxBit;			// set when last bit was sent, wait for clock to fall before enabling Rx
	volatile bool 		TxMode; 			// set when SPI is in Tx mode
	volatile uint32_t 	lastClockTime;		// Used for timeout in Tx mode, to abort Tx if no clock from master
	volatile uint8_t 	buffer;				// spi input/ouput buffer
	volatile int8_t 	shift;				// counter for bit shifting
	#if(DebugSpi>=1)
	volatile uint8_t 	debug[4096];		// Debug, store all the spi data
	volatile uint16_t 	debugPos;			// Debug array index
	#endif
	volatile uint8_t 	pulse_status;		// Debug pulse status
	volatile uint8_t 	pulses;				// Debug pulse counter
}spi_t;

typedef struct{
	volatile uint8_t 	status;				// Stores magazine status
	union{
		volatile uint8_t cmd2;
		struct{
			volatile unsigned 	CD5:1;
			volatile unsigned 	CD6:1;
			volatile unsigned 	unused1:1;
			volatile unsigned 	unused2:1;
			volatile unsigned 	CD1:1;
			volatile unsigned 	CD2:1;
			volatile unsigned 	CD3:1;
			volatile unsigned 	CD4:1;
		};
	};
}magazine_t;

typedef struct{
	volatile bool			present;
	volatile uint8_t 		tracks;
	volatile uint8_t 		mins;
	volatile uint8_t 		secs;
	volatile uint8_t 		disc;
}cdinfo_t;


extern unilink_t 		unilink;
extern spi_t 			spi;
extern slaveBreak_t 	slaveBreak;
extern uint8_t 			raw[16];
extern magazine_t 		mag_data;
extern cdinfo_t 		cd_data[6];

typedef enum{
	unilink_playing  	= 	0x00,
	unilink_changed  	= 	0x20,
	unilink_seeking  	= 	0x21,
	unilink_changing 	= 	0x40,
	unilink_idle   		= 	0x80,
	unilink_ejecting	= 	0xC0,
	unilink_unknown		= 	0xFF,
}unilinkStatus;

typedef enum{
	mag_slot_empty 		= 	0x40,
	mag_inserted  		= 	0x80,
	mag_removed	  		= 	0xC0,
}magInfo;

typedef enum{
	break_idle			=	0,
	break_pending		=	1,
	break_pendingTx 	=	2,
	break_sending		=	3
}breakStatus;

typedef enum{
	destAddr 			=	0,
	srcAddr				=	1,
	cmd1				=	2,
	cmd2				=	3,
	parity1				=	4,
	d1					=	5,
	d2					=	6,
	d3					=	7,
	d4					=	8,
	d2_1				=	9,
	d2_2				=	10,
	d2_3				=	11,
	d2_4				=	12,
	d2_5				=	14,
	parity2				=	14,
}unilinkFrame;

typedef enum{
	cmd_busReset 		=	0x00,
	cmd_idle 			=	0x00,
	cmd_status 			=	0x00,
	cmd_busRequest 		=	0x01,
	cmd_anyone 			=	0x02,
	cmd_anyoneSpecial 	=	0x03,
	cmd_timePoll		=	0x12,
	cmd_slavePoll		=	0x13,
	cmd_play			=	0x20,
	cmd_switch			=	0x21,
	cmd_fastFwd			=	0x24,
	cmd_fastRwd			=	0x25,
	cmd_repeat			=	0x34,
	cmd_shuffle			=	0x35,
	cmd_textRequest		=  	0x84,
	cmd_poweroff		=  	0x87,
	cmd_init 			=	0x89,
	cmd_anyoneResp		=	0x8C,
	cmd_time			=	0x90,
	cmd_mode			=	0x94,
	cmd_magazine		=	0x95,
	cmd_maginfo			=	0x8e,
	cmd_intro			=	0x98,
	cmd_dspdiscchange	=	0x9c,
	cmd_discinfo		=	0x97,
	cmd_goto 			=	0xB0,
	cmd_seek 			=	0xC0,
	cmd_discname		=	0xCD,
	cmd_source 			=	0xF0,
	cmd_raw 			=	0xFE,
	cmd_unknown			=	0xFF,
}unilinkCommands;

typedef enum {
	Adr_Master 			=	0x10,
	Adr_Broadcast 		= 	0x18,
	Adr_Display 		=	0x70,
	Adr_Dsp				=	0x90,
	Adr_Reset 			=	0x30,
}unilinkAddress;

typedef enum{
	label_seek,
	label_time,
	label_magazine,
	label_discinfo,
	label_status,
	label_mode,
	label_raw,
	label_playing,
	label_changed,
	label_seeking,
	label_changing,
	label_idle,
	label_ejecting,
	label_unknownStatus,
	label_unknownCmd,
	label_busReset,
	label_anyone,
	label_anyoneResp,
	label_cmd1,
	label_request1,
	label_power,
	label_setactive,
	label_setidle,
	label_srcselect,
	label_switch,
	label_play,
	label_cmd,
	label_alreadysentId,
	label_poll,
	label_slavePoll,
	label_poll1,
	label_ffwd,
	label_frwd,
	label_repeat,
	label_shuffle,
	label_off,
	label_on,
	label_change,
	label_appoint,
	label_dspdiscchange,
	label_intro,
	label_maginfo,
	label_mag_removed,
	label_mag_slotempty,
	label_mag_inserted,
}labelIndex;

typedef enum{
	disc1_size		=	12,
	disc2_size		=	24,
	disc3_size		=	48,
	disc4_size		=	64,
	disc5_size		=	72,
	disc6_size		=	96,
}discSize;
/*

 		cmd_discinfo bitfield
	bits	76543210
	CD 6  	00000010 (0x02)
	CD 5  	00000001 (0x01)
	CD 4  	10000000 (0x80)
	CD 3  	01000000 (0x40)
	CD 2  	00100000 (0x20)
	CD 1  	00010000 (0x10)

 */
#define 	send_seek 			{ Adr_Display, unilink.ownAddr, cmd_seek, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x08 }
#define		send_time			{ Adr_Display, unilink.ownAddr, cmd_time, 0x00, hex2bcd(unilink.track), hex2bcd(unilink.min), hex2bcd(unilink.sec), ((unilink.disc<<4)|0xA) }
#define		send_magazine		{ Adr_Master, unilink.ownAddr, cmd_magazine ,mag_data.cmd2 ,0x00 ,0x00 ,0x00 , 0x06 }
#define		send_magazine_empty	{ Adr_Master, unilink.ownAddr, cmd_magazine, 0x00 ,0x00 ,0x00 ,0x00 , 0x06 }
#define		send_maginfo		{ Adr_Display, unilink.ownAddr, cmd_maginfo, mag_data.status, 0x20, 0x00, 0x00, (unilink.disc<<4) }
#define		send_dspchange 		{ Adr_Dsp, unilink.ownAddr, cmd_dspdiscchange, 0x00, 0x00, 0x00, 0x00, ((unilink.disc<<4)|0x8) }
#define 	send_introend		{ Adr_Master, unilink.ownAddr, cmd_intro, 0x10, 0x00, 0x00, 0x01, (unilink.disc<<4) }
#define		send_discinfo_empty	{ Adr_Master, unilink.ownAddr, cmd_discinfo, 0x01, 0x99, 0x00, 0x00, 0x01 }
#define		send_discinfo		{ Adr_Master, unilink.ownAddr, cmd_discinfo, 0x01, hex2bcd(cd_data[unilink.disc-1].tracks), hex2bcd(cd_data[unilink.disc-1].mins), hex2bcd(cd_data[unilink.disc-1].secs), (unilink.disc<<4) }
#define		send_status			{ Adr_Master, unilink.ownAddr, cmd_status, unilink.status }
#define		send_raw			rawCmd
#define		send_mode			{ Adr_Master, unilink.ownAddr, cmd_mode, 0x00, 0x00,0x80,0x00,0x00 }
#define		send_anyoneResp		{ Adr_Master, unilink.ownAddr, cmd_anyoneResp, 0x11, 0x14, 0xA8, 0x17, 0x60 }

void initUnilink(TIM_HandleTypeDef* timer);
void unilinkColdReset(void);								//
void unilinkWarmReset(void);								//
void handleUnilink(void);							//
void handleLed(void);
void unilink_parse(void);							//
bool unilink_checksum(void); 						// does a checksum verification on received packet

void unilink_broadcast(void);       				// do broadcast commands
void unilink_myid_cmd(void);        				// do commands for my actual ID
void unilink_appoint(void);	        				// do appoint respond

void repeat_mode(bool isOn);             				// do play mode change
void shuffle_mode(bool isOn);            				// do shuffle mode change

uint8_t hex2bcd(uint8_t hex); 						// convert 1 byte BCD to 1 byte HEX
uint8_t bcd2hex(uint8_t bcd); 						// convert 1 byte BCD to 1 byte HEX
uint8_t bin2ascii(uint8_t bin); 					// convert lower nibble of 1 byte binary to ASCII
uint8_t cmd2label(uint8_t cmd);
uint8_t status2label(uint8_t status);
uint8_t auto_command(void);							//
void update_status(void);							//
void setStatus(uint8_t status);						//
void addSlaveBreak(uint8_t command);				//
void handleSlaveBreak(void);						//
void slaveMsg(void);    						//
void unilink_tx(uint8_t *msg, volatile uint8_t *dest);						//
void txDiscinfo(void);

void initClockTimer(TIM_HandleTypeDef* timer);
void spiTimer(void);								//
void resetClockTimer(void);							//
bool clockTimerExpired(void);						//
void setDataDir(bool isInput);						//
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);		//
#endif
