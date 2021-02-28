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
; Various responses to the commands
; These are clocked out by the master when it is ready

Respond_Anyone:
	movf	MyId, 1				; Test if we've got an ID
	btfss	STATUS, Z
	goto	ReceiveBreak			; we don't need another one

	clrf	Checksum 			; Clear checksum

	movlw	0x10				; 'I'm a .....'
	
	call	SendByte

	movf	MyGroup, 0
	call	SendByte

Respond_Anyone2:
	movlw	Anyone_0
	call	SendByte

	movlw	Anyone_1
	call	SendByte

	movf	Checksum, 0			; Save Current Checksum
	movwf	Count2

	movf	Checksum, 0			; Send Checksum
	call	SendByte

	movf	Count2, 0			; Restore Checksum
	movwf	Checksum
	
	movlw	Anyone_2
	call	SendByte

	movlw	Anyone_3
	call	SendByte

	movlw	Anyone_4
	call	SendByte

	movlw	Anyone_5
	call	SendByte
	
Respond_End:
	movf	Checksum, 0			; Send Checksum
	call	SendByte
	
	movlw	0x00
	call	SendByte
	
	goto	ReceiveBreak			; Done, wait for next command

;--------

Respond_Appoint:
	movf	Rad, 0				; Store allocated ID
	movwf	MyId

	movf	Cmd2, 0				; Store bit position 
	movwf	MyBit				; (for SlavePoll response)

Respond_App2:
	clrf	Checksum 			; Clear checksum

	movlw	0x10				; 'OK I'm ID=X'
	call	SendByte

	movf	MyId, 0
	call	SendByte

	IFDEF	PIC_16
	btfss	OUT_PORT, CdNotMdBit		; unconnected = CD
	goto	Respond_Anyone2			; same as 'Anyone'

	movlw	Appoint_0			; different response
	call	SendByte

	movlw	Appoint_1
	call	SendByte

	movf	Checksum, 0			; Save Current Checksum
	movwf	Count2

	movf	Checksum, 0			; Send Checksum
	call	SendByte

	movf	Count2, 0			; Restore Checksum
	movwf	Checksum
	
	movlw	Appoint_2
	call	SendByte

	movlw	Appoint_3
	call	SendByte

	movlw	Appoint_4
	call	SendByte

	movlw	Appoint_5
	call	SendByte

	goto	Respond_End
	ELSE
	goto	Respond_Anyone2			; same as 'Anyone'
	ENDIF

;--------

Respond_Hello:
	movf	MyId, 1				; Test for ID
	btfss	STATUS, Z
	goto	ReceiveBreak			; keep quiet if we have...

	clrf	Checksum 			; Clear checksum

	movlw	0x10				; 'You've missed me!!'
	call	SendByte

	movlw	0x18
	call	SendByte

	movlw	0x04
	call	SendByte

	movlw	0x00
	call	SendByte

	goto	Respond_End

;--------

Respond_Ping:
	movf	MyId, 0				; Check it's the right ID
	subwf	Rad, 0
	btfss	STATUS, Z
	goto	ReceiveBreak			; Done, wait for next command

	clrwdt					; Reset the watchdog

	clrf	Checksum 			; Clear checksum

	movlw	0x10				; 'I'm playing'
	call	SendByte

	movf	MyId, 0
	call	SendByte

	movlw	0x00
	call	SendByte

	movlw	0x80				; 'Idle'
	
	movf	Display, 1			; test if we're active
	btfss	STATUS, Z
	movlw	0x00				; 'Playing'

	call	SendByte

	movf	Checksum, 0			; Send Checksum
	call	SendByte
	
	movlw	0x00
	call	SendByte

	movf	Display, 0
	andlw	0x7f				; strip off high nyble
	xorlw	0x00				; Test to see if we've got 
						; something to display
	btfss	STATUS, Z
	call	IssueSlaveBreak
	
	goto	ReceiveBreak			; wait for next command

;******************************************************************************
; Response to Master Poll Command

Respond_MasterPoll:
	movf	Display, 0
	xorlw	0x00				; Test if we're active
	btfsc	STATUS, Z
	goto	ReceiveBreak			; wait for next command
	
	clrf	Checksum 			; Clear checksum

	movlw	0x10				; 'Something has changed!'
	call	SendByte

	movlw	0x18				; is to a broadcast address
	call	SendByte

	movlw	0x82
	call	SendByte

	movlw	0x00				; MyBit is 0x0? or 0x1?
	call	Bit_Frig
	call	SendByte

	movf	Checksum, 0			; Save Current Checksum
	movwf	Count2

	movf	Checksum, 0			; Send Checksum
	call	SendByte

	movf	Count2, 0			; Restore Checksum
	movwf	Checksum
	
	movlw	0x20				; MyBit is 0x2? or 0x3?
	call	Bit_Frig
	call	SendByte

	movlw	0x40				; MyBit is 0x4? or 0x5?
	call	Bit_Frig
	call	SendByte

	movlw	0x60				; MyBit is 0x6? or 0x7?
	call	Bit_Frig
	call	SendByte

	movlw	0x80				; MyBit is 0x8? or 0x9?
	call	Bit_Frig
	call	SendByte
	
	goto	Respond_End			; to do checksum and error byte

;******************************************************************************

