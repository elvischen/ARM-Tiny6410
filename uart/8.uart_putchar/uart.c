/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < uart.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/08/04 >
    > Last Changed: 
    > Description:		FriendlyARM - Tiny6410 裸机程序 - UART
						UART 串口设置之输入输出字符
	UART简介:
	通用异步收发器简称UART，即UNIVERSAL ASYNCHRONOUS RECEIVER AND TRANSMITTER;
	它用来传输串行数据。
	发送数据时，CPU 将并行数据写入UART，UART 按照一定的格式在一根电线上串行发出；
	接收数据时，UART 检测另一根电线的信号，将串行收集在缓冲区中，CPU 即可读取UART获得这些数据

	S3C6410 - UART:
	S3C6410 的 UART 提供了四个独立的异步串行 I / O （SIO）端口。
	每个异步串行 I/O（SIO）端口通过中断或者直接存储器存取（DMA）模式来操作(在 CPU 和 UART 之间传输数据);
	该 UART 使用系统时钟的时间可以支持的比特率最高为 115.2kb/s。
	每个UART 的通道包含了两个 64 字节收发 FIFO 存储器。

	数据传输:
	UART 使用标准的TTL/CMCOS 逻辑电平来表示数据;
	为了增强数据抗干扰能力和提高传输长度(ARM-UART -> RS232 -> PC)，通常将TTL/CMOS 逻辑电平转换为RS-232 逻辑电平;
	
	适用UART0，连线：
	tinyADK(1312B) - COM0 -> 串口线 -> PC

	RXD0 - XuRXD0/GPA0
	TXD0 - XuTXD0/GPA1
****************************************************************/

// 功能:初始化串口
#include "uart.h" 

#define ULCON0     (*((volatile unsigned long *)0x7F005000))
#define UCON0      (*((volatile unsigned long *)0x7F005004))
#define UFCON0     (*((volatile unsigned long *)0x7F005008))
#define UMCON0     (*((volatile unsigned long *)0x7F00500C))
#define UTRSTAT0   (*((volatile unsigned long *)0x7F005010))
#define UFSTAT0    (*((volatile unsigned long *)0x7F005018))
#define UTXH0      (*((volatile unsigned char *)0x7F005020))
#define URXH0      (*((volatile unsigned char *)0x7F005024))
#define UBRDIV0    (*((volatile unsigned short *)0x7F005028))
#define UDIVSLOT0  (*((volatile unsigned short *)0x7F00502C))
#define GPACON     (*((volatile unsigned long *)0x7F008000))


void init_uart(void)
{
	/* 1. 配置引脚 */
	//RXD0 - XuRXD0/GPA0
	//TXD0 - XuTXD0/GPA1
	// GPACON[3:0] - GPA0 - 0010 B, UART RXD[0];
	// GPACON[7:4] - GPA1 - 0010 B, UART TXD[0];
	GPACON &= ~0xff;
	GPACON |= 0x22;		// 0010 0010 B;
	
	/* 2. 设置数据格式等 */
	ULCON0 = 0x3;  					// 数据位:8, 无校验, 停止位: 1, 8n1 
	UCON0  = 0x5;  					// 时钟：PCLK，禁止中断，使能UART发送、接收 
	UFCON0 = 0x01; 					// FIFO ENABLE
	UMCON0 = 0;						// 无流控
	
	/* 3. 设置波特率 */
	// DIV_VAL = (PCLK / (bps x 16 ) ) - 1 = (66500000/(115200x16))-1 = 35.08
	// DIV_VAL = 35.08 = UBRDIVn + (num of 1’s in UDIVSLOTn)/16 
	UBRDIV0   = 35;
	UDIVSLOT0 = 0x1;
}

/* 接收一个字符 */
char getchar(void)
{
	while ((UFSTAT0 & 0x7f) == 0);  // 如果RX FIFO空，等待 
	// RX FIFO接收到数据, 跳出循环;
	return URXH0;                   // 取数据 
}

/* 发送一个字符 */
void putchar(char c)
{
	while (UFSTAT0 & (1<<14)); 		// 如果TX FIFO满，等待 
	// TX FIFO未满, 则跳出循环, 将输入写入UTXH0;
	UTXH0 = c;                      // 写数据 
}
