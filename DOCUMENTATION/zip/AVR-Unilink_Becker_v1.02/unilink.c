/*************************************************************************
**  Becker Unilink Bus Interface (Unilink Command Procedures)
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

INTERRUPT(SIG_SPI)
{
  if (!(flags.spi_tx))                            		// do SPI Rx routine here
  {
		#if defined(LED_OUT)
		sbi(PORTD, d_LED3);																// turn ON LED3
		#endif
		unilink_rxdata[unilink_bytecount.rx] = SPDR;			// read byte
    reset_spi();                                  		// reset SPI
    if ((unilink_rxdata[0] == 0x00) && (unilink_bytecount.rx == 0))
			return;																				// skip if 1st byte of packet is 0x00
    unilink_rxsize = 6;                           		// set packet size to short
    if (unilink_rxdata[2] >= 0x80)
			unilink_rxsize = 11;     												// set packet size to medium
    if (unilink_rxdata[2] >= 0xC0)
			unilink_rxsize = 16;    												// set packet size to long
    unilink_bytecount.rx++;
    if (unilink_bytecount.rx >= unilink_rxsize)     	// if a packet is complete
    {
			#if defined(LED_OUT)
			cbi(PORTD, d_LED3);															// turn OFF LED3
			#endif
			unilink_bytecount.rx = 0;                  			// reset byte counter
            flags.unilink_rx_compl = true;          	// set status flag
    }
  } // end of SPI Rx routine
  else                                            		// do SPI Tx routine here
  {
    if (unilink_bytecount.tx <= unilink_txsize)     	// check if bytes left
    {
      SPDR = unilink_txdata[unilink_bytecount.tx];		// output next byte
      unilink_bytecount.tx++;
    }
    else
    {
      reset_spi();
      SPDR = 0x00;          		        	    				// output last end byte 0x00
      flags.spi_tx = false;       		        				// disable Tx mode
			#ifdef BUS_LOGGING
        flags.unilink_tx_log = true;                	// enable Tx packet logging
        bus_logging();                              	// send Tx packet via UART
			#endif
			#if defined(LED_OUT)
				PORTD &=~ _BV(d_LED3);														// turn OFF LED3
			#endif
			DDRB &=~ _BV(b_MISO);         		        				// disable SPI output
      asm volatile ("nop");
		}
  } // end SPI Tx routine
} //end of SPI interrupt routine


void reset_spi(void)  // reset SPI
{
  // inp (SPSR);
  SPDR = 0x00;				// force MISO low
  SPCR = 0x00;				// reset SPI interrupt flag
  SPCR = c_RUN_SPI;		// reconfig SPI
}; //end RESET_SPI


void unilink_tx(unsigned char *msg)
{
  unsigned char loc_checksum = 0;                   // local checksum
  unsigned char loc_index = 0;                      // local index

	if (msg[2] >= 0xC0) unilink_txsize = 14;          	// 16 byte command
	else if (msg[2] >= 0x80) unilink_txsize = 9;      // 11 byte command
	else unilink_txsize = 4;         			        		//  6 byte command

  loc_checksum += msg[destaddr];                     // copy the 1st 4 bytes from msg to Tx buffer
  unilink_txdata[destaddr]=msg[destaddr];            // and calculate checksum for it
  loc_checksum += msg[srcaddr];
  unilink_txdata[srcaddr]=msg[srcaddr];
  loc_checksum += msg[cmd1];
  unilink_txdata[cmd1]=msg[cmd1];
  loc_checksum += msg[cmd2];
  unilink_txdata[cmd2]=msg[cmd2];
  unilink_txdata[parity1] = loc_checksum;             // store calculated checksum
  loc_index = 4;                                      // start with 1st databyte in packet

	do   // copy msg to Tx buffer and calculate checksum
  {
    unilink_txdata[loc_index+1] = msg[loc_index];   	// copy one byte from msg to Tx data buffer
    loc_checksum += msg[loc_index];                 	// add each byte to checksum
    loc_index++;
  }while(loc_index < unilink_txsize-1);

	unilink_txdata[loc_index+1] = loc_checksum;         // store 2nd checksum in Tx buffer
	#if defined(LED_OUT)
		sbi(PORTD, d_LED3);																// turn ON LED3
	#endif
	SPDR = 0x00;
  SPCR = 0x00;	                                    	// reset SPI
  DDRB |= _BV(b_MISO);                                 // MISO as output
  asm volatile ("nop");
  SPCR = c_RUN_SPI;		                            		// reconfig SPI
  flags.spi_tx = true;                               // enable Tx mode
  unilink_bytecount.tx = 1;                           // set bytecount to 1 cause one byte is send direct
  SPDR = unilink_txdata[destaddr];		            		// send 1st byte to SPI, rest via SPI interrupt routine
}; // end of unilink_tx



// start Unilink packet evaluation
void unilink_parse(void)
{
	if(unilink_rxdata[destaddr] == 0x18)
	{
		unilink_broadcast();                			// parse broadcast packets
		return;
	}	

	if(unilink_rxdata[destaddr] == unilink_ownaddr)
	{
		unilink_myid_cmd();			        					// parse packets for my ID
		return;
	}

	if(unilink_ownaddr == unilink_groupid)
	{
		unilink_appoint();												// do ID appoint procedure
		return;
	}
}; // end UNILINK_RXPROC



//*** Broadcast handler
void unilink_broadcast(void)
{
  switch(unilink_rxdata[cmd1]) 												// Switch on CMD1
  {
    case 0x01:  // 0x01 Bus requests (Broadcast)
        switch(unilink_rxdata[cmd2])  									// Switch on CMD2
        {

				// 0x01 0x00 Bus reset
          case 0x00:
              unilink_ownaddr = C_UNILINK_OWNADDR_CD; // clear my old ID
							flags.unilink_anyone = false;
              break;

				// 0x01 0x02 Anyone?
          case 0x02:  
                if(unilink_ownaddr == C_UNILINK_OWNADDR_CD)
                {
									flags.unilink_anyone = true;
                  unsigned char msg[8] = CD_DEVICE_MSG;
									unilink_tx(msg);                    	// send my device info string
                };
              break;
				}; // end of switch CMD 2
				
		// 0xF0 SRC Source select
    case 0xF0:
				if (unilink_rxdata[cmd2] != unilink_ownaddr)		// check if interface is deselected
				{
          unilink_status = 0x80;                        // set idle status on deselect
					#ifndef BUS_LOGGING
            usart_putc(Stop);
					#endif
					#if defined(LED_OUT)
						cbi(PORTD, d_LED2);													// turn OFF LED2
					#endif		
				}
        break;

		// 0x87 Power Event
    case 0x87:
        if(unilink_rxdata[cmd2] == 0x00)								// Power off
				{	// power off events
					unilink_status = 0x80;                 				// set idle status on power off
					#if defined(RELAIS_OUT)
						PORTD &=~ _BV(d_RELAIS);												// turn OFF relais
					#else
						usart_putc(Stop); 													// or send a Stop 
					#endif
					#ifndef BUS_LOGGING
            usart_putc(Power_Off);
					#endif
					#if defined(LED_OUT)
						PORTD &=~ _BV(d_LED2);													// turn OFF LED2 and LED3
					#endif
				}
        break;

  }; // end switch on CMD 1
}; // end of Broadcast handler


//*** Command handler ***
void unilink_myid_cmd(void)
{
	switch(unilink_rxdata[cmd1]) 													// Switch on CMD1
  {
		// 0x01 Bus requests (for my ID)
			case 0x01:  
          switch(unilink_rxdata[cmd2])
          {

						// 0x01 0x12 Respond to time poll (PONG)
              case 0x12:
                  asm volatile ("nop");               // without this, an parse error occurs, no idea why ???
									flags.bus_sleep = false;
									unsigned char msg[4] = STATUS_MSG;
                  unilink_tx(msg);
									break;
                
						// 0x01 0x13 permission to talk after poll request
              case 0x13:
                  send_command();                   	 // send unilink command
                  break;
					}; // end switch CMD2
          break;
        
		// 0x20 PLAY
      case 0x20:
					#if !defined(BUS_LOGGING) && !defined(YAMPP3)
						/* This "Play" is send, when bus logging and Yampp3 control is disabled */
            usart_putc(Play);
					#endif
					#if defined(LED_OUT)
						PORTD |= _BV(d_LED2);													// turn ON LED2
					#endif	
					#if defined(RELAIS_OUT)
						#if defined(YAMPP3)
							// test if Yampp is still powered on, if yes, send a play command
							if(bit_is_set(PORTD, d_RELAIS)) usart_putc(Play);
						#endif
					PORTD |= _BV(d_RELAIS);													// turn ON relais
					#endif	
					unilink_status = 0x00;                        // set PLAY status
					unsigned char msg5[13] = SEEK_MSG;					// send seek to disc and track
					unilink_tx(msg5);
          break;
			
		// 0x21 TA message start
      case 0x21:
					if(unilink_rxdata[cmd2] == 0x20)
					{
						unilink_status = 0x80;                      // set idle status
						#ifndef BUS_LOGGING
							usart_putc(Play);													// send "play" to put Yampp in pause mode
						#endif
						#if defined(LED_OUT)
							cbi(PORTD, d_LED2);												// turn OFF LED2
						#endif
					}
          break;

		// 0x24 Fast Forward
      case 0x24:
					#ifndef BUS_LOGGING
            usart_putc(FFWD);
					#endif
          break;
        
		// 0x25 Fast Reverse
      case 0x25:
					#ifndef BUS_LOGGING
            usart_putc(FRVS);
					#endif
          break;
        
		// 0x34 Repeat mode change
      case 0x34:
					#ifndef BUS_LOGGING
						usart_putc(Repeat);
					#endif
          break;
        
		// 0x35 Shuffle mode change
      case 0x35:
					#ifndef BUS_LOGGING
						usart_putc(Shuffle);
					#endif
          break;
        
		// 0x84 request for command
      case 0x84:
					switch(unilink_rxdata[cmd2])
          {
						// request magazine info
						case 0x95:																	// send magazine info on request
							slavebreak();
							unilink_command.magazine = true;
							break;
						
						// request disc total time and tracks
						case 0x97:
							slavebreak();															// send disc info on request
							unilink_command.disc = true;
							break;
					}
          break;
 
		// 0xB0 Direct Disc keys
      case 0xB0:
					if( unilink_lastdisc > unilink_rxdata[cmd2] ) // check for disc change
					{
						unilink_lastdisc = unilink_rxdata[cmd2];	// store new disc#
						#ifndef BUS_LOGGING
							usart_putc(Disc_up);										// serial command out
            #endif
					}
					else if( unilink_lastdisc < unilink_rxdata[cmd2] )
					{
						unilink_lastdisc = unilink_rxdata[cmd2];	// store new disc#
						#ifndef BUS_LOGGING
							usart_putc(Disc_down);									// serial command out
            #endif
					}

					if( unilink_lasttrack > unilink_rxdata[d1] ) // check for track change
					{
						unilink_lasttrack = unilink_rxdata[d1];			// store new track#
						#ifndef BUS_LOGGING
							usart_putc(Track_up);											// serial command out
            #endif
					}
					else if( unilink_lasttrack < unilink_rxdata[d1] )
					{
						unilink_lasttrack = unilink_rxdata[d1];			// store new disc#
						#ifndef BUS_LOGGING
							usart_putc(Track_down);										// serial command out
            #endif
					}
			    break;
    }; // end switch CMD 1
}; // end of Command handler



