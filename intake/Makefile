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

export ARMGCC_INSTALL_PATH=$(FX3_TOOLCHAIN)/arm-2013.11
export ARMGCC_VERSION="4.8.1"
FX3FWROOT=$(FX3_TOOLCHAIN)
ELF2IMG=$(FX3_TOOLCHAIN)/elf2img/elf2img

all:mkimg

include $(FX3_TOOLCHAIN)/common/fx3_build_config.mak

#CCFLAGS+=-Wextra -Wno-unused-parameter

MODULE = usb-storage-bridge-intake

SOURCE += cyfxtx.c
SOURCE += serial.c
SOURCE += app.c
SOURCE += StartUp.c
SOURCE += Support.c
SOURCE += USB_Descriptors.c
SOURCE += USB_Handler.c
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
	rm -f ./*.elf
	rm -f ./*.map
	rm -f ./*.o
	rm -f ./*.img

compile: $(C_OBJECT) $(A_OBJECT) $(EXES)

mkimg: $(MODULE).$(EXEEXT)
	$(ELF2IMG) -i $(MODULE).$(EXEEXT) -o $(MODULE).img

#[]#
