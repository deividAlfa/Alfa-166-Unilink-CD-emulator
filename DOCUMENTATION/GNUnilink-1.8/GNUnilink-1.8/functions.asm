;******************************************************************************
;
;    This program is free software; you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation; either version 2 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program; if not, write to the Free Software
;    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;
;    I (Simon Wood) can be contacted via email - unilink@mungewell.org
;    or via post - 14 Kirklands Close, Baildon, West Yorkshire, England.
;
;******************************************************************************
; Functions for GNUnilink - these have to be placed at start of code as the 
; PIC12 series can only 'Call' to lower addresses

;******************************************************************************
; Lookup table for serial output - defines the output strings.
; Must not cross a page boundary!

        IFDEF SERIAL_OUTPUT
 
serial_command_table:
	movf	command_offset,0		; Move command offset to W
	addwf	PCL,F				; Add offset to program counter

command_start:
	retlw	'P'
	retlw	'L'
	retlw	'A'
	retlw	'Y'
	retlw	'\n'
	retlw	0

command_pause:
	retlw	'P'
	retlw	'A'
	retlw	'U'
	retlw	'S'
	retlw	'E'
	retlw	'\n'
	retlw	0

command_stop:
	retlw	'S'
	retlw	'T'
	retlw	'O'
	retlw	'P'
	retlw	'\n'
	retlw	0

command_next_track:
	retlw	'N'
	retlw	'E'
	retlw	'X'
	retlw	'T'
	retlw	'\n'
	retlw	0

command_previous_track:
	retlw	'P'
	retlw	'R'
	retlw	'E'
	retlw	'V'
	retlw	'I'
	retlw	'O'
	retlw	'U'
	retlw	'S'
	retlw	'\n'
	retlw	0

	IFNDEF	DISKDIRECT
command_next_disk:
	retlw	'N'
	retlw	'X'
	retlw	'T'
	retlw	'_'
	retlw	'L'
	retlw	'I'
	retlw	'S'
	retlw	'T'
	retlw	'\n'
	retlw	0

command_previous_disk:
	retlw	'P'
	retlw	'R'
	retlw	'V'
	retlw	'_'
	retlw	'L'
	retlw	'I'
	retlw	'S'
	retlw	'T'
	retlw	'\n'
	retlw	0
	ELSE
command_direct_disk:
	retlw	'L'
	retlw	'I'
	retlw	'S'
	retlw	'T'
	retlw	'_'
	retlw	255			; replace this character
	retlw	'\n'
	retlw	0
	ENDIF

	IFDEF	TRACKMODES
command_intro:
	retlw	'I'
	retlw	'N'
	retlw	'T'
	retlw	'R'
	retlw	'O'
	retlw	'\n'
	retlw	0

command_shuffle:
	retlw	'S'
	retlw	'H'
	retlw	'U'
	retlw	'F'
	retlw	'F'
	retlw	'\n'
	retlw	0

command_repeat:
	retlw	'R'
	retlw	'E'
	retlw	'P'
	retlw	'E'
	retlw	'A'
	retlw	'T'
	retlw	'\n'
	retlw	0
	ENDIF

	ENDIF

;******************************************************************************
; Send a Slave Break - tells the Master that you have something to tell them...

; In this function GNUnilink looks for a period when the data bus is low for 
; more than 6ms and then waits for it to be high for 2ms. Once this is seen it 
; knows were in the middle of an unused 'slave phase' and it then forces the 
; bus low for 4ms.

; NOTE - whilst GNUnilink is in this state it is unable to receive other 
; commands from the master

IssueSlaveBreak:
	movlw	60				; 6ms (60 * 100us)
	movwf	Count

BreakLoop1:
	movlw	delay100us			; 100us 
	movwf	Count2

BreakLoop2:
	btfsc	OUT_PORT, DataBit		; Data low ('Master phase')
	goto	IssueSlaveBreak

	goto	BreakLoop3			; NOP
BreakLoop3:

	decfsz	Count2, 1			; Reduce Count2
	goto	BreakLoop2

	decfsz	Count, 1			; Reduce count 
	goto	BreakLoop1

	IFDEF	SERIAL
	bcf	INTCON, GIE
	ENDIF

BreakLoop4:
	IFDEF	NASTY_HACK
	movlw	40				; rely on timing. 
	ELSE
	movlw	20				; 2ms (20 * 100us)
	ENDIF
	movwf	Count

BreakLoop5:
	movlw	delay100us
	movwf	Count2

BreakLoop6:
	IFNDEF	NASTY_HACK
	btfss	OUT_PORT, DataBit		; Data high for 2ms 
	goto	BreakLoop4			; (moving into 'Slave Phase')
	ENDIF

	goto	BreakLoop7			; NOP
