# GNUnilink - emulates Sony CD/MD Multichangers

# system programs
RM              = /bin/rm -f
ARC             = tar -cvf
ZIP             = gzip

# gpasm programs
ASM             = gpasm
DIASM           = gpdasm
CODVIEWER       = gpvc -d 
SIM             = gpsim

# project data
PROJECT         = GNUnilink
VERSION         = 1.2
MAIN_FILE       = GNUnilink.asm 
FILE_LIST       = interrupt.asm functions.asm setup.asm receive.asm \
			parse.asm respond.asm display.asm
HEADER	        = /usr/local/share/gpasm/header

# derived variables
ARCHIVE         = $(PROJECT)_$(VERSION).tar
HEX_FILE        = $(PROJECT).hex
COD_FILE        = $(PROJECT).cod

all:	$(HEX_FILE)

$(COD_FILE): $(HEX_FILE)

$(HEX_FILE): $(MAIN_FILE) $(FILE_LIST)	
	$(ASM) -I $(HEADER) -o $(HEX_FILE) $(MAIN_FILE)

dis: $(HEX_FILE)
	$(DIASM) $(HEX_FILE)

viewcod: $(COD_FILE)
	$(CODVIEWER) $(COD_FILE)

$(ARCHIVE):
	$(ARC) $(ARCHIVE) *

archive: $(ARCHIVE)
	$(ZIP) $(ARCHIVE)

clean:
	$(RM) *.hex *.lst *.cod $(PROJECT)*.gz 

sim:
	gpsim -pp16f627 GNUnilink.hex -sGNUnilink.cod 

copy: GNUnilink.hex
	mount /floppy
	cp GNUnilink.hex /floppy/.
	umount /floppy
