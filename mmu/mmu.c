/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < mmu.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/10/08 >
    > Last Changed: 
    > Description:		Friendly ARM - Tiny6410 - MMU配置
****************************************************************/

// Translation Table, 分配一块内存(可自由分配), 用作地址映射, 
// Translation Table Base (TTB), 转换表首地址, 要储存到协处理器C15的Control Register 2(页表基址寄存器)
// TTB的地址存放在页表基址寄存器的[31:14]; [14:0]默认为0;
#define MMU_BASE  0x51000000
// Control Register 2 : 0101 0001 0000 0000 0000 0000 0000 0000
//						XXXX XXXX XXXX XXXX XX00 0000 0000 0000
// No.1		0x5100_0000 
// No.2		0x5100_0004 0101 0001 0000 0000 0000 0000 0000 0100
// No.3		0x5100_0008 0101 0001 0000 0000 0000 0000 0000 1000
// No.4		0x5100_000C 0101 0001 0000 0000 0000 0000 0000 1100
// ....		........... 0101 0001 0000 0000 00.. .... .... ..00 	([1:0] = 2b00, 地址为4字节对齐)
// No.4096	0x5100_3FFC	0101 0001 0000 0000 0011 1111 1111 1100
//
// TT共有4096(4k)个页表条目, 一个条目4字节/32位; 
// 整个一级页表共占4K x 4Bytes = 16K Bytes空间; Address: 0x5100_0000 ~ 0x5100_3FFF;
// TT(一级页表), 有四种类型的条目(描述符格式):
// 					1. 数据[1:0] = 0b00; Fault, 无效描述符;
//					2. 数据[1:0] = 0b01; Coarse Page Table, 粗页表描述符;
//					3. 数据[1:0] = 0b10; Section Table, 段描述符;
//					4. 数据[1:0] = 0b11; Find Page Table, 细页表描述符;
// 注: 数据[1:0]不是地址[1:0], 地址[1:0]都是0b00(必须4字节对齐);

// 一级页表描述格式 - 段(Section)
// [31:20], Section Base Address, 段表首地址;
// [11:10], AP, 权限检查模式;
#define MMU_SECTION_FULL_ACCESS       (3 << 10)		// AP=0b11, 访问权限:所有模式下允许任何读写访问
// [8:5], Domain, 权限检查域;
#define MMU_SECTION_DOMAIN            (0 << 5)		// 配置为域0 (由CP15-C3寄存器划分16个权限域) 
// [4], 1, 固定为bit 1;
#define MMU_SECTION_SPECIAL           (1 << 4)		// bit 4必须是1
// [3], Ctt, 是否可缓存; 1, Cached; 0, Non-Cached;
#define MMU_SECTION_CACHEABLE         (1 << 3)		// Cached   
// [2], Btt, 写模式;   Ctt=1, Btt=0, 缓存且写穿式(Write Through); Ctt=1, Btt=1, 缓存且回写式(Write Back);
#define MMU_SECTION_BUFFERABLE        (1 << 2)   	// 使能write buffer
// [1:0], 描述符标识, 段描述符为0b10;
#define MMU_SCETION_SECDESC           (2 << 0)      // 段描述符 0b10

// Ctt=1, Btt=1, Cached & Write Back;
#define MMU_SECTION_WB		(MMU_SECTION_FULL_ACCESS | MMU_SECTION_DOMAIN | MMU_SECTION_SPECIAL | \
	   						MMU_SECTION_CACHEABLE | MMU_SECTION_BUFFERABLE | \
							MMU_SCETION_SECDESC)

// Ctt=0, Btt=0, Non-Cached & Non-Buffered;
#define MMU_SECTION_NCNB	(MMU_SECTION_FULL_ACCESS | MMU_SECTION_DOMAIN | MMU_SECTION_SPECIAL | \
							MMU_SCETION_SECDESC)

// Data Cache / Instruction Cache ENABLE
#define ENABLE_DCACHE_ICACHE  0
#define REGISTER_USE_CACHE    0

