/*************************************************************************
**  Becker Unilink Bus Interface (General Procedures)
**  by Michael Wolf
**
**  Released under GNU GENERAL PUBLIC LICENSE
**	See LICENSE.TXT for details
**
**  Revision History
**
**  when         what  who	why
**
**************************************************************************/

unsigned char bcd2hex(unsigned char bcd)
{
	unsigned char loc_bcd;
	loc_bcd = bcd >> 4;
	loc_bcd = (bcd & 0x0f) + 10 * loc_bcd;
    return(loc_bcd);  
}


unsigned char bin2ascii(unsigned char bin)
{
	bin = bin & 0x0f;													// mask lower nibble
    if (bin < 10)
		return bin + 48;                   			// if lower nibble is lower 10 make 0-9 ASCII char
    else
        return bin + 55;                   	// if lower nibble is greater 10 make A-F ASCII char
}


#ifdef BUS_LOGGING
void bus_logging(void)
{
        unsigned char loc_count = 0;        // local byte counter
        unsigned char loc_temp = 0;         // local temp
        unsigned char *loc_datapointer;    	// local pointer

        if (flags.unilink_tx_log)
        {
            flags.unilink_tx_log = false;  	// reset Tx flag
            loc_datapointer = unilink_txdata;// set local pointer to Tx data
            loc_count = unilink_txsize+1;  	// set transmit length
            usart_putc(0x0a);        				// LF
            usart_putc(0x0d);		        		// CR
            usart_putc(0x23);        				// start tag
        } else {
            usart_putc(0x0a);        				// LF
            usart_putc(0x0d);        				// CR
            loc_datapointer = unilink_rxdata;// set pointer to Rx data
            loc_count = unilink_rxsize-1;  	// set transmit length
            usart_putc(0xab);        				// start tag
        }
        /*
        This routine sends each byte of a packet in 2 ASCII chars via UART.
        Its a simple 8 bit hex to 2 byte ASCII conversion.
        example: 0xff is send as 0x4646 (FF)
        So you can monitor the output with a normal terminal program.
        */
        for (;loc_count > 0;loc_count--)           	// for a whole packet do:
        {
            loc_temp = *loc_datapointer;
			usart_putc(bin2ascii(loc_temp >> 4));   // send higher nibble of byte
			usart_putc(bin2ascii(loc_temp));		// send lower nibble of byte
            usart_putc(' ');                        // space between hex values
            *loc_datapointer++;
        }; // end for loop
        usart_putc('0');                            // send end byte
        usart_putc('0');                            // send end byte
}; // end UART_TX
#endif
