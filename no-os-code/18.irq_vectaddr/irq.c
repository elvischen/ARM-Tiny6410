#include "stdio.h"
#define GPNCON     (*((volatile unsigned long *)0x7F008830))
#define GPNDAT     (*((volatile unsigned long *)0x7F008834))
#define EINT0CON0  (*((volatile unsigned long *)0x7F008900))
#define EINT0MASK  (*((volatile unsigned long *)0x7F008920))
#define EINT0PEND  (*((volatile unsigned long *)0x7F008924))
#define PRIORITY    (*((volatile unsigned long *)0x7F008280))
#define SERVICE     (*((volatile unsigned long *)0x7F008284))
#define SERVICEPEND (*((volatile unsigned long *)0x7F008288))
#define VIC0IRQSTATUS  (*((volatile unsigned long *)0x71200000))
#define VIC0FIQSTATUS  (*((volatile unsigned long *)0x71200004))
#define VIC0RAWINTR    (*((volatile unsigned long *)0x71200008))
#define VIC0INTSELECT  (*((volatile unsigned long *)0x7120000c))
#define VIC0INTENABLE  (*((volatile unsigned long *)0x71200010))
#define VIC0INTENCLEAR (*((volatile unsigned long *)0x71200014))
#define VIC0PROTECTION (*((volatile unsigned long *)0x71200020))
#define VIC0SWPRIORITYMASK (*((volatile unsigned long *)0x71200024))
#define VIC0PRIORITYDAISY  (*((volatile unsigned long *)0x71200028))

#define VIC0VECTADDR0      (*((volatile unsigned long *)0x71200100))
#define VIC0VECTADDR1      (*((volatile unsigned long *)0x71200104))
#define VIC0ADDRESS        (*((volatile unsigned long *)0x71200f00))

void eint0_3_irq(void)
{
	int i;
	
	printf("eint0_3_irq\n\r");  /* K1~K4 */
	for (i = 0; i < 4; i ++)
	{
		if (EINT0PEND & (1<<i))
		{
			if (GPNDAT & (1<<i))
			{
				printf("K%d released\n\r", i+1);
			}
			else
			{
				printf("K%d pressed\n\r", i+1);
			}
		}
	}

}

void eint4_11_irq(void)
{
	int i;
	printf("eint4_11_irq\n\r"); /* K5~K6 */
	for (i = 4; i < 6; i ++)
	{
		if (EINT0PEND & (1<<i))
		{
			if (GPNDAT & (1<<i))
			{
				printf("K%d released\n\r", i+1);
			}
			else
			{
				printf("K%d pressed\n\r", i+1);
			}
		}
	}

}


void irq_init(void)
{
	/* 配置GPN0~5引脚为中断功能 */
	GPNCON &= ~(0xfff);
	GPNCON |= 0xaaa;

	/* 设置中断触发方式为: 双边沿触发 */
	EINT0CON0 &= ~(0xfff);
	EINT0CON0 |= 0x777;

	/* 禁止屏蔽中断 */
	EINT0MASK &= ~(0x3f);

	/* 在中断控制器里使能这些中断 */
	VIC0INTENABLE |= (0x3); /* bit0: eint0~3, bit1: eint4~11 */ 

	// 设置中断处理函数
	VIC0VECTADDR0 = (unsigned long)eint0_3_irq;
	VIC0VECTADDR1 = (unsigned long)eint4_11_irq;

}


void do_irq(void)
{
	void (*the_isr)(void);

	// 读VICxADDRESS寄存器获得目前正在发生的中断的处理函数
	the_isr = VIC0ADDRESS;

	// 调用中断处理函数
	the_isr();
	
	// 清中断
	EINT0PEND   = 0x3f;
	VIC0ADDRESS = 0;
}


