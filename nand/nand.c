/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < nand.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/10/20 >
    > Last Changed: 
    > Description:		S3C6410 Nand Flash Controller - Nand Flash (K9K8G08U0E)
****************************************************************/
#include "nand.h"
// HCLK: clock.c, 133M HZ, (7.5ns)

// TACLS/TWRPH0/TWRPH1 - S3C6410X.pdf - Chater 8 - Pages 3 - NAND FLASH MEMORY TIMING
// K9F4G(K8G)08U0D_0.2.pdf - 4.1 Command Latch Cycle, 时序参数要求;
// 表示CLE/ALE的维持时间(hold time)
// tCLH, CLE Hold Time; tALH, ALE Hold Time; 
// K9K8G08U0E min(tCLH) = min(tALH) = 5ns; 
#define NFCONF_TWRPH1		(0x7)			// Duration = HCLK x (TWRPH1 + 1) = 7.5ns x 8 > 5ns
// 表示CLE/ALE的持续时间, 建立时间tCLS + WE Pulse Width + 维持时间tCLH = 5ns+12ns+12ns=29ns;
// K9K8G08U0E min( tWP ) = 12ns;
#define NFCONF_TWRPH0		(0xf)			// Duration = HCLK x (TWRPH0 + 1) = 7.5ns x 17 > 12ns
// 表示CLT/ALE的建立时间(setup time)
// tCLS, CLE Setup Time; tALS, ALE Setup Time;
// K9K8G08U0E min(tCLS) = min(tALS) = 12ns;
#define NFCONF_TACLS		(0x2)			// Duration = HCLK x TACLS = 7.5ns x 2 > tCLS
// 00: 1-bit ECC, 10: 4-bit ECC, 01 : 8-bit ECC(ERROR CORRECTION CODE); 
// K9K8G08U0E ECC Requirement : 1bit/528Byte (528Byte里面可以检验并校验1bit)
#define NFCONF_ECC_TYPE		(0x0)			// 00, 1-bit ECC;
#define NFCONF_MSGLENGTH	(0x0)
#define NFCONF_MLCCLKCTRL	(0x0)

// {{{ Nand Register Define
// Nand Register Base Address: 0x7020_0000
// 其余Nand Register都是在Base address上的偏移;
// S3C6410X.pdf - Chapter 8 - Page 13
#define ELFIN_NAND_BASE		0x70200000

#define NFCONF_OFFSET           0x00	// Configuration register
#define NFCONT_OFFSET           0x04	// Control register
#define NFCMMD_OFFSET           0x08	// Command register
#define NFADDR_OFFSET           0x0c	// Address register
#define NFDATA_OFFSET			0x10	// Data register
#define NFMECCDATA0_OFFSET      0x14	// 1st and 2nd main ECC data register
#define NFMECCDATA1_OFFSET      0x18	// 3rd and 4th main ECC data register
#define NFSECCDATA0_OFFSET      0x1c	// Spare ECC read register
#define NFSBLK_OFFSET           0x20	// Programmable start block address register
#define NFEBLK_OFFSET           0x24	// Programmable end block address register
#define NFSTAT_OFFSET           0x28	// NAND status registet
#define NFESTAT0_OFFSET         0x2c	// ECC error status0 register
#define NFESTAT1_OFFSET         0x30	// ECC error status1 register
#define NFMECC0_OFFSET          0x34	// Generated ECC status0 register
#define NFMECC1_OFFSET          0x38	// Generated ECC status1 register
#define NFSECC_OFFSET           0x3c	// Generated Spare area ECC status register
#define NFMLCBITPT_OFFSET       0x40	// 4-bit ECC error bit pattern register
#define NF8ECCERR0_OFFSET		0x44	// 8bit ECC error status0 register
#define NF8ECCERR1_OFFSET		0x48	// 8bit ECC error status1 register
#define NF8ECCERR2_OFFSET		0x4c	// 8bit ECC error status2 register
#define NFM8ECC0_OFFSET			0x50	// Generated 8-bit ECC status0 register
#define NFM8ECC1_OFFSET			0x54	// Generated 8-bit ECC status1 register
#define NFM8ECC2_OFFSET			0x58	// Generated 8-bit ECC status2 register
#define NFM8ECC3_OFFSET			0x5c	// Generated 8-bit ECC status3 register
#define NFMLC8BITPT0_OFFSET		0x60	// 8-bit ECC error bit pattern 0 register
#define NFMLC8BITPT1_OFFSET		0x64	// 8-bit ECC error bit pattern 1 register

