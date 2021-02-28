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

	TITLE           "GNU-link : CD-Multichanger Emulation"
	SUBTITLE        "Released under the GPL license, (c) S Wood, Oct 2001"

;******************************************************************************
; GNUnilink Configuration.
; Uncomment the lines relivent to your board... 
;
; this should be only configuration required.

; Type of PIC (or closest)
;#define		PIC_12C509
;#define		PIC_16F84
#define		PIC_16F627
;#define		PIC_16F628

; Speed (or closet)
#define		SPEED_20MHZ
;#define		SPEED_16MHZ
;#define		SPEED_10MHZ
;#define		SPEED_8MHZ
;#define		SPEED_4MHZ

; Serial Output - report the button presses on the head unit to the PC via
; serial port
#define		SERIAL_OUTPUT

; Serial Input - allow PC to change Disk/Track Names/Numbers and report Track 
; Counter via serial port
#define		SERIAL_INPUT

; Track Names - use tracknames (as well as disknames)
#define		TRACKNAMES

; Track mode selection - detects Intro, Repeat and Shuf modes.
;#define		TRACKMODES

; Direct Disk Selection.
;#define		DISKDIRECT

; Long Names - use 16 characters Disk/Track names, only usable if serial 
; input is also selected.
;#define		LONGNAMES

;******************************************************************************
; Real settings for the PIC - if you need to add a new one do it here, 
; otherwise select one from above

		Radix   DEC
		EXPAND

	IFDEF	PIC_12C509
		Processor       12c509
		include         "p12c509.inc"

	__CONFIG _CP_OFF & _XT_OSC

#define		OUT_PORT	GPIO
#define		PIC_12
	ENDIF

	IFDEF	PIC_16F84
		Processor       16f84a
		include         "p16f84a.inc"

	__CONFIG _CP_OFF & _PWRTE_ON & _HS_OSC

#define		PIC_16
	ENDIF

	IFDEF	PIC_16F627
		Processor       16f627
		include         "p16f627.inc"

	__CONFIG _CP_OFF & _PWRTE_ON & _HS_OSC

#define		PIC_16F62x
#define		PIC_16

; Nast Hack to only use timing for the slave break
#define		NASTY_HACK
	ENDIF

	IFDEF	PIC_16F628
		Processor       16f628
		include         "p16f628.inc"

	__CONFIG _CP_OFF & _PWRTE_ON & _HS_OSC

#define		PIC_16F62x
#define		PIC_16
	ENDIF

	IFDEF	PIC_16
#define		OUT_PORT	PORTA
#define		OUT_ENA		TRISA
	ENDIF

	IFDEF	SERIAL_OUTPUT
	IFNDEF	SERIAL
#define		SERIAL
	ENDIF
	ENDIF

	IFDEF	SERIAL_INPUT
	IFNDEF	SERIAL
#define		SERIAL
	ENDIF
	ENDIF

;******************************************************************************
; A few standard definitions

_ResetVector	set	0x00
_IntVector      set     0x04

TRUE	equ	1
FALSE	equ	0

;******************************************************************************
; Timing Calibration Values - These are non critical in that they don't have 
; to be absolutely correct, but it's nicer to be almost right if you can....

; 100us delay loop is 8 + (n * 7) cycles.

	IFDEF	SPEED_4MHZ
#define		delay100us	13
#define		baud_val	12	; 19231 baud
	ENDIF

	IFDEF	SPEED_8MHZ
#define		delay100us	27
#define		baud_val	25	; 19231 baud
	ENDIF

	IFDEF	SPEED_10MHZ
#define		delay100us	34
#define		baud_val	32	; 18939 baud
	ENDIF

	IFDEF	SPEED_16MHZ
#define		delay100us	56
#define		baud_val	51	; 19231 baud
	ENDIF

	IFDEF	SPEED_20MHZ
#define		delay100us	70
#define		baud_val	64	; 19231 baud
	ENDIF

;******************************************************************************
;                               Setup Register Locations

