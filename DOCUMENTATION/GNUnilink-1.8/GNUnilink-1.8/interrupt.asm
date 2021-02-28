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
;   RXState is a bit field that holds a lot of information
;   X0000000 - 1=Reception is active
;   0X000000 - 1=ASCII, 0=Numerical
;   00X00000 - 1=Track name, 0=Disk name
;   000X0000 - 1=Hex number, 0=decimal number
;   0000XXXX - Counter for number of bytes to receive (-1)
;******************************************************************************


; Interupt Vector - used to process serial data

	IFDEF	SERIAL
	ORG	_IntVector
	movwf	w_store				; store w register
	swapf	STATUS,W
	bcf	STATUS,RP0
	movwf	status_store			; store status register

	IFDEF	SERIAL_INPUT
	btfsc	RCSTA, OERR 			; test for RX overflow
	goto	IntDecodeRXOverflow

	btfsc	PIR1, RCIF			; test if RX interrupt is set
	goto	IntDecodeRX
	ENDIF

	IFDEF	SERIAL_OUTPUT
	;assume interrupt must have come from TXIF, for now
IntGetNextTX:
	movlw	HIGH serial_command_table
	movwf	PCLATH

	call	serial_command_table		; Get character from table
	xorlw	0xFF
	btfss	STATUS, Z
	goto	IntGetNoReplace

	movf	SerialReplace, 0		; Replace character
	goto	IntGetReplaced

IntGetNoReplace:
	call	serial_command_table		; Get character from table
	addlw	0				; will set zero flag if w=0

	btfsc	STATUS,Z			; Was the byte zero ?
	goto	IntFinishedTX			; If zero, don't TX and disable

IntGetReplaced:
	movwf	TXREG				; TX the current character
	incf	command_offset,1		; now points to next byte
	goto	IntExit

IntFinishedTX:
	;all bytes transmitted
	bsf	STATUS, RP0			; Select BANK1
	bcf	PIE1,TXIE			; disable TX interrupt
	ENDIF

IntExit:
	bcf	STATUS, RP0
	swapf	status_store,W			; restore registers
	movwf	STATUS
	swapf	w_store,F
	swapf	w_store,W    
        
	RETFIE					; re enable interrupts
        
	IFDEF	SERIAL_INPUT
IntDecodeRXOverflow:
	bcf	RCSTA, CREN			; Clear overflow
	bsf	RCSTA, CREN

	movlw	0x00				; Abort reception
	movwf	RXState

	goto	IntExit
	
IntDecodeRX:					; Place decoding here....
	movf	RCREG, 0			; read RX'ed charater
	movwf	RXChar

	btfsc	RXState, 7
	goto	IntDecodeActive

	;movf	RXChar, 0
	xorlw	'd'
	btfsc	STATUS, Z
	goto	IntDecode_DiskNum

	movf	RXChar, 0
	xorlw	't'
	btfsc	STATUS, Z
	goto	IntDecode_TrackNum

	movf	RXChar, 0
	xorlw	'c'
	btfsc	STATUS, Z
	goto	IntDecode_Counter

	movf	RXChar, 0
	xorlw	'D'
	btfsc	STATUS, Z
	goto	IntDecode_DiskName

	IFDEF	TRACKNAMES
	movf	RXChar, 0
	xorlw	'T'
	btfsc	STATUS, Z
	goto	IntDecode_TrackName
	ENDIF

	movf	RXChar, 0
	xorlw	'p'
	btfsc	STATUS, Z
	goto	IntDecode_PageNum

	movlw	'?'				; unknown command
	movwf	TXREG

	goto	IntExit

IntDecode_DiskNum:
	movlw	0xD0
	movwf	RXState

	movlw	DiskNum
	movwf	RXPointer

	goto	IntExit

IntDecode_TrackNum:
	movlw	0xC1
	IFDEF	PIC_16
	btfss	OUT_PORT, CdNotMdBit		; unconnected = high = CD
	movlw	0xD1
	ENDIF
	movwf	RXState

	movlw	TrackNum
	movwf	RXPointer

	goto	IntExit

IntDecode_Counter:
	movlw	0xC3
	IFDEF	PIC_16
	btfss	OUT_PORT, CdNotMdBit		; unconnected = high = CD
	movlw	0xD3 				; mins in hex, secs in dec
	ENDIF
	movwf	RXState

	movlw	Time1
	movwf	RXPointer

	goto	IntExit