//*** Appoint handler ***
void unilink_appoint(void)
{   // respond to ID appoint
	unilink_bitid = unilink_rxdata[cmd2];	 								// store request poll ID
	
	if( (unilink_rxdata[cmd1] == 0x02) && flags.unilink_anyone ) 
	{ // check for Anyone command
		if ((unilink_rxdata[destaddr]&0xF0) == unilink_groupid)// check if packet is for my group
		{ // if packet is for my group
				if(unilink_ownaddr == C_UNILINK_OWNADDR_CD)
				{ // I have no ID and packet is for my group
					unilink_ownaddr = unilink_rxdata[destaddr];   // save my new ID
					#if defined(LED_OUT)
						sbi(PORTD, d_LED1);													// turn ON LED1 since we now have the ID
					#endif	
					unsigned char msg[8] = CD_DEVICE_MSG;
					unilink_tx(msg);                        			// send my device info string
				}
		}// end group check
	}// end of anyone check
}; // end of appoint handler



void send_command(void)
{
	// magazine info
  if (unilink_command.magazine)							// send magazine info
	{
		unsigned char msg2[8] = MAG_MSG;
    unilink_tx(msg2);
		unilink_command.magazine = false;
		return;
	}

	// disc total time and tracks
  if (unilink_command.disc)									// send disc info
	{
		unsigned char msg3[8] = DISC_MSG;
    unilink_tx(msg3);
		unilink_command.disc = false;
		return;
	}
	
	// time info update
  if (unilink_command.time)									// send time info
	{
    unilink_tx(unilink_timeinfo);
		unilink_command.time = false;
		return;
	}

	// disc name update
	if (unilink_command.discname)							// send disc name
	{
		unsigned char msg4[13] = {C_UNILINK_DISPADDR, unilink_ownaddr, 0xCD,
                        unilink_discname[0],unilink_discname[1],
                        unilink_discname[2],unilink_discname[3],
                        unilink_discname[4],unilink_discname[5],
                        unilink_discname[6],unilink_discname[7],
                        0x00,0x0E};
    unilink_tx(msg4);              
		unilink_command.discname = false;
		return;
	}

	// RAW unilink command
  if (unilink_command.raw)									// send RAW command
	{
		unilink_tx(raw);
		unilink_command.raw = false;
		return;
	}
}; // end of send_command