// 初始化mmu
void mmu_init(void)
{
	// Translation Table Base = 0x5100_0000
	volatile unsigned long *table = (volatile unsigned long *)MMU_BASE;
	// table[table_index] = 0x5100_0000 | table_index;
	// table_index必须4字节对齐, 即[1:0] = 0b00; eg, 0x4, 0x8, 0xC, 0x10, 0x14, 0x18...
	unsigned long virtul_addr, physical_addr;

	// ---- 1. 初始化页表 ----

	// 方法1:
	//table[0x100]   = 0x7f000000 | MMU_SECTION_NCNB;  // MVA: 0x10000000 => PA: 0x7f000000 , 寄存器地址, 因此不缓存 NCNB
	// 0x7F00_0000 ~ 0x7F10_0000 => 0x1000_0000 ~ 0x1010_0000
	// 方法2:
	virtul_addr   = 0x10000000;
	physical_addr = 0x7F000000;
	*(table + (virtul_addr >> 20)) = (physical_addr & 0xFFFC0000) | MMU_SECTION_NCNB;
	// (0x1000_0000 >> 20) == 0x100;

	// Stepping Stone的起始物理地址为0，第一部分程序(Start.S)的起始运行地址就是0 (链接地址为0, 程序相关跳转都与该地址有关)
	// 为了在开启MMU后仍能运行第一部分的程序, 将0～1M的虚拟地址映射到同样的物理地址
	//table[0]     = 0 | MMU_SECTION_WB; 		     // MMU使能(bl mmu_init)前后的代码所在的1M空间, 保持它们的虚拟地址等于物理地址
	virtul_addr   = 0x0;
	physical_addr = 0x0;
	*(table + (virtul_addr >> 20)) = (physical_addr & 0xFFFC0000) | MMU_SECTION_WB;


	//table[0xc00] = 0x50000000 | MMU_SECTION_WB;  // MVA: 0xc0000000 => PA: 0x50000000, SDRAM开头的1M空间 
	//  0x5000_0000 ~ 0x5010_0000 , 1M Bytes, ==>  0xC000_0000 ~ 0xC010_0000;
	virtul_addr   = 0xC0000000;
	physical_addr = 0x50000000;
	*(table + (virtul_addr >> 20)) = (physical_addr & 0xFFFC0000) | MMU_SECTION_WB;

	// ---- 2. 启动MMU ----
	// ARM体系架构与编程; 嵌入汇编：LINUX内核完全注释
	__asm__ (
			"mov    r1, #0\n"
			"mcr    p15, 0, r1, c7, c7, 0\n"    /* 使无效ICaches和DCaches */

			"mcr    p15, 0, r1, c7, c10, 4\n"   /* drain write buffer on v4 */
			"mcr    p15, 0, r1, c8, c7, 0\n"    /* 使无效指令、数据TLB */

			"mcr p15, 0, %0, c2, c0, 0\n" /* write TTB register */
			"mrc p15, 0, r1, c3, c0, 0\n" /* read domain 15:0 access permissions */
			"orr r1, r1, #3\n"            /* domain 0, Accesses are not checked */
			"mcr p15, 0, r1, c3, c0, 0\n" /* write domain 15:0 access permissions */

			// 对于控制寄存器，先读出其值，在这基础上修改特定位, 然后再写入;
			"mrc p15, 0, r1, c1, c0, 0\n" /* Read control register */
			/* 控制寄存器的低16位含义为：.RVI ..RS B... .CAM
			 * [14], RR : Round robin replacement, 表示换出Cache中的条目时使用的算法，
			 *     		  0 = Random replacement；1 = Round robin replacement
			 * [13], V : 表示异常向量表所在的位置，
			 *     		 0 = Low addresses = 0x00000000；1 = High addresses = 0xFFFF0000
			 * [12], I : 0 = 关闭ICaches；1 = 开启ICaches
			 * [9:8],R、S : 用来与页表中的描述符一起确定内存的访问权限
			 * [7],  B : 0 = CPU为小字节序；1 = CPU为大字节序
			 * [2],  C : 0 = 关闭DCaches；1 = 开启DCaches
			 * [1],  A : 0 = 数据访问时不进行地址对齐检查；1 = 数据访问时进行地址对齐检查
			 * [0],  M : 0 = 关闭MMU；1 = 开启MMU
			 */

			/*
			// 寄存器清零
			// .RVI ..RS B... .CAM
			"bic    r0, r0, #0x3000\n"          // ..11 .... .... .... 清除V、I位
			"bic    r0, r0, #0x0300\n"          // .... ..11 .... .... 清除R、S位
			"bic    r0, r0, #0x0087\n"          // .... .... 1... .111 清除B/C/A/M
			*/
#if ENABLE_DCACHE_ICACHE
			"orr r1, r1, #(1<<2)\n"       /* Data cache enable */
			"orr r1, r1, #(1<<12)\n"      /* Instruction cache enable */
			"orr r1, r1, #(1<<14)\n"      /* Round robin replacement */
#endif		
			"orr r1, r1, #(1<<1)\n"       /* 开启对齐检查 */
			"orr r1, r1, #(1<<0)\n"       /* MMU enable */
			"mcr p15, 0, r1, c1, c0, 0\n"     /* write control register */

			// "%0"和"%1"代表指令的操作数，称为占位符
			// 指令模板后面用小括号括起来的是C语言表达式, (table)在第二个":"后面，对应%1;
			// 在每个操作数前面有一个用引号括起来的字符串(例如"r", "r1"),字符串的内容是对该操作数的限制或者说要求。
			// "r" (table), 表示将C语言的table变量传递给占位符%1, "r"表示需要将table与某个通用寄存器相关联，先将table的值读入寄存器，然后在指令中使用相应寄存器，而不是table本身
			// 如果为"=r" (table), 则步进不仅将table与寄存器关联，读入寄存器；还在最后将寄存器的值写如table变量中。"="表示输出。
			:
			: "r" (table)
			: "r1"
			);
}