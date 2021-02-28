/*************************************************************************
**  Becker Unilink Interface (Configuration File)
**  by Michael Wolf
**
**  Released under GNU GENERAL PUBLIC LICENSE
**	See LICENSE.TXT for details
**
**  Revision History
**
**  when			what  who	why
**	19/03/04	v1.00	MIC	Initial Release
**	22/08/04	v1.01	MIC	+ added option for new op-amp hardware
**
**************************************************************************/

#define MCU_XTAL     7372800		// MCU crystal speed in Hz (max 16MHz!)
#define BAUD_RATE    115200			// UART communication speed (max 115200!)

/*************************************************************************/
/* Enable this option to use this firmware with new op-amp driven hardware */
//#define OPAMP_HW

/************************************************************************/
#define WATCHDOG

/* enable the watchdog.
*************************************************************************/
#define BUS_LOGGING

/* enable packet logging via UART but:
   If bus logging is enabled, key/event output is disabled!
   Note: Baud rates lower than 38400 will not work with
         BUS_LOGGING enabled!
*************************************************************************/
//#define RAW_COMMAND

/* enable raw command mode
   send raw commands in the following way:
   *xxxxxxxxxxxxx
   ||||||       |
   |||||up to 9 data bytes, no checksum, no end byte
   ||||CMD2
   |||CMD1
   ||TAD (transmitter adress in hex)
   |RAD (receiver adress in hex)
   start tag
   
   All command bytes are hexadecimal!
*************************************************************************/

/************************************************************************
 The following features are especially for use with a Yampp3 MP3 player
 or other MP3 player control.
 Do not enable "YAMPP3" if no Yampp is connected!                       
*/

#define LED_OUT

/* enable LED output on ATmega8
LED1 = interface ID status
LED2 = interface selected as source
LED3 = Unilink Rx/Tx traffic */

#define RELAIS_OUT						

/* enable Power relais output on ATmega8
Relais will turned on when the interface is selected
as source by head unit */

//#define YAMPP3						

/* enable special "Play" sequence for Yampp support
works in the following way:
- interface receives "Play" from head unit
- interface turns on power relais to power-up the Yampp and waits for RS232 signal
- when Yampp is running he sends a "!" to signal "I'm ready"
- now the interface sends the "Play" command to Yampp to start playing
************************************************************************/

// Define the UART output for each key/event here
#define Play            'P'         // output on play command
#define Stop            'S'         // output on stop command
#define Track_up        'T'         // output on next track command
#define Track_down      't'         // output on previous track command
#define Disc_up         'D'         // output on next disc command
#define Disc_down       'd'         // output on previous disc command
#define FFWD            'F'         // output on fast forward command
#define FRVS            'R'         // output on fast reverse command
#define Repeat          'r'         // output on repeat mode change, followed by a mode number
#define Shuffle         's'         // output on shuffle mode change, followed by a number
#define Power_Off       'O'         // output on power off command


// My output definition to control the Yampp3/USB MP3 player
/*
#define Play            'g'         // play
#define Stop            'G'         // stop
#define Track_up        ' '         // next track
#define Track_down      ' '         // previous track
#define Disc_up         ' '         // next playlist
#define Disc_down       ' '         // previous playlist
#define FFWD            'n'         // fast forward command
#define FRVS            'p'         // fast reverse command
#define Repeat          ' '         // repeat mode change
#define Shuffle         ' '         // shuffle mode change
#define Power_Off       ' '         // Power off
*/
