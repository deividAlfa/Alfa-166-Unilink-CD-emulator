/*************************************************************************
**  Becker Unilink Interface (Unilink Header File)
**  by Michael Wolf
**
**  Released under GNU GENERAL PUBLIC LICENSE
**	See LICENSE.TXT for details
**
**  Revision History
**
**  when			what  who	why
**	19/03/04	v1.00	MIC	Initial Release  
**
**************************************************************************/

#define CD_DEVICE_MSG	{0x10, unilink_ownaddr, 0x8C, 0x10, 0x24, 0xA8, 0x17, 0xA0}
//#define CD_DEVICE_MSG {0x10, unilink_ownaddr, 0x8C, 0x00, 0x24, 0x2C, 0x22, 0x00} //MDC device info           


#define C_UNILINK_BCADDR	         		0x18		// broadcast ID
#define	C_UNILINK_MADDR		         		0x10		// masters ID
#define	C_UNILINK_DISPADDR	         	0x70		// display ID
#define	C_UNILINK_OWNADDR_CD         	0x30		// my group ID for CDC

#define	C_UNILINK_CMD_BUSRQ			 			0x01		// cmd 1 bus request
#define	C_UNILINK_CMD_BUSRQ_RESET	 		0x00		// cmd 2 re-initialize bus
#define	C_UNILINK_CMD_BUSRQ_ANYONE	 	0x02		// cmd 2 anyone
#define	C_UNILINK_CMD_BUSRQ_ANYONEs	 	0x03		// cmd 2 anyone special
#define	C_UNILINK_CMD_BUSRQ_TPOLLE	 	0x11		// cmd 2 time poll end
#define	C_UNILINK_CMD_BUSRQ_TPOLL	 		0x12		// cmd 2 time poll
#define	C_UNILINK_CMD_BUSRQ_POLL	 		0x13		// cmd 2 request time poll
#define	C_UNILINK_CMD_BUSRQ_POLLWHO	 	0x15		// cmd 2 who want to talk

#define	C_UNILINK_CMD_CONFIG		 			0x02		// cmd 1 config, for appoint
#define	C_UNILINK_CMD_SELECT		 			0xF0		// cmd 1 select source
#define	C_UNILINK_CMD_PLAY			 			0x20		// cmd 1 play
#define	C_UNILINK_CMD_TEXTRQ		 			0x84		// cmd 1 text request
#define	C_UNILINK_CMD_DISPKEY		 			0x80		// cmd 1 display key, switch through display modes
#define	C_UNILINK_CMD_NEXTTRACK		 		0x26		// cmd 1 next track
#define	C_UNILINK_CMD_PREVTRACK		 		0x27		// cmd 1 prev track
#define	C_UNILINK_CMD_POWEROFF1		 		0x87		// cmd 1 power off
#define	C_UNILINK_CMD_POWEROFF2		 		0x6B		// cmd 2 power off

// UNILINK Commdands
// geht nach wechsel zu cd1 #define DISC_MSG					{C_UNILINK_MADDR, unilink_ownaddr, 0x90, 0x50, 0x00, unilink_lastdisc*16, unilink_lastdisc*16, unilink_lastdisc*16}

#define MISSED_POLL_MSG		{0x10, unilink_ownaddr, 0x04, 0x00}
#define STATUS_MSG				{0x10, unilink_ownaddr, 0x00, unilink_status}
#define SEEK_MSG					{C_UNILINK_DISPADDR, unilink_ownaddr, 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x18}
//#define DISC_MSG					{C_UNILINK_DISPADDR, unilink_ownaddr, 0x97, 0x01, 0x01, 0x99, 0x99, 0x0A}
//#define DISC_MSG					{C_UNILINK_DISPADDR, unilink_ownaddr, 0x97, 0x01, 0x01, unilink_lastdisc*16, unilink_lastdisc*16, 0x0A}

#define MAG_MSG						{C_UNILINK_DISPADDR, unilink_ownaddr, 0x00, 0x40}
//xx#define PONG_CHANGING					{C_UNILINK_DISPADDR, unilink_ownaddr, 0x00, 0x40}
#define PONG_CHANGING					{C_UNILINK_DISPADDR, unilink_ownaddr, 0x00, 0x40}

//#define DISC_MSG					{C_UNILINK_DISPADDR, unilink_ownaddr, 0x97, 0x01, 0x01, unilink_lastdisc*16, unilink_lastdisc*16, 0x0A}
//#define DISC_MSG					{C_UNILINK_MADDR, unilink_ownaddr, 0x90, 0x30, 0x00, unilink_lastdisc*16, unilink_lastdisc*16, 0xFA}
#define DISC_MSG					{C_UNILINK_MADDR, unilink_ownaddr, 0x90, 0x50, 0x00, 0x06, 0x06, (unilink_lastdisc-0x10)*16}
#define NOINFO					        {C_UNILINK_DISPADDR, unilink_ownaddr, 0x90, 0x0A}
//#define DISC_MSG					{C_UNILINK_MADDR, unilink_ownaddr, 0x90, 0x50, 0x01, 0x00, 0x30, 0x010}
//#define DISC_MSG					{C_UNILINK_MADDR, unilink_ownaddr, 0x97, 0x01, 0x01, unilink_lastdisc*16, unilink_lastdisc*16, 0x0A}
//#define DISC_MSG					{C_UNILINK_DISPADDR, unilink_ownaddr, 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x18}
