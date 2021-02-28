;;******************************************************************************
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
; Sequenced reponses to the Slave Poll, called so that the screen is updated.

Respond_SlavePoll:
	movf	MyId, 0				; Check it's the right ID
	subwf	Rad, 0
	btfss	STATUS, Z
	goto	ReceiveBreak			; Done, wait for next command

	clrf	Checksum 			; Clear checksum

	incf	Display, 1			; Increment Display Count,
						; remember you're "+1"

	movf	Display, 0
	xorlw	0x02				; Check if 1st repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_1

	movf	Display, 0
	xorlw	0x03				; Check if 2nd repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_2

	movf	Display, 0
	xorlw	0x04				; Check if 3rd repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_3

	IFDEF	TRACKNAMES
	movf	Display, 0
	xorlw	0x05				; Check if 4th repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_4

	IFDEF	LONGNAMES
	movf	Display, 0
	xorlw	0x06				; Check if 5th repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_5
	ENDIF
	ENDIF	;TRACKNAMES

	movf	Display, 0
	xorlw	0x07				; Check if 6th repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_6

	IFDEF	LONGNAMES
	movf	Display, 0
	xorlw	0x08				; Check if 7th repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_7
	ENDIF

	IFDEF	SERIAL_INPUT
	IFDEF	TRACKNAMES
	movf	Display, 0
	xorlw	0x11				; Check if 0x10th repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_10

	IFDEF	LONGNAMES
	movf	Display, 0
	xorlw	0x12				; Check if 0x11th repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_11
	ENDIF
	ENDIF	;TRACKNAMES

	movf	Display, 0
	xorlw	0x13				; Check if 0x12th repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_12

	IFDEF	LONGNAMES
	movf	Display, 0
	xorlw	0x14				; Check if 0x13th repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_13
	ENDIF

	movf	Display, 0
	xorlw	0x15				; Check if 0x14th repsonse
	btfsc	STATUS, Z
	goto	Respond_SlavePoll_14

	ENDIF	;SERIAL_INPUT

	goto	ReceiveBreak			; should NEVER get here!!

;--------

Respond_SlavePoll_1:
	movlw	0x77				; 'Seeking CD'
	call	SendByte

	movf	MyId, 0
	call	SendByte

	movlw	0xc0
	call	SendByte

	movlw	0x40
	call	SendByte

	movf	Checksum, 0
	movwf	Count2

	movf	Checksum, 0
	call	SendByte

	movf	Count2, 0
	movwf	Checksum
	
	movlw	0x00
	call	SendByte

	movlw	0x00
	call	SendByte

	movlw	0x00
	call	SendByte

	movlw	0x00
	call	SendByte
	
	movlw	0x00
	call	SendByte
	
	movf	TrackNum, 0			; Track Number
	call	SendByte

	movlw	0x00
	call	SendByte

	movlw	0x00
	call	SendByte

	swapf	DiskNum, 0			; Disk Number + disp code
	iorlw	0x01
	call	SendByte
	
	goto	Slave_Respond_End

;--------

Respond_SlavePoll_2:
	movlw	0x70				; 'Seeking to Track'
	call	SendByte

	movf	MyId, 0
	call	SendByte

	movlw	0xc0
	call	SendByte

	movlw	0x00
	call	SendByte

	movf	Checksum, 0
	movwf	Count2

	movf	Checksum, 0
	call	SendByte

	movf	Count2, 0
	movwf	Checksum
	
	movlw	0x00
	call	SendByte

	movlw	0x00
	call	SendByte

	movlw	0x00
	call	SendByte

	movlw	0x00
	call	SendByte
	
	movlw	0x00
	call	SendByte
	
	movf	TrackNum, 0			; Track Number
	call	SendByte

	movlw	0x00
	call	SendByte

	movlw	0x00
	call	SendByte

	swapf	DiskNum, 0			; Disk Number + disp code
	iorlw	0x0d
	call	SendByte
	
	;IFNDEF	TRACKNAMES
	;IFDEF	PIC_16
	;btfss	OUT_PORT, CdNotMdBit		; unconnected = high = CD
	;goto	Slave_Respond_End
	;ENDIF
	;ENDIF

	IFNDEF	TRACKNAMES
	movlw	0x07				; cause Display to skip
	movwf	Display				; ahead to DiskName
	ENDIF

	goto	Slave_Respond_End

;--------
	
Respond_SlavePoll_14:
Respond_SlavePoll_3:
	movlw	0x70				; "CD d, Track t, mm:ss"
	call	SendByte

	movf	MyId, 0
	call	SendByte

	movlw	0x90
	call	SendByte

	IFDEF	TRACKNAMES
	movlw	0x50
	ELSE
	movlw	0x00				;what's this??
	ENDIF
	call	SendByte

	movf	Checksum, 0
	movwf	Count2

	movf	Checksum, 0
	call	SendByte

	movf	Count2, 0
	movwf	Checksum
	
	movf	TrackNum, 0			; Track Number
	call	SendByte

	movf	Time1, 0			; Track Timer
	call	SendByte

	movf	Time0, 0			; Track Timer
	call	SendByte

	swapf	DiskNum, 0			; Disk Number + Disp code
	iorwf	DispCtrl, 0
	call	SendByte

	goto	Slave_Respond_End

;----

	IFDEF	TRACKNAMES
Respond_SlavePoll_10:
	IFNDEF	LONGNAMES
	movlw	0x7f				; to stop slave polling
	movwf	Display				; will be inc'ed later
	ENDIF
	ENDIF	;TRACKNAMES

	IFDEF	TRACKNAMES
