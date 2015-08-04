System Clock Control
=======================

FriendlyARM Tiny6410

----------------------------------
        
System Controller:         
	|              
	+------ System Clock Control           
	|          
	+------ System Power-management Control           
             
---------------------------------         

S3C6410:          

	system clock signals:          

		* ARMCLK - CPU
		* HCLK - AXI/AHB 总线外设(存储/中断/lcd等控制器)
		* PCLK - APB 总线外设(看门狗，定时器，SD等)

	PPL : 

		1. ARMPLL, 用于ARMCLK (for CPU); 
		2. main PLL, 用于HCLK & PLK (AHB/APB总线设备);
		3. extra PLL, 用于外设 (供UART,IIS,IIC使用, especially for audio related clocks.); 
	
	Five power-saving mode:    	
		NORMAL, IDLE, STOP, DEEP-STOP, and SLEEP
		

---------------------------------         
###CLOCK SOURCE SELECTION

The OM[4:0] pins determines the operating mode(启动模式, 如SD卡启动/NAND Flash启动等等) of S3C6410X when the external reset signal is asserted.   
As described in the table, the OM[0] selects the external clock source,   
i.e., if the OM[0] is 0, the XXTIpll (external crystal) is selected.  
Otherwise, XEXTCLK is selected.  

OM[4:0]决定了S3C6410的启动模式, 启动OM[0]决定了S3C6410的外部时钟源:  
	* OM[0] = 0, XEXTCLK, 外部时钟(external clock);
	* OM[1] = 1, XXTIpll, 外部晶振(external crystal;

The operating mode is mainly classified into six categories according to the boot device.    
The boot device can be among SROM, NOR, OneNAND, MODEM and Internal ROM.    

XSELNAND配置:   
* When NAND Flash device is used, XSELNAND pin must be 1, even if it is used as boot device or storage device.   
* When OneNAND Flash device is used, XSELNAND must be 0, even if it is used as boot device or storage device.   
* When NAND/OneNAND device is not used, XSELNAND can be 0 or 1.

参考: S3C6410X.pdf - Page 123 - 3.3.3 CLOCK SOURCE SELECTION

---------------------------------         
###PLL锁相环
**作用：利用外部输入的参考信号(FIN)控制锁相环路内部的频率和相位，最终输出等比例的频率与相位**
**意义：外部时钟信号只有一个，而系统内部需要多个不同频率的信号，因此需要分频/倍频时钟信号并保证相位同步，因此需要PLL锁相环电路。**

* 输入：外部参考信号(可以经过Pre-Divider预分频)
* 输出: 经过锁相环锁频锁相的信号(可以通过分频配置来调节频率)
* 负反馈系统, 输入输出信号经过PFD输出反馈信号，经过VCO进一步负反馈调节输出信号;

**PFD: Phase Frequency Detector, 鉴频鉴相器.**
一种能将两种输入信号的相位进行比较并输出频率误差和相位误差信息的设备。
有两个不同的信号。是锁相环电路中的一个关键部件。
在锁相环中，输入信号通常一个来自压控振荡器(VCO)，另一个由外部信号提供(FIN)，
两个信号经过PFD比较，输出频率误差和相位误差信息，这些输出信息反馈将控制VCO改变输出频率，
最终使得VCO输出的频率经过分频后与外部信号相同，即VCO输出的信号是外部信号的倍频，即实现了倍频功能。

PHASE LOCKED LOOP (PLL)  
1. The Voltage Controlled Oscillator (VCO) generates the output frequency proportional to input DC voltage.
2. The pre-divider divides the input frequency (FIN) by P. 
3. The main divider divides the VCO output frequency by M, which is input to Phase Frequency Detector (PFD).
4. The phase difference detector calculates the phase difference and the charge pump increases / decreases the output voltage.   
5. The post scaler divides the VCO output frequency by S.


1. VCO(压控振荡器)可以根据输入的直流电压产生对应比例频率的周期信号;
2. Pre-Divider(预分频器)根据P[5:0]对FIN(PLL输入信号)进行分频, 分频后的信号作为PDF的输入信号之一;
3. VCO输出信号经过Main-Divider(M[9:0]配置)分频后, 作为PFD的输入信号之一;
4. PFD输出两个输入信号的误差, 经过Charge Pump计算出应该给VCO的电压值，然后调节电压，更改VCO输出频率；通过不断反馈调节，最终实现倍频；
5. VCO输入的倍频信号经过scaler分频(S[2:0]), 作为外部的输出信号FOUT信号源之一;

参考: S3C6410X.pdf - Page 124 - 3.3.4 PHASE LOCKED LOOP (PLL)

---------------------------------         
####锁相环电路输出信号选择

S3C6410有三个如上的锁相环电路, 分别为APLL, MPLL, EPLL;   
每个PLL锁相环电路的外部参考信号均为外部时钟源;   
CLK_SRC选择PLL的输出, 为0表示选择外部时钟源，即不使用PLL后的信号;为1表示选择PLL输出信号;    

* APLL - CLK_SRC[0]
* MPLL - CLK_SRC[1]
* EPLL - CLK_SRC[2]

---------------------------------         
####各个时钟的频率限制:

PLL输出信号，还可以进一步通过其他寄存器来配置分频，最终作为ARM芯片内的各种设备所需的时钟信号;    
* APLL -> ARMCLK for CPU, S3C6410X - 适用ARM1176架构, 最高频率667MHZ
* MPLL -> 
		0. HCLKX2 for DDR controllers(DDR0, DDR1), maximux 266MHX; 
		1. HCLK for AXI / AHB bus, maximux 133MHZ; 
		2. PCLK for APB bus, maximux 66MHZ;
* EPLL -> 

以上HCLKX2、HCLK、PCLK，均可以独立关闭，从而停止时钟驱动，降低功耗;
* HCLK_GATE
* PCLK_GATE
* SCLK_GATE

而ARMCLK要驱动CPU，不可关闭;

---------------------------------         
####AHB Clock & APB Clock

As described, the frequency ratio between AHB clock and APB clock must be an even integer value.   
For example, if DIVHCLK has 1 of CLK_DIV0[8], then DIVPCLK must be 1, 3, ... of CLK_DIV0[15:12].   
Otherwise, the IPs on APB bus system cannot transfer data correctly.    

即两频率比值必须为偶数关系, 否则APB总线系统的器件将无法正常转换数据;


---------------------------------         

AHB新增两个时钟CLKJPEG/CLKSECUR;

JPEG and security sub-system on AHB bus system cannot be running at 133MHz.    
AHB clocks are independently generated with DIVCLKJPEG and DIVCLKSECUR.

---------------------------------         
###SYNCHRONOUS 667MHZ OPERATING MODE

The clock ratio between ARMCLK and HCLK must be integer to use synchronous interface between ARM core and AXI bus interface.   
S3C6410X does not have any limitation up to 533MHz synchronous interface,    
i.e.,ARMCLK = 533MHz, HCLKX2= 266MHz, HCLK = 133MHz.    
However, there is some constraints over 533MHz, typically 667MHz interface.     
The supported clock ratio is only 1:2.5:5 (ARMCLK = 667MHz, HCLKX2=266MHz, HCLK = 133MHz)   

---------------------------------         


