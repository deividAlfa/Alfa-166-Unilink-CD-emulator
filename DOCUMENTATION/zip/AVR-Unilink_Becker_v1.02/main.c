/*************************************************************************
**  Becker Unilink Interface (Main Procedure)
**  by Michael Wolf
**
**  Released under GNU GENERAL PUBLIC LICENSE
**	See LICENSE.TXT for details
**
**	See README.TXT for serial command details
**
**  Contact: michael@mictronics.de
**  homepage: www.mictronics.de
**
**  Revision History
**
**  when			what  who	why
**	19/03/04	v1.00	MIC	Initial Release
**	22/08/04	v1.01	MIC	+ added support for new op-amp hardware
**	04/02/05	v1.02	* Removed all deprecated WinAVR macros to be
**                  combatible with avr-libc v1.2.1 and higher
**
**  Used develompent tools (download @ www.avrfreaks.net):
**  Programmers Notepad v2.0.5.32
**  WinAVR (GCC) 3.4.1
**  AvrStudio4 for simulating and debugging
**	Tab Width: 2
**
**  [           Legend:          ]
**  [ + Added feature            ]
**  [ * Improved/changed feature ]
**  [ - Bug fixed (I hope)       ]
**
**************************************************************************/
#include <avr/signal.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include "config.h"
#include "main.h"
#include "usart.c"
#include "unilink.h"
#include "unilink.c"
#include "general.c"

// Main loop
int main(void)
{
	cli();
	#if defined(WATCHDOG)
	wdt_enable(WDTO_2S);					// Watchdog enable with 2 seconds timeout
	#endif
	#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega8515__)
	GICR = 0x00;     							// disable external interupts
	#else
	GIMSK = 0x00; 		            // disable external interupts
	#endif

	#if defined(__AVR_AT90S8515__) || defined(__AVR_ATmega8515__)
  cbi (MCUCR, SRE); 	          // disable external RAM
	PORTA = 0x00;		            	// PortA all low
	DDRA = 0x00;		            	// PortA all input
	#endif
	PORTB = 0x00;		            	// PortB all low
	DDRB = 0x00;		            	// PortB all input
  PORTC = 0x00;			            // PortC all low
	DDRC = 0x00;		            	// PortC all input
  PORTD = 0x00;			            // PortD all low

	#if defined(__AVR_ATmega8__)
		#if defined(LED_OUT)
		DDRD = (1<<d_LED1)|(1<<d_LED2)|(1<<d_LED3);	// set PortD LED output on ATmega8
		#elif defined(RELAIS_OUT)
		DDRD = (1<<d_RELAIS);												// set PortD relais output on ATmega8
		#else
		DDRD = (0<<d_RELAIS)|(0<<d_LED1)|(0<<d_LED2)|(0<<d_LED3);	// set PortD all input on ATmega8
		#endif	
	#else
	DDRD = 0x00;											// PORTD all input on AT...8515
	#endif

  TIMSK = 1<<TOIE1;	  	            // enable Timer1 overflow interrupt
  TCCR0 = 0x00;     	              // Stop Timer0
	TCCR1A = 0x00;
	TCNT1 = c_delay;									// Load Timer1
  TCCR1B = c_Timer1_stop;		        // Stop Timer1
            
	SPCR = c_RUN_SPI;	            		/* SPI control register (11000100)
                                    SPI in slave mode, SPI interupt enabled,
                                    SPI speed = Fclk/64
                                    */
  DDRB = 0x40;		            			// MISO as output and low
  SPDR = 0x00;

  usart_init();                   	// init USART
	
  unilink_ownaddr = C_UNILINK_OWNADDR_CD;// use CD mode only
	unilink_groupid = C_UNILINK_OWNADDR_CD;

	unilink_lastdisc = 0x00;					// init disc and track reminder
	unilink_lasttrack = 0x00;
	
	flags.unilink_anyone = false;
  sei();                    	     	// enable interrupts
	TCNT0 = c_6ms;		     		       	// load Timer0
  TCCR0 = c_Timer0_run;		    			// start Timer0

  // wait for idle time between two packets, then start receiving
  while(bit_is_clear(TIFR, TOV0))                             // wait for MOSI high for minimum of 6ms
	{                                
    if (bit_is_clear(PINB, b_MOSI)) TCNT0 = c_6ms;		    		// load Timer0 if MOSI is not high
  };// wait until timer0 overflow
  TIFR |= _BV(TOV0);                                         	// clear timer0 overflow flag
  TCNT0 = c_6ms;		                                        	// load Timer0
  while(bit_is_clear(TIFR, TOV0))                             // wait for MOSI low for minimum of 6ms
  {
    if (bit_is_set(PINB, b_MOSI)) TCNT0 = c_6ms;		    			// load Timer0  if MOSI is not low
  };// wait until timer overflow
  TCNT0 = c_Timer0_stop;		    															// stop timer0


	// now we are going in end endless loop waiting for packets
	while(true)
  {
		do
		{
			// the watchdog will be reset if bus is in idle or during data transfer
			if ( bit_is_set(PINB, 3) )	// test data line
				wdt_reset();
				
			// check for serial commands and parse if we have one
			if( (usart_rx_buffer[0] == 't' ) && ( usart_rx_index >= 8 ) )
				usart_rx_proc();
				
			else if( (usart_rx_buffer[0] == 'D' ) && ( (usart_rx_index >= 17) || (usart_rx_buffer[usart_rx_index-1] == '~') ) )
				usart_rx_proc();

			else if( (usart_rx_buffer[0] == 'T' ) && ( (usart_rx_index >= 17) || (usart_rx_buffer[usart_rx_index-1] == '~') ) )
				usart_rx_proc();

			else if( usart_rx_buffer[0] == '*' )
			{
				if
				(
					((usart_rx_buffer[3] < 0x80 ) && (usart_rx_index >= 5)) ||
					((usart_rx_buffer[3] < 0xC0 ) && (usart_rx_index >= 9)) ||
					((usart_rx_buffer[3] >= 0xC0 ) && (usart_rx_index >= 14))
				)
					usart_rx_proc();
			}
		#if defined(YAMPP3)
			else if (usart_rx_buffer[0] == '!')
				usart_rx_proc();
		#endif

		} while(!flags.unilink_rx_compl);
		
		flags.unilink_rx_compl = false;				// clear RX complete flag, wait for new packet
		checksum_check();
    if ( flags.checksum_ok )								// do a parity check of received packet and proceed if OK
    {
			unilink_parse();
			
			#ifdef BUS_LOGGING
        flags.unilink_tx_log = false;			// disable Tx packet logging
        bus_logging();                      // send valid Rx packet via UART
			#endif
    }  // end of routine if checksum was ok
	}; // end of endless while loop
}; // end of main()


// Timer1 Overflow interrupt
SIGNAL(SIG_OVERFLOW1)
{
	flags.timer1_ovf = true;				// set overflow flag
	TCNT1 = c_delay;           			// restart Timer1
};// end of TIMER1 overflow interrupt
