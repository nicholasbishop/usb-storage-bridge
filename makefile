## Copyright Cypress Semiconductor Corporation, 2010-2011,
## All Rights Reserved
## UNPUBLISHED, LICENSED SOFTWARE.
##
## CONFIDENTIAL AND PROPRIETARY INFORMATION
## WHICH IS THE PROPERTY OF CYPRESS.
##
## Use of this file is governed
## by the license agreement included in the file
##
##      <install>/license/license.txt
##
## where <install> is the Cypress software
## installation root directory path.
##

export ARMGCC_INSTALL_PATH=/home/nbishop/cypress_fx3/ARM_GCC/arm-2013.11
export ARMGCC_VERSION="4.8.1"
FX3FWROOT=/home/nbishop/cypress_fx3/firmware
FX3PFWROOT=/home/nbishop/cypress_fx3/firmware/firmware/u3p_firmware
ELF2IMG=/home/nbishop/cypress_fx3/firmware/util/elf2img/elf2img

all:mkimg

include $(FX3FWROOT)/firmware/common/fx3_build_config.mak

MODULE = usb-storage-bridge

#SOURCE += $(MODULE).c
SOURCE += cyfxmscdemo.c
SOURCE += cyfxmscdscr.c
SOURCE += cyfxtx.c
#SOURCE += DebugConsole.c
SOURCE_ASM += cyfx_gcc_startup.S

C_OBJECT=$(SOURCE:%.c=./%.o)
A_OBJECT=$(SOURCE_ASM:%.S=./%.o)

EXES = $(MODULE).$(EXEEXT)

$(MODULE).$(EXEEXT): $(A_OBJECT) $(C_OBJECT)
	$(LINK)

$(C_OBJECT) : %.o : %.c
	$(COMPILE)

$(A_OBJECT) : %.o : %.S
	$(ASSEMBLE)

clean:
	rm -f ./$(MODULE).$(EXEEXT)
	rm -f ./$(MODULE).map
	rm -f ./*.o

compile: $(C_OBJECT) $(A_OBJECT) $(EXES)

mkimg: $(MODULE).$(EXEEXT)
	$(ELF2IMG) -i $(MODULE).$(EXEEXT) -o $(MODULE).img

#[]#
