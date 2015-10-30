/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < main.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/09/27 >
    > Last Changed: 
    > Description:		Frindly ARM - Tiny6410 裸机程序
						UART 串口通信
****************************************************************/

#include "uart.h"
#include "leds.h"

int main()
{
	char data;
	
	leds_init();
	//开始默认关闭LED灯
	leds_OFF_all();

	// 串口初始化, 在start.S中已经调用过
	UART0_Port_Init();
	UART0_Set_BaudRate();
	UART0_Set_Format();

	
	// 测试看门狗的定时功能
	//wtd_operate(0,1,0,1,100,100000,100000);

	// '\n' 换行
	// '\r' 回车
	UART0_SendString("\n\rHello World!\n\r");
	//UART0_SendString("Hello World!\n");

	while(1)
	{
		data = UART0_GetChar();
		UART0_PutChar(data);
	}
    return 0;
}
