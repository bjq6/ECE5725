# Compiling and runnning bare metal projects on RPI2/3

1. install arm-none-eabi toolchain from launchpad to `/usr/local/` and make sure it's in your $PATH 

2. format SD card as fat32, named BOOT.

3. copy over RPi's boot code (bootcode.bin and start.elf) to the root of the SD card. 

4. run with sd card still in. First will compile a kernel.img to run, the second will copy to SD card (this is convenience target, can also easily just copy the resulting kernel.img to the SD card root).
```
> make
> make copy
```
5. install SD card to RPi and plug in power.