BreakLoop7:

	decfsz	Count2, 1			; Reduce Count2
	goto	BreakLoop6

	decfsz	Count, 1			; Reduce count 
	goto	BreakLoop5

	IFDEF PIC_12
	movlw	0xfe				; Enable data pin
	tris	0
	ENDIF

	IFDEF PIC_16
	bsf	STATUS, RP0			; Enable data pin as output
	bcf	OUT_ENA, DataBit
	bcf	STATUS, RP0
	ENDIF

	bcf	OUT_PORT, DataBit		; Force Data low.... 4ms

	movlw	40
	movwf	Count

BreakLoop9:
	movlw	delay100us
	movwf	Count2

BreakLoop10:
	goto	BreakLoop11			; 2 * NOP
BreakLoop11:
	goto	BreakLoop12			; 2 * NOP
BreakLoop12:

	decfsz	Count2, 1
	goto	BreakLoop10

	decfsz	Count, 1
	goto	BreakLoop9

	IFDEF PIC_12
	movlw	0xff				; Tristate data pin
	tris	0
	ENDIF

	IFDEF PIC_16
	bsf	STATUS, RP0			; Tristate data pin
	bsf	OUT_ENA, DataBit
	bcf	STATUS, RP0
	ENDIF

	IFDEF	SERIAL
	bsf	INTCON, GIE
	ENDIF

	return

;******************************************************************************
; Receive a byte - received byte is placed in W register.

; Input is sampled between 1 cycle before and 3 cycles after Clock rising edge,
; depending on the asynchronous alignment with the Master

ReadByte:
	clrf	DataVal

	movlw	0x08
	movwf	Count

	bcf	STATUS, C			; Ensure carry bit is 0

ReadLoop1:
	rlf	DataVal, 1
	
ReadLoop2:
	btfss	OUT_PORT, ClockBit		; Wait for clock going high
	goto	ReadLoop2

ReadLoop3:
	IFDEF	SERIAL
	bcf	INTCON, GIE
	ENDIF

	movf	OUT_PORT, 0			; Sample input

	btfsc	OUT_PORT, ClockBit		; Wait for clock going low
	goto	ReadLoop3

	movwf	Count2				; (inverted) input
	btfss	Count2, DataBit
	bsf	DataVal, 0
	
	decfsz	Count, 1			; Do next bit, if any left...
	goto	ReadLoop1

	IFDEF	SERIAL
	bsf	INTCON, GIE
	ENDIF

	movf	DataVal, 0
	return

;******************************************************************************
; Send a byte - byte to ouput is in W register.

; This is the piece of code which is the most time critcal.
; Worst case the outpit bit is set a maximum of 7 cycles after Clock goes High
; and held until the Clock goes high again

; this hopefully gives enough time for the master to register it.... a LOT 
; dodgy on slow systems.

; We may have to reverse the logic on this so that the output is set before the 
; Clock goes high and changed after the Clock goes low. This will improve the i
; set up, but reduce the hold time.

SendByte:
	movwf	DataVal				; Store byte to output

	addwf	Checksum, 1			; Compute Checksum

	IFDEF PIC_12
	movlw	0xfe				; Enable data pin
	tris	0
	ENDIF

	IFDEF PIC_16
	bsf	STATUS, RP0			; Enable data pin
	bcf	OUT_ENA, DataBit
	bcf	STATUS, RP0
	ENDIF

	movlw	0x08
	movwf	Count

SendLoop1:
	bcf	OUT_PORT, DataBit		; remember it's INVERTED

SendLoop2:
	btfss	OUT_PORT, ClockBit		; Wait for clock going high
	goto	SendLoop2

	IFDEF	SERIAL
	bcf	INTCON, GIE
	ENDIF
	
	btfss	DataVal, 7
	bsf	OUT_PORT, DataBit

	rlf	DataVal, 1

SendLoop3:
	btfsc	OUT_PORT, ClockBit		; Wait for clock going low
	goto	SendLoop3

	decfsz	Count, 1			; Do next bit, if any left...
	goto	SendLoop1

	goto	SendLoop4			; just a little data hold
SendLoop4:

	IFDEF PIC_12
	movlw	0xff				; Tristate data pin
	tris	0
	ENDIF

	IFDEF PIC_16
	bsf	STATUS, RP0			; Trisate data pin
	bsf	OUT_ENA, DataBit
	bcf	STATUS, RP0
	ENDIF
	
	IFDEF	SERIAL
	bsf	INTCON, GIE
	ENDIF

	return

;******************************************************************************
; Bit frig - works out which bit to set in the response to Master Poll

; W register is input of which stage you are on (0x00, 0x20, 0x30, 0x40 etc)
; and is returned with the byte to write (0x00 if wrong stage).

Bit_Frig:
	xorwf	MyBit, 0
	andlw	0xe0				; Strip off low bits

	btfsc	STATUS, Z			; Do we have a hit?
	goto	Bit_Frig_Hit

	movlw	0x00
	return

Bit_Frig_Hit:
	btfss	MyBit, 4			; Do we need to swap nybbles?
	goto	Bit_Frig_Swap

	movf	MyBit, 0
	andlw	0x0F
	return

Bit_Frig_Swap:
	swapf	MyBit, 0
	andlw	0xF0
	return
