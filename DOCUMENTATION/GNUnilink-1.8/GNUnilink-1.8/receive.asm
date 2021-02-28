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

; Main code loop - detects a 'break' and then waits for a 'packet'. Once 
; received this is interpreted and if one of the commands that we act on, the 
; PIC responds with a suitable reply.

ReceiveBreak:
	movlw	40				; Value for 4ms (40 * 100us)
	movwf	Count

ReceiveLoop1:
	movlw	delay100us			; Value for 100us 
	movwf	Count2

ReceiveLoop2:
	btfsc	OUT_PORT, ClockBit		; Test for Clock being low
	goto	ReceiveBreak			; restart break detection

	btfss	OUT_PORT, BusOnBit		; Test for BusOn being high
	;goto	SystemSetup			; force reset(do we want this?)
	goto	WarmBoot

	decfsz	Count2, 1			; Reduce Count2
	goto	ReceiveLoop2

	decfsz	Count, 1			; Reduce Count 
	goto	ReceiveLoop1

ReceiveCommand:
	clrf	Checksum			; Clear checksum

	call	ReadByte			; Get RAD
	movwf	Rad
	addwf	Checksum, 1

	btfsc	STATUS, Z			; Test for a Zero RAD
	goto	ReceiveBreak			; as you're out of sync.

	call	ReadByte			; Get TAD
	movwf	Tad
	addwf	Checksum, 1

	call	ReadByte			; Get CMD1
	movwf	Cmd1
	addwf	Checksum, 1

	call	ReadByte			; Get CMD2
	movwf	Cmd2
	addwf	Checksum, 1

ReceiveParity1:
	call	ReadByte
	movwf	Parity1
	subwf	Checksum, 0			; Test checksum

	btfss	STATUS, Z
	goto	ReceiveBreak			; Checksum was wrong!!

	btfss	Cmd1, 7				; Test for 'Short' commands
	goto	ReceiveEnd
	
	IF	0				; Ignore Medium Commands to save
	goto	ReceiveBreak			; space in code....
	ELSE
RecieveMedium:
	call	ReadByte
	movwf	Data11
	addwf	Checksum, 1

	call	ReadByte
	movwf	Data12
	addwf	Checksum, 1

	call	ReadByte
	movwf	Data13
	addwf	Checksum, 1

	call	ReadByte
	movwf	Data14
	addwf	Checksum, 1

	btfss	Cmd1, 6				; Test for 'Medium' commands
	goto	ReceiveParity2

	IF	0				; Ignore Long Commands to save
	goto	ReceiveBreak			; space in code....
	ELSE
RecieveLong:
	call	ReadByte
	movwf	Data21
	addwf	Checksum, 1

	call	ReadByte
	movwf	Data22
	addwf	Checksum, 1

	call	ReadByte
	movwf	Data23
	addwf	Checksum, 1

	call	ReadByte
	movwf	Data24
	addwf	Checksum, 1

	call	ReadByte
	movwf	Data25
	addwf	Checksum, 1
	ENDIF

ReceiveParity2:
	call	ReadByte
	movwf	Parity2
	subwf	Checksum, 0			; Test checksum

	btfss	STATUS, Z
	goto	ReceiveBreak			; Checksum was wrong!!
	ENDIF

ReceiveEnd:
	call	ReadByte
	movwf	EndByte

	btfss	STATUS, Z
	goto	ReceiveBreak			; EndByte was non-zero!!

	goto	Interpret
