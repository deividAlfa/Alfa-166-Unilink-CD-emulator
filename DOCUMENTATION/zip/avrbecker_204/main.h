/*************************************************************************
**  Becker Unilink Interface (Main Header File)
**  by Michael Wolf
**
**  Released under GNU GENERAL PUBLIC LICENSE
**	See LICENSE.TXT for details
**
**  Revision History
**
**  when			what  who	why
**	19/03/04	v1.00	MIC	Initial Release
**	22/08/04	v1.01	MIC + added SPI config byte for new op-amp hardware
**
**************************************************************************/

// Check if target is supported
#if !defined(__AVR_AT90S8515__) && !defined(__AVR_ATmega8515__) && !defined(__AVR_ATmega8__)
	#error Target not supported!
#endif

// Check if MCU frequency is supported
#if MCU_XTAL < 6000000
	#warning MCU clock frequency to low!
#elif MCU_XTAL > 16000000
	#warning MCU clock frequency to high!
#elif (MCU_XTAL > 8000000) && defined(__AVR_AT90S8515__)
	#warning MCU clock frequency to high for AT90S8515!
#endif

// Set correct timer prescaler dependent on MCU frequency
#if (MCU_XTAL >= 1000000) && (MCU_XTAL <= 2000000)
	#define PRESCALER 64
	#define c_Timer0_run 3              // run Timer0 with clk/64
	#define c_Timer1_run 3              // run Timer1 with clk/64
#elif (MCU_XTAL > 2000000) && (MCU_XTAL <= 8000000)
	#define PRESCALER 256
	#define c_Timer0_run 4              // run Timer0 with clk/256
	#define c_Timer1_run 4              // run Timer1 with clk/256
#elif (MCU_XTAL > 8000000) && (MCU_XTAL <= 16000000)
	#define PRESCALER 1024
	#define c_Timer0_run 5              // run Timer0 with clk/1024
	#define c_Timer1_run 5              // run Timer1 with clk/1024	
#else
	#error MCU clock frequency not supported by Timer0 or Timer1!
#endif
	
// Check for maximum Baudrate
#if BAUD_RATE > 115200
	#error Selected Baudrate to high!
#endif

// Check for low Baudrate and Bus logging
#if defined(BUS_LOGGING) && (BAUD_RATE < 38400)
	#error Baudrate to low for Bus logging!
#endif

// disable LED's and relais if target is not ATmega8
#if !defined(__AVR_ATmega8__)
	#undef LED_OUT				
	#undef RELAIS_OUT
#endif 

// disable Yampp3 control if bus logging is enabled and power control via interface is disabled
#if defined(BUS_LOGGING) || !defined(RELAIS_OUT)
	#undef YAMPP3
#endif

#define UART_BAUDRATE   ((unsigned long)((unsigned long)MCU_XTAL/(BAUD_RATE*16)-1))	// calculate baud rate value for UBBR
#define RX_BUFFER_SIZE      50      // size of Rx buffer
#ifdef OPAMP_HW
	#define c_RUN_SPI					0xcc    // SPI config byte
#else
	#define c_RUN_SPI					0xc4		// SPI config byte
#endif
#define c_Timer0_stop 			0       // stop Timer0
#define c_Timer1_stop 			0       // stop Timer1

// Calculate delay (256 - TIME_ms * (CLOCK / PRESCALER) / 1000UL)
#define c_2ms ((unsigned char)(256 - 2 * (MCU_XTAL / PRESCALER) / 1000UL))
#define c_3ms ((unsigned char)(256 - 3 * (MCU_XTAL / PRESCALER) / 1000UL))
#define c_4ms ((unsigned char)(256 - 4 * (MCU_XTAL / PRESCALER) / 1000UL))
#define c_6ms ((unsigned char)(256 - 6 * (MCU_XTAL / PRESCALER) / 1000UL))
#define c_7ms ((unsigned char)(256 - 7 * (MCU_XTAL / PRESCALER) / 1000UL))
#define c_8ms ((unsigned char)(256 - 8 * (MCU_XTAL / PRESCALER) / 1000UL))
#define c_delay  ((unsigned int)(256 - 500 * (MCU_XTAL / PRESCALER) / 1000UL))

#if defined(__AVR_ATmega8__)
// Pin definition for ATmega8
#define b_SS        2               // Pin SS
#define b_MOSI      3               // Pin MOSI
#define b_MISO      4               // Pin MISO
#define b_CLK       5               // Pin CLK
#define b_JUMPER    0               // Pin CD/MD mode jumper

#define d_RELAIS	7									// Power relais output
#define d_LED1		2									// LED 1
#define d_LED2		3									// LED 2
#define d_LED3		4									// LED 3