Respond_SlavePoll_4:
	movlw	0x70				; 'Track name'
	call	SendByte

	movf	MyId, 0
	call	SendByte

	movlw	0xC9
	call	SendByte

	movf	Track0, 0
	call	SendByte

	movf	Checksum, 0
	movwf	Count2

	movf	Checksum, 0
	call	SendByte

	movf	Count2, 0
	movwf	Checksum
	
	movf	Track1, 0
	call	SendByte

	movf	Track2, 0
	call	SendByte

	movf	Track3, 0
	call	SendByte

	movf	Track4, 0
	call	SendByte
	
	movf	Track5, 0
	call	SendByte

	movf	Track6, 0
	call	SendByte

	movf	Track7, 0
	call	SendByte

	movlw	0x00
	call	SendByte

	swapf	DiskNum, 0			; Disk Number + Disp code
	IFDEF	PIC_16
	btfss	OUT_PORT, CdNotMdBit		; unconnected = high = CD
	movlw	0x00				; field 0
	ENDIF
	iorwf	DispCtrl, 0
	call	SendByte

	IFDEF	LONGNAMES
	goto	Slave_Respond_End

;--------
Respond_SlavePoll_11:
	movlw	0x80
	movwf	Display

Respond_SlavePoll_5:
	movlw	0x70				; 'Track name, part2'
	call	SendByte

	movf	MyId, 0
	call	SendByte

	movlw	0xC9
	call	SendByte

	movf	Track8, 0
	call	SendByte

	movf	Checksum, 0
	movwf	Count2

	movf	Checksum, 0
	call	SendByte

	movf	Count2, 0
	movwf	Checksum
	
	movf	Track9, 0
	call	SendByte

	movf	TrackA, 0
	call	SendByte

	movf	TrackB, 0
	call	SendByte

	movf	TrackC, 0
	call	SendByte
	
	movf	TrackD, 0
	call	SendByte

	movf	TrackE, 0
	call	SendByte

	movf	TrackF, 0
	call	SendByte

	movlw	0x00
	call	SendByte

	movlw	0x10				; field 1
	iorwf	DispCtrl, 0
	call	SendByte

	ELSE
	incf	Display, 1			; skip response '0x08'
	ENDIF	;LONGNAMES

	goto	Respond_End
	ENDIF	;TRACKNAMES

;--------

Respond_SlavePoll_12:
	IFNDEF	LONGNAMES
	movlw	0x7f				;do a 'nubers' poll next
	movwf	Display				; will be inc'ed later
	ENDIF

Respond_SlavePoll_6:
	movlw	0x70				; 'CD name'
	call	SendByte

	movf	MyId, 0
	call	SendByte

	movlw	0xCD
	call	SendByte

	movf	Disk0, 0
	call	SendByte

	movf	Checksum, 0
	movwf	Count2

	movf	Checksum, 0
	call	SendByte

	movf	Count2, 0
	movwf	Checksum
	
	movf	Disk1, 0
	call	SendByte

	movf	Disk2, 0
	call	SendByte

	movf	Disk3, 0
	call	SendByte

	movf	Disk4, 0
	call	SendByte
	
	movf	Disk5, 0
	call	SendByte

	movf	Disk6, 0
	call	SendByte

	movf	Disk7, 0
	call	SendByte

	movlw	0x00
	call	SendByte

	swapf	DiskNum, 0			; Disk Number + Disp code
	IFDEF	PIC_16
	btfss	OUT_PORT, CdNotMdBit		; unconnected = high = CD
	movlw	0x00				; field 0
	ENDIF
	iorwf	DispCtrl, 0
	call	SendByte

	IFDEF	LONGNAMES
	goto	Slave_Respond_End

;--------
Respond_SlavePoll_13:
	movlw	0x80
	movwf	Display

Respond_SlavePoll_7:
	movlw	0x70				; 'CD name, part 2'
	call	SendByte

	movf	MyId, 0
	call	SendByte

	movlw	0xCD
	call	SendByte

	movf	Disk8, 0
	call	SendByte

	movf	Checksum, 0
	movwf	Count2

	movf	Checksum, 0
	call	SendByte

	movf	Count2, 0
	movwf	Checksum
	
	movf	Disk9, 0
	call	SendByte

	movf	DiskA, 0
	call	SendByte

	movf	DiskB, 0
	call	SendByte

	movf	DiskC, 0
	call	SendByte
	
	movf	DiskD, 0
	call	SendByte

	movf	DiskE, 0
	call	SendByte

	movf	DiskF, 0
	call	SendByte

	movlw	0x00
	call	SendByte

	movlw	0x10				; field 0
	iorwf	DispCtrl, 0
	call	SendByte

	ELSE
	incf	Display, 1			; skip response '0x08'
	ENDIF	;LONGNAMES

	IFDEF	SERIAL_INPUT
	movf	Display, 0			; check for 0x09th response
	xorlw	0x08				; so that serial can be enabled
	btfss	STATUS, Z
	goto	Respond_SlavePoll_Done

	movf	RCREG, 0			; clear RX buffer
	movf	RCREG, 0
	bcf	RCSTA, CREN			; clear overflow
	bsf	RCSTA, CREN

	bsf	STATUS, RP0			; Select BANK1
	bsf	PIE1, RCIE			; enable RX interrupt
	bcf	STATUS, RP0			; Select BANK0
	ENDIF

Respond_SlavePoll_Done:
	movlw	0x80				; to stop slave polling
	movwf	Display

	goto	Respond_End

;----

Slave_Respond_End:
	movf	Checksum, 0			; Send Checksum
	call	SendByte
	
	movlw	0x00
	call	SendByte

	call	IssueSlaveBreak			; cause next_display to i
						; appear quickly

	goto	ReceiveBreak			; Done, wait for next command

