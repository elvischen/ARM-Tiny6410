uart.bin: start.o main.o uart.o clock.o
	arm-linux-ld -Ttext 0x50000000 -o uart.elf $^
	arm-linux-objcopy -O binary uart.elf uart.bin
	arm-linux-objdump -D uart.elf > uart.dis

%.o : %.S
	arm-linux-gcc -o $@ $< -c

%.o : %.c
	arm-linux-gcc -o $@ $< -c -fno-builtin 
# GCC编译器提供了很多内置函数来来替换C库函数，以达到优化目的
# 使用-fno-builtin参数来关闭GCC内置函数功能。

clean:
	rm *.o *.elf *.bin *.dis  -rf