// NAND Flash Configuration register
#define NFCONF				(ELFIN_NAND_BASE+NFCONF_OFFSET)
#define NFCONT				(ELFIN_NAND_BASE+NFCONT_OFFSET)
#define NFCMMD				(ELFIN_NAND_BASE+NFCMMD_OFFSET)
#define NFADDR           	(ELFIN_NAND_BASE+NFADDR_OFFSET)
#define NFDATA          	(ELFIN_NAND_BASE+NFDATA_OFFSET)
#define NFMECCDATA0     	(ELFIN_NAND_BASE+NFMECCDATA0_OFFSET)
#define NFMECCDATA1     	(ELFIN_NAND_BASE+NFMECCDATA1_OFFSET)
#define NFSECCDATA0      	(ELFIN_NAND_BASE+NFSECCDATA0_OFFSET)
#define NFSBLK          	(ELFIN_NAND_BASE+NFSBLK_OFFSET)
#define NFEBLK           	(ELFIN_NAND_BASE+NFEBLK_OFFSET)
#define NFSTAT           	(ELFIN_NAND_BASE+NFSTAT_OFFSET)
#define NFESTAT0         	(ELFIN_NAND_BASE+NFESTAT0_OFFSET)
#define NFESTAT1         	(ELFIN_NAND_BASE+NFESTAT1_OFFSET)
#define NFMECC0          	(ELFIN_NAND_BASE+NFMECC0_OFFSET)
#define NFMECC1          	(ELFIN_NAND_BASE+NFMECC1_OFFSET)
#define NFSECC           	(ELFIN_NAND_BASE+NFSECC_OFFSET)
#define NFMLCBITPT          (ELFIN_NAND_BASE+NFMLCBITPT_OFFSET)
#define NF8ECCERR0			(ELFIN_NAND_BASE+NF8ECCERR0_OFFSET)
#define NF8ECCERR1			(ELFIN_NAND_BASE+NF8ECCERR1_OFFSET)
#define NF8ECCERR2			(ELFIN_NAND_BASE+NF8ECCERR2_OFFSET)
#define NFM8ECC0			(ELFIN_NAND_BASE+NFM8ECC0_OFFSET)
#define NFM8ECC1			(ELFIN_NAND_BASE+NFM8ECC1_OFFSET)
#define NFM8ECC2			(ELFIN_NAND_BASE+NFM8ECC2_OFFSET)
#define NFM8ECC3			(ELFIN_NAND_BASE+NFM8ECC3_OFFSET)
#define NFMLC8BITPT0		(ELFIN_NAND_BASE+NFMLC8BITPT0_OFFSET)
#define NFMLC8BITPT1		(ELFIN_NAND_BASE+NFMLC8BITPT1_OFFSET)

#define __REG(x)				(*(volatile unsigned long *)(x))
#define __REGb(x)				(*(volatile unsigned char *)(x))
#define NFCONF_REG				__REG(ELFIN_NAND_BASE+NFCONF_OFFSET)
#define NFCONT_REG				__REG(ELFIN_NAND_BASE+NFCONT_OFFSET)
#define NFCMD_REG				__REG(ELFIN_NAND_BASE+NFCMMD_OFFSET)
#define NFADDR_REG           	__REG(ELFIN_NAND_BASE+NFADDR_OFFSET)
#define NFDATA_REG          	__REG(ELFIN_NAND_BASE+NFDATA_OFFSET)
#define NFDATA8_REG          	__REGb(ELFIN_NAND_BASE+NFDATA_OFFSET)
#define NFMECCDATA0_REG     	__REG(ELFIN_NAND_BASE+NFMECCDATA0_OFFSET)
#define NFMECCDATA1_REG     	__REG(ELFIN_NAND_BASE+NFMECCDATA1_OFFSET)
#define NFSECCDATA0_REG      	__REG(ELFIN_NAND_BASE+NFSECCDATA0_OFFSET)
#define NFSBLK_REG          	__REG(ELFIN_NAND_BASE+NFSBLK_OFFSET)
#define NFEBLK_REG           	__REG(ELFIN_NAND_BASE+NFEBLK_OFFSET)
#define NFSTAT_REG           	__REG(ELFIN_NAND_BASE+NFSTAT_OFFSET)
#define NFESTAT0_REG         	__REG(ELFIN_NAND_BASE+NFESTAT0_OFFSET)
#define NFESTAT1_REG         	__REG(ELFIN_NAND_BASE+NFESTAT1_OFFSET)
#define NFMECC0_REG          	__REG(ELFIN_NAND_BASE+NFMECC0_OFFSET)
#define NFMECC1_REG          	__REG(ELFIN_NAND_BASE+NFMECC1_OFFSET)
#define NFSECC_REG           	__REG(ELFIN_NAND_BASE+NFSECC_OFFSET)
#define NFMLCBITPT_REG         	__REG(ELFIN_NAND_BASE+NFMLCBITPT_OFFSET)
//}}}

