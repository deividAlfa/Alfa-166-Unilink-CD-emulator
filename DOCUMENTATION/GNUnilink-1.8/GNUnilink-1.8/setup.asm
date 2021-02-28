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

; Initialisation of all the variables, called on power up on on a bus reset.

SystemSetup:	
	movlw	0
	movwf	Time1
	movwf	Time0
	movwf	DiskNum

	movlw	1
	movwf	TrackNum

	movlw	0x0e				; default Disp mode - user selects
	movwf	DispCtrl

	IFDEF	TRACKNAMES
	movlw	Track0_def
	movwf	Track0
	movlw	Track1_def
	movwf	Track1
	movlw	Track2_def
	movwf	Track2
	movlw	Track3_def
	movwf	Track3
	movlw	Track4_def
	movwf	Track4
	movlw	Track5_def
	movwf	Track5
	movlw	Track6_def
	movwf	Track6
	movlw	Track7_def
	movwf	Track7

	IFDEF	LONGNAMES
	movlw	' '
	movwf	Track8
	movwf	Track9
	movwf	TrackA
	movwf	TrackB
	movwf	TrackC
	movwf	TrackD
	movwf	TrackE
	movwf	TrackF
	ENDIF
	ENDIF	;TRACKNAMES

	movlw	Disk0_def
	movwf	Disk0
	movlw	Disk1_def
	movwf	Disk1
	movlw	Disk2_def
	movwf	Disk2
	movlw	Disk3_def
	movwf	Disk3
	movlw	Disk4_def
	movwf	Disk4
	movlw	Disk5_def
	movwf	Disk5
	movlw	Disk6_def
	movwf	Disk6
	movlw	Disk7_def
	movwf	Disk7

	IFDEF	LONGNAMES
	movlw	' '
	movwf	Disk8
	movwf	Disk9
	movwf	DiskA
	movwf	DiskB
	movwf	DiskC
	movwf	DiskD
	movwf	DiskE
	movwf	DiskF
	ENDIF

WarmBoot:
	clrf	MyId 				; Set up initial values
	clrf	Display
	clrf	MyBit
	
	IFDEF	PIC_16
	bcf	STATUS, RP0

	IFDEF	PIC_16F62x
	bcf	STATUS, RP1
	ENDIF

	clrf	PORTA

	IFDEF	PIC_16F62x
	movlw	0x07				; Disable the Comparitors
	movwf	CMCON
	ENDIF

	bsf	STATUS, RP0			; Configure Ports
	movlw	0xEF
	movwf	TRISA				; Port A

	movlw	0x03
	movwf	TRISB				; Port B - even if not used

	movlw	0x0F				; Set up watchdog timer
	iorwf	OPTION_REG, 1
	clrwdt

	bcf	STATUS, RP0
	clrf	PORTA
	clrf	PORTB
	ENDIF

	IFDEF SERIAL 				; Code to enable serial port
	bsf	STATUS, RP0			; Select BANK1
	movlw	0x26
	movwf	TXSTA				; Set transmit options
	movlw	baud_val
	movwf	SPBRG				; Set BAUD rate
	clrf	PIE1
	bcf	STATUS, RP0			; Select register bank 0
	clrf	PIR1
	movlw	0x80
	movwf	RCSTA				; Set receive options
	movlw	0xc0
	movwf	INTCON				; Enable global interrupt & 
						; peripheral interrupt
	ENDIF

	movlw	0x30				; Set up Grouping - Either CD or MD
	IFDEF	PIC_16
	btfss	OUT_PORT, CdNotMdBit		; unconnected = high = CD
	movlw	0xD0
	ENDIF
	movwf	MyGroup

	goto	ReceiveBreak
