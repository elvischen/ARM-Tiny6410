ARM-Tiny6410-DRAM
====
**备注：此文件内的程序所编译的sdram.bin, 不能通过MiniTools.exe的裸机程序选项下载。**    
因为此程序是学习DRAM控制器的配置，而使用MiniTools.exe的裸机程序下载，是已经配置好DRAM后，将代码下载到SDRAM中的0x50000000地址处运行。程序可以运行，但无法验证程序是否成功配置SDRAM。   
**解决方法1**:  使用MiniTools.exe的Linux选项，将sdram.bin数据文件下载到Nand FLash设备的Linux Bootloader的位置。（详情见sd-no-os/文件夹的ARM启动流程）    
**解决方法2**:  使用dd命令将.bin文件通过dd命令直接烧写到SDHC卡的BL1区域。（详情见sd-no-os/文件夹的ARM启动流程）   

----

<http://www.samsung.com/us/business/oem-solutions/pdfs/PSG_2H_2012.pdf>    
DRAM:       
SAMSUNG K4X2G323PD-8GD8   
Mobile DDR     
64Mx32    
200Mhz     
Halogen Free FBGA90    
90-FBGA, 800MHz      
1.8V


	K4X2G323PD-8GD8 
	K:		SAMSUNG Memort
	4:	 	DRAM
	X:		Mobile DDR SDRAM
	2G:	2G, 8K/64ms
	32:	x32 (bit organization)		3:		4 of Internal Banks
	P:		LVTTL, 1.8V, 1.8V
	D:		5th Generation
	
	S/D: 90balls FBGA
	
**DDR SDRAM**:    
Double Rate Synchronous Dynamic Random Access Memory    
双倍速率传输，在时钟的上升沿和下降沿均可传输数据。

----

硬件连接：


	nCAS:		Xm1CASn
	nWE:		Xm1WEn
	
----

###DRAM控制器（DRAM CONTROLLER）
参考：<http://www.linuxidc.com/Linux/2011-09/42025.htm>   

6410使用DRAM控制器对DDR进行操作。   
在程序开始时要根据DDR的参数设置DRAM控制器。


DRAM控制器可以通过配置**兼容SDRAM类型芯片**。通过向DRAM控制器中PL340写入内存芯片配置参数，内存时序，来控制内存工作。 






* 支持64位的AMBA AXI总线类型

* 支持16位、64位内存总线
		
	* 存储器接口1：支持16位DDR SDRAM和Mobile DDR SDRAM类型芯片；
	*	支持32位DDR SDRAM，Mobile DDR SDRAM，Mobile SDR SDRAM和SDR SDRAM类型芯片；
	*	不支持16位Mobile SDR SDRAM和SDR SDRAM类型芯片

* 地址空间：存储器接口1支持最多2Gb地址空间
* 支持正常节能模式和预充电的节能模式
* 数据传输低延迟特性
* 外部存储器总线优化
* 通过设置SFR寄存器支持选择外部存储器选型
* 通过SFR寄存器配置存储器的时序
* 支持扩展MRS指令集
* 工作电压：存储器接口1:   1.8V，2.5V




a) 向mem_cmd寄存器写入2b10，使其进入NOP工作状态

b) 向mem_cmd寄存器写入2b00，使其进入Prechargeall（整片预充电）工作状态

c) 向mem_cmd寄存器写入2b11，使其进入Autorefresh（自刷新）工作状态

d) 再次向mem_cmd寄存器写入2b11，使其进入Autorefresh（自刷新）工作状态

e) 向mem_cmd寄存器写入2b10，使其进入MRS工作状态，并且地址空间内的EMRS必须置位

f) 再次向mem_cmd寄存器写入2b10，使其进入MRS工作状态，并且地址空间内的MRS必须置位

备注：程序中并没有使用这种配置方法，而是DRAM CONTROLLER INITIALIZATION SEQUENCE。上述方法未测试。

----

ISSUE
====

Makefile中，如果去掉main.c/uart.c/sdram.c编译选项中的**优化选项'-O2'**，编译出来的程序在ARM开发板上运行错误，无法实现正常的串口通信。

Makefile:

	test1:
		arm-linux-gcc -o start.o start.S -c
		arm-linux-gcc -o clock.o clock.c -c
		arm-linux-gcc -g -c -O2 -o main.o main.c    # must -O2 !!!
		arm-linux-gcc -g -c -O2 -o uart.o uart.c    # must -O2 !!!
		arm-linux-gcc -g -c -O2 -o sdram.o sdram.c  # must -O2 !!!
		arm-linux-ld -T sdram.lds -o sdram.elf start.o main.o clock.o uart.o sdram.o
		arm-linux-objcopy -O binary sdram.elf sdram.bin
		arm-linux-objdump -D sdram.elf > sdram.dis
	test2:
		arm-linux-gcc -o start.o start.S -c
		arm-linux-gcc -o clock.o clock.c -c
		#arm-linux-gcc -o main.o main.c -c          # 没有-O2优化选项，导致程序无法正常工作!
		arm-linux-gcc -g -c -o main.o main.c        # 删掉-O2优选选项，发现串口无法输入
		#arm-linux-gcc -o uart.o uart.c -c
		arm-linux-gcc -g -c -O2 -o uart.o uart.c
		#arm-linux-gcc -o sdram.o sdram.c -c
		arm-linux-gcc -g -c -O2 -o sdram.o sdram.c
		arm-linux-ld -T sdram.lds -o sdram.elf start.o main.o clock.o uart.o sdram.o
		arm-linux-objcopy -O binary sdram.elf sdram.bin
		arm-linux-objdump -D sdram.elf > sdram.dis


###[gcc常用选项对代码的影响](http://if.ustc.edu.cn/%7Exbzhou/blog/archives/sth_gcc.html#I198)