#define NAND_DISABLE_CE()	(NFCONT_REG |= (1 << 1))
#define NAND_ENABLE_CE()	(NFCONT_REG &= ~(1 << 1))
// NFSTAT, NAND Flash operation status register; 
// [0], RnB (Read-only), 0: busy, 1: ready;
// 等待Ready信号;
#define NF_TRANSRnB()		do { while(!(NFSTAT_REG & (1 << 0))); } while(0)

// {{{ NAND Flash Commands
// Standard NAND flash commands
#define NAND_CMD_READ			0x0				// Read (00h, 30h)
#define NAND_CMD_READ_COPYBACK	0x0				// Read for Copy Back (00h, 35h)
#define NAND_CMD_RNDOUT			0x5				// Random Data Output ( 05h, E0h)
#define NAND_CMD_PAGEPROG		0x80			// Page Program (80h, 10h)
#define NAND_CMD_WRITE_COPYBACK	0x85			// Copy-Back Program (85h, 10h) 
#define NAND_CMD_ERASE1			0x60			// Block Erase (60h, d0h)
#define NAND_CMD_ERASE2			0xd0			// Block Erase (60h, d0h)
#define NAND_CMD_STATUS			0x70			// Read Status (70h)
#define NAND_CMD_STATUS2		0xf1			// Read Status 2 (f1h)
#define NAND_CMD_RNDIN			0x85			// Random Data Input (85h)
#define NAND_CMD_READID			0x90			// Read ID (90h)
#define NAND_CMD_RESET			0xff			// Reset (ffh)
// Extended commands for large page devices
#define NAND_CMD_READ_START		0x30			// Read (00h, 30h)
#define NAND_CMD_READ_COPYBACK_START 0x35		// Read for Copy Back (00h, 35h)
#define NAND_CMD_PAGEPROG_START	0x10			// Page Program (80h, 10h)
#define NAND_CMD_RNDOUT_START	0xE0			// Random Data Output ( 05h, E0h)
#define NAND_CMD_WRITE_COPYBACK_START	0x10	// Copy-Back Program (85h, 10h) 
// }}}

#define MEM_SYS_CFG     (*((volatile unsigned long *)0x7E00F120))

