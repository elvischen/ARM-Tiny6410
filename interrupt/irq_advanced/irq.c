/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < irq.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/10/26 >
    > Last Changed:		 < 2015/10/28 > 
    > Description:		S3C6410 - Vectored Interrupt Controller ( PL192 )
 ****************************************************************/
#include "irq.h"
#include "key.h"

// {{{ 中断初始化 void irq_init(void)
void irq_init(void)
{
	// 将协处理器P15的寄存器C1中的bit24置1，该比特位为VIC使能位
	__asm__(
			"mrc p15, 0, r0, c1, c0, 0\n"
			"orr r0, r0, #(1 << 24)\n"
			"mcr p15, 0, r0, c1, c0, 0"
		   );
	// 关闭所有中断
	VIC0INTENCLEAR = 0xffffffff;
	// 清空VICxADDR
	VIC0ADDRESS = 0;
	// 中断挂起标志位清零, Each bit is cleared by writing "1";
	EINT0PEND = 0xffffffff;

	// 配置GPN0~3引脚为中断功能(KEY 1~4)
	// Key1, GPN0, External interrupt Group 0, EINT0
	// Key2, GPN1, External interrupt Group 0, EINT1
	// Key3, GPN2, External interrupt Group 0, EINT2
	// Key4, GPN3, External interrupt Group 0, EINT3
	key_interrupt_init();

	// 设置中断触发方式为: 下降沿触发(0b01x)
	// EINT1,0 [2:0], EINT3,2 [6,4]
	EINT0CON0 &= ~(0xff);
	EINT0CON0 |= 0x33;		// 0b0011_0011

	// 硬件滤波配置
	// EINT0FLTCON0, External Interrupt 0(Group0) Filter Control Register 0
	// EINT0, 1, [5:0], Filtering width of EINT0,1; This value is valid when FLTSEL is 1.
	// FLTSEL, [6], Filter Selection for EINT0,1: 0 = delay filter, 1 = digital filter(clock count);
	// FLTEN, [7], Filter Enable for EINT 0,1: 0 = disables, 1 = enabled;
	// [8:13], [14], [15], EINT 2,3;
//	EINT0FLTCON0 |= 0x1010;
		// 0b 1000_0000_1000_0000
	EINT0FLTCON0 |= 0xcfff;
		// 0b 1100_1111_1111_1111

	// 禁止屏蔽中断, 1:屏蔽中断, 0:不屏蔽
	EINT0MASK &= ~(0xf);	// 0b0000
	// 设置中断类型
	VIC0INTSELECT &= ~(0x1); // 0, IRQ; 1, FIQ;

	// 设置ISR地址
#ifdef IRQ_HANDLE_ASM
	__asm__(
			"ldr r3, =0x71200100\n"
			"ldr r2, =key_handle_asm\n"
			"str r2, [r3]\n"
			);
#else
	VIC0VECTADDR(0) = (unsigned int)key_irq_handler; 
#endif

	// 在中断控制器里使能这些中断
	// [1], INT_EINT0 : External interrupt Group 0 (EINT0~EINT3)
	VIC0INTENABLE |= (0x1); 
	
	// 设置CPSR的总中断位, [7], I; [6], F; 写1表示禁止中断;
	// 在Start.S中已经开启总中断;
	/*
	__asm__(
			"mrs r0, cpsr\n"
			"bic r0, r0, #(1 << 7)\n"
			"msr cpsr, r0\n"
		   );
	*/
}//}}}