#else
// Pin definition for AT90S8515 and ATmega8515
#define b_SS        2               // Pin SS
#define b_MOSI      5               // Pin MOSI
#define b_MISO      6               // Pin MISO
#define b_CLK       7               // Pin CLK
#define b_JUMPER    0               // Pin CD/MD mode jumper

#endif

#define true		1
#define false		0

// byte in packet array locations
#define destaddr    0               // destination address
#define srcaddr     1               // source address
#define cmd1        2               // command 1
#define cmd2        3               // command 2
#define parity1     4               // parity 1
#define data        5               // start of data
#define d1					5								// data byte 1
#define d2					6								// data byte 2
#define d3					7								// data byte 3
#define d4					8								// data byte 4
#define d5					9								// data byte 5
#define d6					10							// data byte 6
#define d7					11							// data byte 7
#define d8					12							// data byte 8
#define d9					13							// data byte 9

//*** Global variables ***
volatile struct {                   // use a bit field as flag store
unsigned char checksum_ok :			1 ;	// set if Unilink packet is valid
unsigned char spi_tx:						1 ; // set when SPI is in Tx mode
unsigned char unilink_rx_compl :	1 ; // set when Unilink packet is complete
unsigned char unilink_tx_log :		1 ; // set if Unilink packet is transmitted, used in logging routine
unsigned char timepoll_req :			1 ; // indicates a requested time poll
unsigned char timer1_ovf :				1 ;	// timer 1 overflow occured
unsigned char bus_sleep :				1 ; // set if bus is in sleep mode
unsigned char unilink_anyone :		1 ; // set on respond to anyone
} flags;                            	// declare as flag byte

volatile struct {
unsigned char discname :			 		1 ;	// set if discname update
unsigned char time :         		1 ;	// set if time info update
unsigned char magazine :					1 ; // set if magazine info is requested
unsigned char disc : 						1 ; // set if disc total time is requested
unsigned char raw : 							1 ; // set if RAW command
unsigned char mode : 							1 ; // set if RAW command

} unilink_command;


volatile struct {
unsigned char rx: 5;			    // counter for received bytes
unsigned char tx: 5;			    // counter for transmitted bytes
} unilink_bytecount;

unsigned char unilink_ownaddr = 0;            // holds the actual ID
unsigned char unilink_groupid = 0;						// holds current group ID
unsigned char unilink_rxsize =  0;            // receive packet size in bytes
unsigned char unilink_txsize =  0;            // transmit packet size in bytes
unsigned char unilink_timeinfo[8] = {0x70, 0x00, 0x90, 0x50, 0x01, 0x00, 0x00, 0x1E}; // init timeinfo string; Disc 1 Track 1 0:00
unsigned char unilink_status = 0x80;          // initial interface status (Idle)
unsigned char unilink_rxdata[16];             // holds the received packet
unsigned char unilink_txdata[16];             // holds the transmit packet
unsigned char unilink_bitid = 0;							// holds ID for poll request
unsigned char unilink_discname[8];	          // holds disc name

unsigned char tracknbr=1;

// needed to detect disc and track change
unsigned char unilink_lastdisc=0;								// last disc reminder
unsigned char unilink_lasttrack;								// last track reminder

unsigned char usart_rx_buffer[RX_BUFFER_SIZE+1];// UART Rx buffer
unsigned char usart_rx_index = 0;		          // index of next char to be put into the buffer

// memory for RAW commands
unsigned char raw[13] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

//*** Function prototype declaration ***
void usart_init(void);             		// init UART
void reset_spi(void);               		// reset SPI after interrupt
void checksum_check(void); 						// does a checksum verification on received packet
void bus_logging(void);             		// send valid received packed via UART
void usart_putc(unsigned char byte);	// send one byte via UART
void unilink_tx(unsigned char *msg);	// send unilink packet
void usart_rx_proc(void);           		// process received UART message
void slavebreak(void);              		// do a slave break, force request poll
void unilink_parse(void);	        		// Unilink command evaluation
void unilink_broadcast(void);       		// do broadcast commands
void unilink_myid_cmd(void);        		// do commands for my actual ID
void unilink_appoint(void);	        	// do appoint respond
void repeat_mode(void);             		// do play mode change
void shuffle_mode(void);            		// do shuffle mode change
void send_command(void);         			// do a display update if necessary
unsigned char bcd2hex(unsigned char bcd); // convert 1 byte BCD to 1 byte HEX
unsigned char bin2ascii(unsigned char bin); // convert lower nibble of 1 byte binary to ASCII

