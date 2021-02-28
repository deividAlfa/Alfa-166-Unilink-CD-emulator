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

; Work out which 'command' we have received and issue the appropriate response

Interpret:
	movf	Cmd1, 0
	xorlw	0x01
	btfsc	STATUS, Z
	goto	Cmd1_was_01

	movf	Cmd1, 0
	xorlw	0x02
	btfsc	STATUS, Z
	goto	Respond_Appoint

	movf	Cmd1, 0
	xorlw	0x20
	btfsc	STATUS, Z
	goto	Cmd_Selected

	movf	Cmd1, 0
	xorlw	0x26
	btfsc	STATUS, Z
	goto	But_NextTrack

	movf	Cmd1, 0
	xorlw	0x27
	btfsc	STATUS, Z
	goto	But_PrevTrack

	IFDEF	DISKDIRECT
	movf	Cmd1, 0
	xorlw	0xB0
	btfsc	STATUS, Z
	goto	But_DirectDisk
	ELSE
	movf	Cmd1, 0
	xorlw	0x28
	btfsc	STATUS, Z
	goto	But_NextDisk

	movf	Cmd1, 0
	xorlw	0x29
	btfsc	STATUS, Z
	goto	But_PrevDisk
	ENDIF

	IFDEF   SERIAL_OUTPUT
	IFDEF	TRACKMODES
	movf	Cmd1, 0
	xorlw	0x34
	btfsc	STATUS, Z
	goto	But_Repeat

	movf	Cmd1, 0
	xorlw	0x35
	btfsc	STATUS, Z
	goto	But_Shuffle

	movf	Cmd1, 0
	xorlw	0x36
	btfsc	STATUS, Z
	goto	But_Intro
	ENDIF
	ENDIF	;SERIAL_OUTPUT

	movf	Cmd1, 0
	xorlw	0x87
	btfsc	STATUS, Z
	goto	Cmd_PowerOff

	movf	Cmd1, 0
	xorlw	0xF0
	btfsc	STATUS, Z
	goto	Cmd_Deselected

	goto	Cmd_Unkown

;--------

Cmd1_was_01:
	movf	Cmd2, 0
	btfsc	STATUS, Z			; Check for 'Init'
	goto	WarmBoot

	movf	Cmd2, 0
	xorlw	0x02				; Check for 'Anyone'
	btfsc	STATUS, Z
	goto	Respond_Anyone

	movf	Cmd2, 0
	xorlw	0x11				; Check for 'Alive'
	btfsc	STATUS, Z
	goto	Respond_Hello

	movf	Cmd2, 0
	xorlw	0x12				; Check for 'Ping'
	btfsc	STATUS, Z
	goto	Respond_Ping

	movf	Cmd2, 0
	xorlw	0x13				; Check for 'Slave Poll'
	btfsc	STATUS, Z
	goto	Respond_SlavePoll

	movf	Cmd2, 0
	xorlw	0x15				; Check for 'Master Poll'
	btfsc	STATUS, Z
	goto	Respond_MasterPoll

	goto	Cmd_Unkown			; Unkown command

Cmd_Unkown:
	goto	ReceiveBreak

;--------

Cmd_Selected:
	movf	MyId, 0				; ensure it's the right ID
	subwf	Rad, 0
	btfss	STATUS, Z
	goto	ReceiveBreak

Cmd_Selected2:
	IFDEF	SERIAL
	bsf	PORTB, 3			; Enable CTS
	ENDIF

	IFDEF	SERIAL_INPUT
	bsf	STATUS, RP0			; Select BANK1
	bcf	PIE1, RCIE			; disable RX interrupt
	bcf	STATUS, RP0			; Select BANK0
	ENDIF

	movlw	0x01				; Restart the display sequence
	movwf	Display

	call	IssueSlaveBreak			; cause display to 
						; appear quickly

	IFDEF	SERIAL_OUTPUT
	movlw	(command_start - command_start)
	goto	But_Done
	ELSE
	goto	ReceiveBreak
	ENDIF

