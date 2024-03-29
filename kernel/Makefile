TARGET = kernel.elf
OBJS = main.o newlib.o libcxx.o \
		graphics/graphics.o graphics/font.o graphics/mouse.o graphics/console.o graphics/logger.o \
		memory/stack.o memory/segment.o memory/paging.o memory/memory_manager.o memory/heap.o \
		registry/registry.o \
		devices/pci.o devices/usb/xhc.o devices/usb/ring.o devices/usb/port.o devices/usb/device.o devices/usb/classdriver.o devices/usb/hid.o devices/usb/mouse.o devices/usb/keyboard.o

CPPFLAGS += -I. -I/x86_64-elf/include/c++/v1 -I/x86_64-elf/include -nostdlibinc -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17
LDFLAGS  += --entry CallKernel -z norelro --image-base 0x100000 --static -z separate-code -L/x86_64-elf/lib

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf *.o */*.o */*.bin */*/*.o

kernel.elf: $(OBJS) Makefile
	ld.lld $(LDFLAGS) -o kernel.elf $(OBJS) -lc -lc++ -lc++abi -lm

%.o: %.c Makefile
	clang $(CPPFLAGS) $(CFLAGS) -c $<

%.o: %.cpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.asm Makefile
	nasm -f elf64 -o $@ $<

graphics/font.bin: graphics/font.txt
	../makefont.py $@ $<

graphics/font.o: graphics/font.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $< $@