IntDecode_DiskName:
	IFDEF	LONGNAMES
	movlw	0x8F
	ELSE
	movlw	0x87
	ENDIF
	movwf	RXState

	movlw	Disk0
	movwf	RXPointer

	goto	IntExit

	IFDEF	TRACKNAMES
IntDecode_TrackName:
	IFDEF	LONGNAMES
	movlw	0xAF
	ELSE
	movlw	0xA7
	ENDIF
	movwf	RXState

	movlw	Track0
	movwf	RXPointer

	goto	IntExit
	ENDIF

IntDecode_PageNum:
	movlw	0xD0
	movwf	RXState

	movlw	DispCtrl
	movwf	RXPointer

	goto	IntExit

IntDecodeActive:
	movf	RXPointer, 0			; set up Indirect addressing
	movwf	FSR
	bcf	STATUS, IRP

	btfsc	RXState, 6
	goto	IntDecodeDigit

	movf	RXChar, 0			; Copy ASCII char to pointer
	movwf	INDF

	incf	RXPointer, 1			; increase pointer

	goto	IntDecodeTestDone

IntDecodeDigit:
	movlw	0x30
	subwf	RXChar, 1			; subtract and store in RXChar

	btfss	STATUS, C			; RXChar < 0x30
	goto	IntDecodeDigitBlank

	movf	RXChar, 0
	sublw	0x09

	btfsc	STATUS, C			; RXChar > 0x39
	goto	IntDecodeDigitWhich

	btfss	RXState, 4			; Decimal or Hex?
	goto	IntDecodeDigitBlank

IntDecodeHex:
	movf	RXChar, 0
	sublw	0x36

	btfss	STATUS, C			; RXChar > 0x66
	goto	IntDecodeDigitBlank

	movf	RXChar, 0
	sublw	0x30

	btfsc	STATUS, C			; RXChar < 0x61
	goto	IntDecodeDigitBlank

	movlw	0x27
	subwf	RXChar, 1			; subtract and store in RXChar

	goto	IntDecodeDigitWhich

IntDecodeDigitBlank:
	movlw	0x0f				; set to ' ' (blank)
	IFDEF	PIC_16
	btfss	OUT_PORT, CdNotMdBit		; unconnected = high = CD
	movlw	0x00				; set to '0'
	ENDIF
	movwf	RXChar
	
IntDecodeDigitWhich:
	btfsc	RXState, 0
	goto	IntDecodeDigitUpper

	movlw	0xf0				; ensure lower nyble is 0
	andwf	INDF, 1

	movf	RXChar, 0			; 'xor' in lower nyble with
	xorwf	INDF, 1				;  pointer contents
	
	incf	RXPointer, 1			; increase pointer

	goto	IntDecodeTestDone

IntDecodeDigitUpper:
	swapf	RXChar, 0			; Copy to upper nyble
	movwf	INDF
	
	;goto	IntDecodeTestDone

IntDecodeTestDone:
	movf	RXState, 0			; test if the last character
	andlw	0x0F
	btfsc	STATUS, Z

	goto	IntDecodeDone

	IFDEF	PIC_16
	btfsc	OUT_PORT, CdNotMdBit		; unconnected = high = CD
	goto	IntDecodeTestDone2
	movf	RXState, 0			; half-arsed system....
	xorlw	0xD2				; mins in hex, secs in dec
	btfsc	STATUS, Z
	bcf	RXState, 4			; switch to decimal
	ENDIF

IntDecodeTestDone2:
	decf	RXState, 1
	goto	IntExit

IntDecodeDone:
	bcf	RXState, 7

	movf	Display, 0			; test if Display !=0
	andlw	0x7f
	xorlw	0x00

	btfss	STATUS, Z
	goto	IntExit

	movlw	0x14				; update display counter,
	btfsc	RXState, 6			; disk & track numbers
	goto	IntDecodeDone2

	movlw	0x12				; update Disk name
	btfss	RXState, 5
	goto	IntDecodeDone2

	movlw	0x10				; update Track name

IntDecodeDone2:
	movwf	Display
	goto	IntExit

	ENDIF

	ENDIF
