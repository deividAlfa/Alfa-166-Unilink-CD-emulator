# OLD PROJECT
Moved to: https://github.com/deividAlfa/Alfa-166-CD-Changer-Unilink-Emulator
<br><br><br><br><br><br><br><br>
## Alfa romeo 166 unilink CD changer emulator.

<!-- MarkdownTOC -->

* [Introduction](#intro)
* [Project description](#description)
* [Compiling](#compiling)
* [Debugging](#debugging)
* [Connections](#connections)

<!-- /MarkdownTOC -->

<a id="intro"></a>
## Introduction
First of all, this project is not supported in any way.<br>
I began to develop it, but the complexity and other circumstances caused it to be prematurely cancelled.<br>
Don't ask me to do "X" or "Y" thing.

If you want to learn how Unilink works, check the DOCUMENTATION folder.<br>
There is a lot of data. All I learned from is there.<br>

Most of it is actually gone from Internet, as the sites were very old. Some were dated back to 2001!<br>
I could recover some thanks to www.archive.org.<br>
The code is very valuable, there's almost no information about this on the net.<br>

I spend a lot of hours debugging the ICS communications to make it work well, so I uploaded it for anyone.<br>
Maybe someone with further knowledge could set up an MP3 decoder and finish it.

<a id="description"></a>
## Project description
This project enables communication with the ICS emulating the presence of the CD changer.<br>
It uses a STM32 "blackpill" board with STM32F411 running at 96MHz.<br>
The project is done in STM32 Cube IDE and uses ST's HAL Library.<br>
Part of the configuration is done in the .ioc file (CubeMX configuration).<br>

The serial unilink data is handled using interrupts, sampling the I/O lines.<br>
This is very inefficient, but I started this way for best stability during development.<br>
As I later stopped the project, I didn't bother to make a hardware SPI implementation.<br>
So, it needs a fast STM32. I tried with a 72MHz STM32F103 and I had issues due the lower speed.<br>
If you modify the code to use hardware SPI, the speed requirements will be much lower.<br>

It also includes the Tape emulator code from my other project: https://github.com/deividAlfa/Alfa166Bluetooth<br>
Both modes are working at the same time and don't cause conflicts.

The actual firmare handles almost all the unilink protocol used in the ICS.<br>
It also uses the USB OTG function. The CDs are stored as CD01 ... CD06 folders in the USB drive.<br>
It automatically scans these folders and its contents, and makes a listing for the ICS.<br>
So, for example, if CD01 has 45 files, it will tell to the ICS that CD01 is present and contains 45 tracks.<br>
It's able to change tracks, also to tell the ICS when the selected disc is not present, and the communication is pretty stable.<br>

However the audio implementation is not done, I dropped the project at this stage because it was too much for me.<br>
However, the ICS will enable the CD audio input, so you can hook an external audio source.

<a id="compiling"></a>
## Compiling

To compile:<br>
- Download STM32 Cube IDE.
- Clone or download the repository.
- Open STM32 Cube IDE, import existing project and select the folder where the code is.

It should recognize it and be ready for compiling or modifying for your own needs.<br>
Check "unilink.h" and "unilink.c". 
  
 
  
<a id="debugging"></a>
## Debugging

The firmware has different levels of debugging the Unilink data, see unilink.h.<br>
- "DebugSpi" enables a 4KB buffer that will store all SPI data as it was received or sent. Useful when debugging.
- "DebugLevel" will show internal Unilink protocol data. 0 is disabled, 1-3 enable the different verbose levels.
- "DebugLog" will display the unilink frames in a readable format. It has two additional modifiers:
	- "LogFormat" will split the data frames within brackets, so the data and checksums can be read easier.
	- "OnlyLog" will disable the slave interface and set the device in sniffer mode.<br>
	In this mode it can output the dialog between the ICS a CD changer.
	
 Example log outputs:<br>
 
	LogFormat=0:
	18 10 01 00 29
	18 10 01 02 2B
	10 30 8C 10 DC 24 A8 17 60 1F
	
	LogFormat=1:	
	[18 10 01 00][29]
	[18 10 01 02][2B]
	[10 30 8C 10][DC][24 A8 17 60][1F]

- In documentation, you have an Excel file that will parse this data, "Parse unilink data.xlsm". It already has data for the curious.<br>
  It uses VBA, so you need to enable macros. The function is called "ParseUnilink".<br>
  The result would be:
  
		01 MASTER REQUEST - BUS RESET		18 10 01 00 29 00
		01 MASTER REQUEST - ANYONE?		18 10 01 02 2B 00
		8C DEVICE INFO			10 30 8C 11 DD 14 A8 17 60 10 00
	  
		

The debugging data is sent using the serial port, and also throught the SWO pin.<br>
I used a logic analyzer to sniff the Unilink and serial data, and view everything in real time.<br>
Too see the data easier, I had to set up the serial in different speeds depending on "Debuglevel". You can change that in "serial.c".<br>
- Debuglevel=1: 9600
- Debuglevel=2: 38400
- Debuglevel=2: 57600 
 
<a id="connections"></a>
## Connections

The ICS connection is as follows:<br>
![IMAGE](https://github.com/deividAlfa/Alfa-166-Unilink-CD-emulator/blob/main/DOCUMENTATION/ICS_pinout.jpg)
![IMAGE](https://github.com/deividAlfa/Alfa-166-Unilink-CD-emulator/blob/main/DOCUMENTATION/ICS_pinout2.jpg)

  - All "terminal 30" pins = 12V power
  
  - Audio:
    - Audio gnd: ICS F18
    - Left input: ICS F19
    - Right input: ICS F20
    
  - Unilink interface:
    - Only Data and Clock are used. Reset and Enable are not implemented, not needed, it works perfectly.
    - Data: ICS F10, STM32 PC15
    - Clk: ICS F11, STM32 PC14

The stm32 pinout is as follows:<br>

![IMAGE](https://github.com/deividAlfa/Alfa-166-Unilink-CD-emulator/blob/main/DOCUMENTATION/stm32_pinout.jpg)


Some STM32F411 boards have an issue with USB OTG not working.<br>
This is caused by diode, not allowing the power to go from the board to the USB.<br>
The diode was removed on later revisions of the board. The fix is easy, just replace the diode with a jumper:

![IMAGE](https://github.com/deividAlfa/Alfa-166-Unilink-CD-emulator/blob/main/DOCUMENTATION/411_OTGFIX.jpg)
