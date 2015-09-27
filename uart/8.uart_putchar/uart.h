/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < uart.h >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/09/27 >
    > Last Changed: 
    > Description:		ARM-Friendly Tiny6410 UART
****************************************************************/


#ifndef __uart__
#define __uart__

// UART0 通道引脚配置为UART功能 
void UART0_Port_Init();

// 配置波特率
void UART0_Set_BaudRate();

// 配置传输格式
void UART0_Set_Format();

//发送字符
void UART0_PutChar();

//接收字符
void UART0_GetChar();

#endif
