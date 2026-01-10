# stm32n6570-dk

This project aims to evaluate the camera IP and NPU included into stm32n6 SoC from ST-Microelectronics.

Current status: Peripherals initialized, the YOLOX object detector model implemented.

This project runs directly from SRAM (no secure boot) which aim to ease development workflow. Enable developer mode by switching the BOOT1 jumper to the HIGH position.

Note this project uses MDEPX RTOS which replaces STM32 HAL in full.

### Set up compiler
    $ sudo apt install gcc-arm-none-eabi

### Get sources and build the project
    $ git clone --recursive https://github.com/mdepx/stm32n6570-dk
    $ cd stm32n6570-dk
    $ make clean all

### Flash network data
    $ make flash-network-data

### Run GDB server (in a separate terminal)
    $ ./gdb-server.sh

### Place the binary into SRAM and run
    $ make run

![n6dk](https://raw.githubusercontent.com/mdepx/stm32n6570-dk/main/images/yolox.jpg)
![n6dk dev mode](https://raw.githubusercontent.com/mdepx/stm32n6570-dk/main/images/n6dk_devmode.jpg)
