## Alfa romeo 166 unilink CD changer emulator.

<!-- MarkdownTOC -->

* [Introduction](#intro)
* [Project description](#description)
* [Compiling](#compiling)
* [Connections](#connections)

<!-- /MarkdownTOC -->

<a id="intro"></a>
## Introduction
First of all, this project is not supported in any way.<br>
I began developing it, but stopped prematurely before finishing. So don't ask me to do "X" or "Y" thing.

If you want to learn how Unilink works, check the DOCUMENTATION folder. There is a lot of data there.<br>
Most of it is gone from Internet, as the sites were very old. Some were dated back to 2001!<br>
I could recover some thanks to www.archive.org.<br>
The code is very valuable, there's almost no information about this on the net.<br>

I spend a lot of hours debugging the ICS communications to make it work well, so I uploaded it for anyone.<br>
Maybe someone with further knowledge could set up an MP3 decoder and finish it.

<a id="description"></a>
## Project description
This project enables communication with the ICS emulating the presence of the CD changer.<br>
It uses a STM32 "blackpill" board with STM32F411 running at 96MHz.<br>
The project is done in STM32 Cube IDE and uses ST's HAL Library. The pinouts can be seen opening the .ioc file (CubeMX configuration).<br>

The serial unilink data is handled using interrupts, sampling the I/O lines.<br>
This is very inefficient, but I started this way for best stability during development.<br>
As I latter stopped the project, I didn't bother to make a hardware SPI implementation.<br>
So, it needs a fast STM32. I tried with a 72MHz STM32F103 and I had issues due the lower speed.<br>
If you modify the code to use hardware SPI, the speed requirements will be much lower.<br>

It also includes the Tape emulator code from my other project: https://github.com/deividAlfa/Alfa166Bluetooth<br>
Both modes are working at the same time.

The actual firmare handles almost all the unilink protocol used in the ICS.<br>
It also uses the USB OTG function. The CDs are stored as CD01 ... CD06 folders in the USB drive.<br>
It automatically scans these folders and its contents, and makes a listing for the ICS.<br>
So, for example, if CD01 has 45 files, it will tell to the ICS that CD01 is present and contains 45 tracks.<br>
It's able to change tracks, also to tell the ICS when the selected disc is not present, and the communication is pretty stable.<br>

However the audio implementation is not done, I dropped the project at this stage because it was too much for me.<br>
At this stage it can only work as aux input. The ICS will enable the CD audio input, you can hook an external audio source.

<a id="compiling"></a>
## Compiling

- Download STM32 Cube IDE<br>
- Clone or download the repository<br>
- Open STM32 Cube IDE, import existing project and select the folder where the code is.<br>

  It should recognize it and be ready for compiling or modifying for your own needs.<br>  
  Check "unilink.h" and "unilink.c". 

<a id="connections"></a>
## Connections

The ICS connection is as follows:<br>
![IMAGE](https://github.com/deividAlfa/Alfa-166-Unilink-CD-emulator/blob/main/DOCUMENTATION/ICS_pinout.jpg) ![IMAGE](https://github.com/deividAlfa/Alfa-166-Unilink-CD-emulator/blob/main/DOCUMENTATION/ICS_pinout2.jpg)

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
