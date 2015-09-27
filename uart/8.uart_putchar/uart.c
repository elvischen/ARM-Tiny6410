/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < uart.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/09/27 >
    > Last Changed: 
    > Description:		FriendlyARM - Tiny6410 裸机程序 - UART
						UART 串口设置之输入输出字符

	适用UART0，连线：
	tinyADK(1312B) - COM0 -> 串口线 -> PC

****************************************************************/

#include "uart.h" 

#define UART0_CLK			66500000	// PCLK, 66.5 MHZ
#define UART0_BAUDRATE		115200		// Baud Rate	
#define UART0_DIV_VAL		( (UART0_CLK) / ((UART0_BAUDRATE) * 16) - 1)
#define UART0_UBRDIV0		(int)(UART0_DIV_VAL)
#define UART0_UDIVSLOT_NUM	(int)((UART0_DIV_VAL - UART0_UBRDIV0) * 16) 

// S3C6410X.pdf - Pages 1101
const int UDIVSLOT_LIST[16] = 
{
	0x0000, 0x0080, 0x0808, 0x0888,
	0x2222, 0x4924, 0x4A52, 0x54AA,
	0x5555, 0xD555, 0xD5D5, 0xDDD5,
	0xDDDD, 0xDFDD, 0xDFDF, 0xFFDF
}

// DIV_VAL = UBRDIVn + (num of 1’s in UDIVSLOTn)/16
// DIV_VAL = (PCLK / (bps x 16 ) ) - 1

//{{{ BIT位掩码宏定义
#define	BIT0		(1<<0)
#define	BIT1		(1<<1)
#define	BIT2		(1<<2)
#define	BIT3		(1<<3)
#define	BIT4		(1<<4)
#define	BIT5		(1<<5)
#define	BIT6		(1<<6)
#define	BIT7		(1<<7)
#define	BIT8		(1<<8)
#define	BIT9		(1<<9)
#define	BIT10		(1<<10)
#define	BIT11		(1<<11)
#define	BIT12		(1<<12)
#define	BIT13		(1<<13)
#define	BIT14		(1<<14)
#define	BIT15		(1<<15)

#define GPACON     (*((volatile unsigned long *)0x7F008000))
// 0010 = UART
#define GPA0_UART		(2<<(0*4))
#define GPA1_UART		(2<<(1*4))
//GPACON的位掩码
#define GPA0_mask		(0xf<<(0*4))
#define GPA1_mask		(0xf<<(1*4))
// }}}

//{{{ UART0 寄存器宏定义
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
// }}}

//{{{ UART0 通道引脚配置为UART功能 
void UART0_Port_Init()
{
	// RXD0 - XuRXD0/GPA0
	// TXD0 - XuTXD0/GPA1
	// GPACON[3:0] - GPA0 - 0010 B, UART RXD[0];
	// GPACON[7:4] - GPA1 - 0010 B, UART TXD[0];
	GPACON &= ~(GPA0_mask + GPA1_mask);
	GPACON |= (GPA0_UART + GPA1_UART);		// 0010 0010 B;
}	
//}}}

//{{{ UART0 - 配置波特率
void UART0_Set_BaudRate()
{
	// DIV_VAL = (PCLK / (bps x 16 ) ) - 1 = (66500000/(115200x16))-1 = 35.08
	// DIV_VAL = 35.08 = UBRDIVn + (num of 1’s in UDIVSLOTn)/16 
	UBRDIV0   = UART0_UBRDIV0;
	UDIVSLOT0 = UDIVSLOT_LIST[UART0_UDIVSLOT_NUM];
}
//}}}

//{{{ UART0 - 配置传输格式
void UART0_Set_Format();
{
	// UART LINE CONTROL REGISTER - ULCON0
	ULCON0 = 0x3;  					// 数据位:8, 无校验, 停止位: 1, 8n1 
	
	UCON0  = 0x5;  					// 时钟：PCLK，禁止中断，使能UART发送、接收 
	UFCON0 = 0x01; 					// FIFO ENABLE
	UMCON0 = 0;						// 无流控
}
//}}}

//{{{ UART0 - 发送字符
void UART0_PutChar(char data);
{
	while (UFSTAT0 & (1<<14)); 		// 如果TX FIFO满，等待 
	// TX FIFO未满, 则跳出循环, 将输入写入UTXH0;
	UTXH0 = data;                      // 写数据 
}
//}}}

//{{{ UART0 - 接收字符
char UART0_GetChar();
{
	while ((UFSTAT0 & 0x7f) == 0);  // 如果RX FIFO空，等待 
	// RX FIFO接收到数据, 跳出循环;
	return URXH0;                   // 取数据 
}
//}}}
