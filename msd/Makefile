export ARMGCC_INSTALL_PATH=$(FX3_TOOLCHAIN)/arm-2013.11
export ARMGCC_VERSION="4.8.1"
FX3FWROOT=$(FX3_TOOLCHAIN)
ELF2IMG=$(FX3_TOOLCHAIN)/elf2img/elf2img

all:mkimg

include $(FX3_TOOLCHAIN)/common/fx3_build_config.mak

CCFLAGS+=-Wextra -Wno-unused-parameter -Wno-sign-compare

MODULE = usb-storage-bridge-msd

SOURCE += cyfxmscdemo.c
SOURCE += cyfxmscdscr.c
SOURCE += cyfxtx.c
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
