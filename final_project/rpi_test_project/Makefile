
ARMGNU ?= arm-none-eabi
CFLAGS = -Wall -Werror -O2 -nostdlib -nostartfiles -ffreestanding -march=armv7-a -mtune=cortex-a7

TARGET = rpi_test
OBJDIR = obj

all : setup gcc memmap
gcc : $(OBJDIR)/kernel.img

setup: 
	mkdir -p $(OBJDIR)

clean :
	rm -rf $(OBJDIR)/

## create the binary for execution
$(OBJDIR)/kernel.img : $(OBJDIR)/$(TARGET).elf
	$(ARMGNU)-objcopy $< -O binary $@

## link objects into an elf and dump it
$(OBJDIR)/$(TARGET).elf : $(OBJDIR)/vectors.o $(OBJDIR)/$(TARGET).o ## inclue the other targets here. 
	$(ARMGNU)-ld $^ -T memmap -o $@
	$(ARMGNU)-objdump -D $@ > $@.list

## compile all the source into objects
$(OBJDIR)/vectors.o : vectors.s
	$(ARMGNU)-as $< -o $@

$(OBJDIR)/$(TARGET).o : $(TARGET).c ## include related h files here 
	$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@

## create other (C) object with this. Copy as example above for assembly 

#$(OBJDIR)/<target name> : <target source, c file first>
#	$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@