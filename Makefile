APP =		stm32n6570-dk

CROSS_COMPILE ?= arm-none-eabi-
export CROSS_COMPILE

CC =		${CROSS_COMPILE}gcc
LD =		${CROSS_COMPILE}ld
OBJCOPY =	${CROSS_COMPILE}objcopy

OSDIR =		mdepx

ADAPTOR_ID ?= 000700284142500E20353451

all:
	python3 -B ${OSDIR}/tools/emitter.py -j mdepx.conf
	@${OBJCOPY} -O binary obj/${APP}.elf obj/${APP}.bin
	${CROSS_COMPILE}size -x obj/${APP}.elf

readelf:
	${CROSS_COMPILE}readelf -a obj/${APP}.elf | less

objdump:
	${CROSS_COMPILE}objdump -S -d obj/${APP}.elf | less

reset:
	~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI -c port=SWD mode=HOTPLUG ap=0 -hardRST

clean:
	@rm -rf obj/*

sign-untested:
	rm -f obj/${APP}-signed.bin
	~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_SigningTool_CLI -bin obj/${APP}.bin -nk -of 0x80000000 -t fsbl -o obj/${APP}-signed.bin -hv 2.3 -dump obj/${APP}-signed.bin -align

flash:
	~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI -c port=SWD mode=HOTPLUG -el ~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr -hardRST -w obj/${APP}-signed.bin 0x70000000

erase_all:
	~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer.sh -c port=SWD mode=HOTPLUG reset=HWrst -el ~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr -e all

run:	all
	gdb-multiarch ./obj/${APP}.elf

gdb-server:	reset
	~/st/stm32cubeide_2.0.0/plugins/com.st.stm32cube.ide.mcu.externaltools.stlink-gdb-server.linux64_2.2.300.202509021040/tools/bin/ST-LINK_gdbserver -p 61234 -l 1 -d -s -cp ~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ -m 1 -g -i ${ADAPTOR_ID}

dump_regs:
	~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer.sh -c port=SWD mode=HOTPLUG -el ~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr -regdump regdump.txt

include ${OSDIR}/mk/user.mk
