/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < watchdog.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/10/30 >
    > Last Changed: 
    > Description:
****************************************************************/

#include "watchdog.h"

void irq_init(void)
{
	/* 在中断控制器里使能这些中断 */
	VIC0INTENABLE |= (1<<26);
}

void wtd_operate(unsigned long uenreset, unsigned long uenint, unsigned long uselectclk, unsigned long uenwtd, unsigned long uprescaler,unsigned long uwtdat, unsigned long uwtcnt)
{
	WTDAT = uwtdat;
	WTCNT = uwtcnt;
	
	/*uenreset: 是否使能reset
	  *uenint: 是否使能中断
	  *uselectclk: 分频系数
	  *uenwtd:是否启动定时器
	  *uprescaler:预分频系数
	  */
	WTCON = (uenreset<<0)|(uenint<<2)|(uselectclk<<3)|(uenwtd<<5)|((uprescaler)<<8);
}

//看门狗中断处理函数
void do_irq()
{
	static int i=1;
	printf("Watchdog interrupt  occur %d\r\n",i);

	// 看门狗相关中断清除
	WTCLRINT = 1;

	if(i++==5)
	{
		// 看门狗reset
		printf("waiting system reset\r\n");
		wtd_operate(1,1,0,1,100,100000,100000);
	}
}

