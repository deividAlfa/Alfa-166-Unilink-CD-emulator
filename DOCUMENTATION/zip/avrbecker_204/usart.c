/*************************************************************************
**  Becker Unilink Bus Interface (USART Procedures)
**  by Michael Wolf
**
**  Released under GNU GENERAL PUBLIC LICENSE
**	See LICENSE.TXT for details
**
**  Revision History
**
**  when			what  who	why
**	19/03/04	v1.00	MIC	Initial Release
**	04/02/05	v1.01	* Removed all deprecated WinAVR macros to be
**                  combatible with avr-libc v1.2.1 and higher
**
**************************************************************************/

// UART receive interrupt
SIGNAL(SIG_UART_RECV)
{
    usart_rx_buffer[usart_rx_index] = UDR;		            // put received char in buffer
    if(++usart_rx_index > RX_BUFFER_SIZE)       	        // wrap index pointer
        usart_rx_index = 0;
};// end of UART receive interrupt



void usart_putc(unsigned char byte)
{
	#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega8515__)
	while ((UCSRA & _BV(UDRE)) != _BV(UDRE));					// wait for USART to become available
	#else
	while ((USR & _BV(UDRE)) != _BV(UDRE));        		// wait for USART to become available
	#endif
	UDR = byte;		                                    // send character
}; //end usart_putc



void usart_init(void)					         		       	// init USART
{
	#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega8515__)
	UBRRH = UART_BAUDRATE>>8;		// set baud rate
	UBRRL = UART_BAUDRATE;
	UCSRB =((1<<RXCIE)|(0<<TXCIE)|(0<<UDRIE)|(1<<RXEN)|(1<<TXEN)|(0<<UCSZ2)|(0<<RXB8)|(0<<TXB8));
	// enable Rx & Tx, enable Rx interrupt
	UCSRC =((1<<URSEL)|(0<<UMSEL)|(0<<UPM1)|(0<<UPM0)|(0<<USBS)|(1<<UCSZ1)|(1<<UCSZ0)|(0<<UCPOL));
	// config USART; 8N1
	#else
	UCR = 0x98;			            							// enable Rx & Tx, enable Rx interrupt
  UBRR = (unsigned char)UART_BAUDRATE;	  // set baud rate
	#endif
}; // end UART_INIT



void usart_rx_proc(void)
{
	#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega8515__)
	UCSRB = 0x88;	                                        // disable UART Rx
	#else
	UCR = 0x88;		                                        // disable UART Rx
	#endif
	unsigned char loc_temp = 0;
    switch(usart_rx_buffer[0])
    {
        case 't':
            // Disc number
            unilink_timeinfo[7] = ((((usart_rx_buffer[1])+1)&0x0F)<<4) | 0x0E;

            // for track number, make 1 byte BCD from two byte ASCII
            unilink_timeinfo[4] = ((usart_rx_buffer[2] - 0x30) << 4 |
								(usart_rx_buffer[3] - 0x30)); // one byte track number from 2 byte ASCII
				
            // its the same for minutes
            unilink_timeinfo[5] = ((usart_rx_buffer[4]-0x30) << 4 | 
							(usart_rx_buffer[5] - 0x30));// one byte minutes from 2 byte ASCII

            // seconds
            unilink_timeinfo[6] = ((usart_rx_buffer[6] - 0x30) << 4 | 
								(usart_rx_buffer[7]-0x30));	// one byte seconds from 2 byte ASCII
            unilink_timeinfo[1] = unilink_ownaddr;              // set TAD

						unilink_command.time = true;												// set flag to force a time update						
						slavebreak();	  																		// do slavebreak for display update
						break;

        case 'D':
						// disc name
						for(loc_temp = 0; (loc_temp < 8) ; loc_temp++ )// copy disc name to name buffer
						{
							if(usart_rx_buffer[loc_temp +1] == '~') break;
							unilink_discname[loc_temp] = usart_rx_buffer[loc_temp +1];
						};
						for(; loc_temp < 8; loc_temp++)
							unilink_discname[loc_temp] = ' '; 						// clear track name buffer
						unilink_command.discname = true;								// set flag to force a discname update
						slavebreak();		  															// do slavebreak for display update
						break;

		#ifdef RAW_COMMAND
        case '*':
            // raw command mode
            asm volatile ("nop");
            for(loc_temp = 0; loc_temp < 13; loc_temp++)
							raw[loc_temp] = usart_rx_buffer[loc_temp +1];
						unilink_command.raw = true;
						slavebreak();																		// do slavebreak for display update
            break;
		#endif

		#if !defined(BUS_LOGGING) && defined(YAMPP3)
				/* When bus logging is disabled and Yampp3 control is enabled, the interface will wait for
				   a signal from Yampp to send a "Play" when the Yampp has finished power-up and when 
				   interface status is in play mode */
		case '!':
			if(unilink_status == 0x00) usart_putc(Play);
			break;
		#endif
    }; // end switch
		usart_rx_buffer[0] = 0x00;
		usart_rx_index = 0;                                   // reset UART Rx buffer index
	
	#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega8515__)
	UCSRB = 0x98;	                                        	// enable UART Rx
	#else
	UCR = 0x98;		                                        	// enable UART Rx
	#endif                                              
}; // end of UART_RX_PROC

