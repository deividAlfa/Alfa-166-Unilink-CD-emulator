"D:\WinAVR\bin\avrdude" -p m8 -c stk200 -P lpt1 -U flash:r:"D:\avrbecker_204\flash.hex":i -n -v >D:\avrbecker_204\flash.out 2>D:\avrbecker_204\flash.err
"D:\WinAVR\bin\avrdude" -p m8 -c stk200 -P lpt1 -U eeprom:r:"D:\avrbecker_204\eeprom.hex":i -n -v >D:\avrbecker_204\eeprom.out 2>D:\avrbecker_204\eeprom.err
"D:\WinAVR\bin\avrdude" -p m8 -c stk200 -P lpt1 -U lfuse:r:"D:\avrbecker_204\lfuse.hex":r -n -v >D:\avrbecker_204\lfuse.out 2>D:\avrbecker_204\lfuse.err
"D:\WinAVR\bin\avrdude" -p m8 -c stk200 -P lpt1 -U hfuse:r:"D:\avrbecker_204\hfuse.hex":r -n -v >D:\avrbecker_204\hfuse.out 2>D:\avrbecker_204\hfuse.err
