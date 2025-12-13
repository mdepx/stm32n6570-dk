# stm32n6570-dk

This project aims to evaluate the NPU included into stm32n6 SoC from ST-Microelectronics.

Current status: Display initialized, no camera or NPU yet in use.

### Set up compiler
    $ sudo apt install gcc-arm-none-eabi

### Get sources and build the project
    $ git clone --recursive https://github.com/mdepx/stm32n6570-dk
    $ cd stm32n6570-dk
    $ make clean all

### Run GDB server
    $ ./gdb-server.sh

### Place the binary into SRAM and run
    $ make run

![n6dk](https://raw.githubusercontent.com/mdepx/stm32n6570-dk/main/images/n6dk.jpg)
![n6dk dev mode](https://raw.githubusercontent.com/mdepx/stm32n6570-dk/main/images/n6dk_devmode.jpg)