DataVal		set		0x20
Count		set		0x21
Count2		set		0x22
Checksum	set		0x23

Rad		set		0x24
Tad		set		0x25
Cmd1		set		0x26
Cmd2		set		0x27
Parity1		set		0x28
Data11		set		0x29
Data12		set		0x2A
Data13		set		0x2B
Data14		set		0x2C
Data21		set		0x2D
Data22		set		0x2E
Data23		set		0x2F
Data24		set		0x30
Data25		set		0x31
Parity2		set		0x32
EndByte		set		0x33

MyId		set		0x34
Display		set		0x35
MyBit		set		0x36
MyGroup		set		0x37

	IFDEF	SERIAL
command_offset	set		0x38	; used to hold the current output text
status_store	set		0x39	; stores STATUS reg during interrupt
w_store		set		0x3A	; stores w reg during interrupt
w_store		set		0x3B	; stores w reg during interrupt
SerialReplace	set		0x3C	; temp storage for replacement character
	ENDIF

	IFDEF	SERIAL_INPUT
Disk0		set		0x40
Disk1		set		0x41
Disk2		set		0x42
Disk3		set		0x43
Disk4		set		0x44
Disk5		set		0x45
Disk6		set		0x46
Disk7		set		0x47
	IFDEF	LONGNAMES
Disk8		set		0x48
Disk9		set		0x49
DiskA		set		0x4A
DiskB		set		0x4B
DiskC		set		0x4C
DiskD		set		0x4D
DiskE		set		0x4E
DiskF		set		0x4F
	ENDIF

Track0		set		0x50
Track1		set		0x51
Track2		set		0x52
Track3		set		0x53
Track4		set		0x54
Track5		set		0x55
Track6		set		0x56
Track7		set		0x57
	IFDEF	LONGNAMES
Track8		set		0x58
Track9		set		0x59
TrackA		set		0x5A
TrackB		set		0x5B
TrackC		set		0x5C
TrackD		set		0x5D
TrackE		set		0x5E
TrackF		set		0x5F
	ENDIF

Time1		set		0x60
Time0		set		0x61

DiskNum		set		0x62
TrackNum	set		0x63

RXChar		set		0x64
RXState		set		0x65
RXPointer	set		0x66

DispCtrl	set		0x67
	ENDIF


;******************************************************************************
; Port definitions, which signal is where on the OUT_PORT
#define		DataBit		0
#define 	ClockBit	1
#define		BusOnBit	2
#define 	CdNotMdBit	3

;******************************************************************************
; Default definations for responses, these may need to be changed for you
; particular HEADUNIT

Anyone_0	set		0x8C
Anyone_1	set		0x10
Anyone_2	set		0xFF
Anyone_3	set		0xFF
Anyone_4	set		0xFF
Anyone_5	set		0xFF

Appoint_0	set		0x8C
Appoint_1	set		0x10
Appoint_2	set		0x05
Appoint_3	set		0xA8
Appoint_4	set		0x00
Appoint_5	set		0x00

;******************************************************************************
; Default definations for names etc.

	IFDEF	TRACKNAMES
Track0_def	set		'G'		; Track Name
Track1_def	set		'P'
Track2_def	set		'L'
Track3_def	set		'-'
Track4_def	set		'2'
Track5_def	set		'0'
Track6_def	set		'0'
Track7_def	set		'3'
	ENDIF

Disk0_def	set		'<'		; Disk Name
Disk1_def	set		'A'
Disk2_def	set		'U'
Disk3_def	set		'X'
Disk4_def	set		'-'
Disk5_def	set		'I'
Disk6_def	set		'N'
Disk7_def	set		'>'

;******************************************************************************
; Start of code space

	ORG     _ResetVector
	
	goto	SystemSetup

;******************************************************************************

	include "interrupt.asm"
	include "functions.asm"
	include "setup.asm"
	include "receive.asm"
	include "parse.asm"
	include	"respond.asm"
	include "display.asm"

;******************************************************************************

	END