void checksum_check(void)              							// check parity of complete Unilink packet
{
        unsigned char loc_bytecount = 0;      			// local byte counter
        unsigned char loc_rxsize = unilink_rxsize-2;// local rxsize, minus 1 checksum and 1 end byte
        unsigned char loc_checksum = 0;							// local checksum

        for(;loc_bytecount < 4; loc_bytecount++)   	// calculate checksum for byte 1-4
        {
            loc_checksum += unilink_rxdata[loc_bytecount];// add to checksum
        };

        if (loc_checksum == unilink_rxdata[parity1])// verify the 1st checksum, skip rest if is invalid
        {
            if (loc_bytecount==loc_rxsize)         	// check if short packet
            {
                if (unilink_rxdata[loc_bytecount+2] == 0x00)
                {
                    flags.checksum_ok = true;      	// return with true if checksum 1 is ok AND end byte is 0x00 (short)
                }; // end if end byte
            }; // end if short packet
        } // end if parity ok
        else
        {
            flags.checksum_ok = false;             	// if checksum 1 or end byte is invalid, return false
        }; // end if parity wrong

        loc_bytecount++;            								// skip byte 4

        for (;loc_bytecount < loc_rxsize;loc_bytecount++)// calculate checksum for all other bytes
        {
            loc_checksum += unilink_rxdata[loc_bytecount];// add to checksum
        };

        if (loc_bytecount == loc_rxsize) 						// check for medium or long packet
        {
            if (loc_checksum == unilink_rxdata[loc_bytecount])
            {
                if (unilink_rxdata[loc_bytecount+1] == 0x00)
                {
                    flags.checksum_ok = true;  			// return with true if checksum 2 is ok AND end byte is 0x00 (medium or long)
                }
            }
            else
            {
                flags.checksum_ok = false;     			// if checksum 2 or end byte is invalid return false
            }
        } // end if
} // end checksum_check