// 设置NAND Flash控制器
// {{{ void nand_init(void)
void nand_init(void)
{
	// NFCONF, NAND Flash Configuration register
	// [6:4], TWRPH1, TWRPH1 duration setting value (0~7), Duration= HCLKx(TWRPH1+1);
	// [10:8], TWRPH0, TWRPH0 duration setting value (0~7) Duration= HCLKx(TWRPH0+1);
	// [14:12], TACLS, CLE & ALE duration setting value (0~7) Duration = HCLK x TACLS;
	// [24:23], ECCType, 00: 1-bit ECC, 10: 4-bit ECC, 01 : 8-bit ECC;
	// [25], MsgLength, Message (Data) length for 4/8 bit ECC, 0: 512-byte, 1: 24-byte;
	// [30], MLCClkCtrl, Clock control for 4-bit ECC & 8-bit ECC engine.(Hidden Spec.);
	//		 0: Recommended when system clock is more than 66MHz.
	//		 1: Recommended when system clock is less than 66MHz
	NFCONF_REG = ( (NFCONF_MLCCLKCTRL << 30) | (NFCONF_MSGLENGTH << 25) | (NFCONF_ECC_TYPE << 23)|\
				   (NFCONF_TACLS << 12) | (NFCONF_TWRPH0 << 8) | (NFCONF_TWRPH1 << 4));

	// NFCONT, NAND Flash control register
	// [0], MODE, NAND Flash controller operating mode, 0: NAND Flash Controller Disable (Don’t work), 1: NAND Flash Controller Enabl;
	// [1], Reg_nCE0, NAND Flash Memory Xm0CSn2 signal control; 0: Force Xm0CSn2 to low(Enable chip select), 1: Force Xm0CSn2 to High(Disable chip select);
	// [2], Reg_nCE1, NAND Flash Memory Xm0CSn3 signal control;
	// [4], InitSECC, 1: Initialize spare area ECC decoder/encoder (write-only)
	// [5], InitMECC, 1: Initialize main area ECC decoder/encoder (write-only);
	// [6], SpareECCLock, 0: Unlock Spare ECC 1: Lock Spare ECC;
	// [7], MainECCLock, 0: Unlock Main area ECC 1: Lock Main area ECC;
	// ...
	NFCONT_REG |= (0x3<<0); // NAND Flash Controller Enable; Xm0CSn2 signal control;
} // }}}

// Read ID
// {{{ static int nand_read_ID(void);
// K9F4G(K8G)08U0D_0.2.pdf - 4.17 Read ID Operation
static char nand_read_ID(void)
{
	char i = 0; 
	char id[6] = {0,0,0,0,0,0};

	// 发片选
	NAND_ENABLE_CE();

	// 发读命令：0x90
	NFCMD_REG = NAND_CMD_READID;

	// 1st Address 
	NFADDR_REG = 0;

	// Read ID
	for(i = 0; i < 6; i++)
		id[i] = NFDATA8_REG;

	return id[0];
} // }}}

// 读一页，即2048byte
// {{{ static int nandll_read_page (unsigned char *buf, unsigned long addr);
static int nandll_read_page (unsigned char *buf, unsigned long addr)
{

	int i;
	int page_size = 2048;	// 1 Page = (2K + 64) Bytes

	// 发片选
	NAND_ENABLE_CE();

	// 发读命令：0x00
	NFCMD_REG = NAND_CMD_READ;

	// 发地址 ( Column Address (2) + Row Address (3) )
	// 1st Cycle: 列地址低位地址(A0~A7)
	NFADDR_REG = 0;
	// 2st Cycle: 列地址高位地址(A8~A11), 2^11 = 每块每页2k列
	NFADDR_REG = 0;
	// 3st Cycle, 行地址地位地址(A12~A19)
	NFADDR_REG = (addr) & 0xff;
	// 4st Cycle, 行地址中位地址(A20~A27)
	NFADDR_REG = (addr >> 8) & 0xff;
	// 5st Cycle, 行地址高位地址(A28~A29), 2^16 = 每块64K页
	NFADDR_REG = (addr >> 16) & 0xff;

	// 发读命令：0x30
	NFCMD_REG = NAND_CMD_READ_START;

	// 等待数据, Ready信号;
	NF_TRANSRnB();

	// 连续读2048个字节
	for(i = 0; i < page_size; i++)
	{
		*buf++ = NFDATA8_REG;
	}

	// 取消片选
	NAND_DISABLE_CE();

	return 0;
}
// }}}


// 从NAND中拷贝代码到DRAM
// {{{ int copy2ddr(unsigned int nand_start, unsigned int ddr_start, unsigned int len);
int copy2ddr(unsigned int nand_start, unsigned int ddr_start, unsigned int len)
{
	unsigned char *buf = (unsigned char *)ddr_start;
	int i;
	unsigned int page_shift = 11;

	// 发片选
	NAND_ENABLE_CE();

	// 使len为2048的整数倍
	len = (len/2048+1)*2048;

	// 循环拷贝，每次拷贝一页数据
	for (i = 0; i < (len>>page_shift); i++, buf+=(1<<page_shift))
	{
		// 读一页，即2048byte
		nandll_read_page(buf, i);
	}

	return 0;
} // }}}
