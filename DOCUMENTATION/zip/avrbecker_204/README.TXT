
  All files and the complete source code is released
  under GNU GENERAL PUBLIC LICENSE
	
  Last edit: 22/08/04

  For updates and informations about the project and Sony/Becker Unilink
  check my page: www.mictronics.de
  or contact me: michael@mictronics.de

  Bug reports, comments, questions or requests are welcome.

	*** Op-amp hardware ***
	
	Enable the OPAMP_HW option in config.h if you build the v3 hardware.


  Display update commands:  

  *** Time and track info ***

  tdttmmss
  ||| | |
  ||| | ss = seconds, range 0-99
  ||| mm = minutes, CD and MD mode range: 0-99
  ||tt = track number, CD mode range 0-99 ; MD mode range: 0-99
  |d = disc number, but +1, ie 9 is disc 10, range 0-9
  t = command id for time update 


  *** Disc name ***

  Dxxx...xxxxx~
  ||         ||
  ||         |End tag
  |ASCII string
  D = command id for disc name update

  Length of ASCII string is 0-8 characters.
  This means you can display a disc name with up to 8 characters.
	
  If your name is shorter than 8 character you must send an ~ as
  last character!

  ie: DMyCDNAME~


  *** Raw Commands ***

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


  ### RS232 output ###

  This source code version produces RS232 output
  dependent wich key is pressed on your head unit.

  You can configure the output in file config.h

  RS232 output support for following head unit keys
  or modes changes:

  Play, Stop, Track_up, Track_down, Disc_up, Disc_down,
  FFWD, FRVS, Repeat, Shuffle, Power_Off


  ### Additional features for ATmega8 schematic ###

  LED1 - ID status; ON if interface has ID else OFF (D4)
  LED2 - Play status; ON if interface is selected as source
         and in play mode else OFF (D3)
  LED3 - Unilink Rx/Tx traffic

  Relais1 - Turns ON if interface is selected as source
            AND in play mode, turns NOT OFF when
            another source is selected.
  The relais will turn OFF only on "power off"
  commands from head unit NOT on "Source" change.