// Slavebreak routine
void slavebreak(void)
{
    SPDR = 0xFF;		                                     // force Data low
    SPCR = 0x00;		                                     // disable SPI

    TCNT0 = c_7ms;		                                   // load Timer0
    TCCR0 = c_Timer0_run;		                             // start Timer0
    TIFR |= _BV(TOV0);                                    // clear timer0 overflow flag

		// wait for idle time between two packets, then do slave break
		//
    // wait for data line low for minimum of 7ms
		//
		while(bit_is_clear(TIFR, TOV0))
    {
        if (bit_is_clear(PINB, b_MOSI)) TCNT0 = c_7ms;	  // load Timer0 if MOSI is low == high on bus
    };
    // wait until timer0 overflow

    TIFR |= _BV(TOV0);                                     // clear timer0 overflow flag
    TCNT0 = c_2ms;		                                    // load Timer0

		//
    // wait for data line high for 2ms
		//
    while(bit_is_clear(TIFR, TOV0))
    {
        if (bit_is_set(PINB, b_MOSI)) TCNT0 = c_2ms;			// load Timer0  if MOSI is high == low on bus
    };
    // wait until timer overflow

		//
    // force data line 3ms low
		//
    TIFR |= _BV(TOV0);                                     // clear timer0 overflow flag
    TCNT0 = c_3ms;		                                     // load timer0 for 3ms
    DDRB |= _BV(b_MISO);                                   // MISO output and high (inverted by hardware to low)
    PORTB |= _BV(b_MISO);
    while(bit_is_clear(TIFR, TOV0));

    DDRB &=~ _BV(b_MISO);                                   // MISO input and data line high
    PORTB &=~ _BV(b_MISO);
    TCNT0 = c_Timer0_stop;		                            // stop timer0
    SPDR = 0x00, SPDR;	
    SPCR = c_RUN_SPI, SPCR;                               // enable SPI
		flags.timepoll_req = true;
}; // end of slavebreak	