Cmd_PowerOff:
	movf	Cmd2, 0
	xorlw	0x6B
	btfss	STATUS, Z
	goto	ReceiveBreak

	IFDEF	SERIAL
	bcf	PORTB, 3			; Disable CTS
	ENDIF

	IFDEF	SERIAL_INPUT
	bsf	STATUS, RP0			; Select BANK1
	bcf	PIE1, RCIE			; disable RX interrupt
	bcf	STATUS, RP0			; Select BANK0
	ENDIF

	clrf	Display 			; Immediately stop attempting 
						; to display stuff

	IFDEF	SERIAL_OUTPUT
	movlw	(command_stop - command_start)
	goto	But_Done
	ELSE
	goto	ReceiveBreak
	ENDIF

Cmd_Deselected:
	movf	Cmd2, 0				; ensure it's the right ID
	subwf	MyId, 0
	btfsc	STATUS, Z
	goto	Cmd_Selected2			; actually it's me selected

	IFDEF	SERIAL
	bcf	PORTB, 3			; Disable CTS
	ENDIF

	IFDEF	SERIAL_INPUT
	bsf	STATUS, RP0			; Select BANK1
	bcf	PIE1, RCIE			; disable RX interrupt
	bcf	STATUS, RP0			; Select BANK0
	ENDIF

	clrf	Display 			; Immediately stop attempting 
						; to display stuff

	IFDEF	SERIAL_OUTPUT
	movlw	(command_pause - command_start)
	goto	But_Done
	ELSE
	goto	ReceiveBreak
	ENDIF

;----
; Button outputs, cause strings to appear on serial port

But_NextTrack:
	IFDEF   SERIAL_OUTPUT
	movlw	(command_next_track - command_start)
	ENDIF

	goto	But_Done

But_PrevTrack:
	IFDEF   SERIAL_OUTPUT
	movlw	(command_previous_track - command_start)
	ENDIF

	goto	But_Done

	IFDEF	DISKDIRECT
But_DirectDisk:
	IFDEF   SERIAL_OUTPUT
	movlw	(command_direct_disk - command_start)
	ENDIF
	goto	But_Direct_Done
	ELSE	;DISKDIRECT
But_NextDisk:
	IFDEF   SERIAL_OUTPUT
	movlw	(command_next_disk - command_start)
	ENDIF

	goto	But_Done

But_PrevDisk:
	IFDEF   SERIAL_OUTPUT
	movlw	(command_previous_disk - command_start)
	ENDIF

	goto	But_Done
	ENDIF	;DISKDIRECT

	IFDEF   SERIAL_OUTPUT
	IFDEF	TRACKMODES
But_Intro:
	movlw	(command_intro - command_start)
	goto	But_Done

But_Repeat:
	movlw	(command_repeat - command_start)
	goto	But_Done

But_Shuffle:
	movlw	(command_shuffle - command_start)
	goto	But_Done
	ENDIF
	ENDIF	;SERIAL_OUTPUT

	IFDEF	DISKDIRECT
But_Direct_Done:
	movwf	command_offset			; store offset for later

	movf	Cmd2, 0				; Send the disk number
	andlw	0x0F
	xorlw	0x30

	movwf	TXREG

	goto	But_Done2
	ENDIF

But_Done:
	IFDEF   SERIAL_OUTPUT
	movwf	command_offset			; Select command 3 'PRV_LIST'

	movlw	HIGH serial_command_table
	movwf	PCLATH
	call	serial_command_table		; Get first character to tx 
						; from Look-up table

	movwf	TXREG
	incf	command_offset, 1		; now points to first byte to
						; be sent by interrupt
But_Done2:
	bsf	STATUS, RP0			; Select bank 1
	bsf	PIE1, TXIE			; Enable TX interrupts
	bcf	STATUS, RP0			; select bank 0
	ENDIF

	goto	ReceiveBreak
