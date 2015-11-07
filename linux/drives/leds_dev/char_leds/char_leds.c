/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < char_leds.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/11/07 >
    > Last Changed: 
    > Description:		Friendly ARM Tiny6410 - Linux Device Drivers - LED - 标志字符设备驱动程序
						需要手动创建设备节点: 
						# insmod char_leds.ko
						得到分配的设备号, 假设为253, 则创建设备节点
						# mknod /dev/ledsdev c 253 0

						LED I/O 在open()时配置为输出方向, 
						应用程序用过ioctl发送cmd(0,关闭; 1,打开)和arg(LED编号);

		LED1 -> GPK4 -> GPKCON0[19:16] / GPKDAT[4]
		LED2 -> GPK5 -> GPKCON0[23:20] / GPKDAT[5]
		LED3 -> GPK6 -> GPKCON0[27:24] / GPKDAT[6]
		LED4 -> GPK7 -> GPKCON0[31:28] / GPKDAT[7]
		低电平点亮LED灯
****************************************************************/
// {{{ include file
#include <linux/module.h>		// THIS_MODULE, MODULE_AUTHOR, MODULE_LICENSE...
#include <linux/init.h>			// module_init, module_exit 
#include <linux/fs.h>			// struct file_operations; 系统调用与驱动程序的映射;
								// open(), read(), write(), ioctl() ...
								// 应用程序打开与驱动程序挂钩的设备节点文件，对其进行系统调用，
								// 最终都会执行到file_operations中对应的函数;
#include <asm/io.h>				// ioread32,iowrite32
// linux/arch/arm/mach-s3c64xx/include/mach/gpio-bank-[a-q].h
#include <mach/gpio-bank-k.h>	// S3C64XX_GPKCON, S3C64XX_GPKDAT
#include <mach/regs-gpio.h>		//定义了gpio-bank-k中使用的S3C64XX_GPK_BASE
#include <mach/map.h>			//定义了S3C64XX_VA_GPIO 
#include<linux/cdev.h>			//包含字符驱动设备struct cdev的定义;
// }}}

// {{{ Device Name 
// 加载模式后，执行"cat /proc/devices"命令看到的设备名称
#define DEVICE_NAME		"ledsdev"
// 设备编号, 为0时会自动分配
#define DEVICE_MAJOR	0
//}}} 

// {{{ cdev
// 每个字符设备都对应一个cdev结构体变量.
// cdev一般它有两种定义初始化方式:静态的和动态的。
// 静态内存定义初始化：
//		struct cdev my_cdev;				// 创建cdev结构体变量
//		cdev_init(&my_cdev, &fops);			// 绑定file_operations
//		my_cdev.owner = THIS_MODULE;		// 配置owner
//动态内存定义初始化:
//		struct cdev *my_cdev = cdev_alloc();
//		my_cdev->ops = &fops;
//		my_cdev->owner = THIS_MODULE;
// 两种使用方式的功能是一样的，只是使用的内存区不一样，一般视实际的数据结构需求而定。
//
/********************** <linux-2.6.38/include/linux/cdev.h> *******************
   strut cdev{
		struct kobject kobj;				//内嵌的内核对象
		struct module *owner;				//指向内核模块对象的指针
		const struct file_operations *ops;	//应用程序调用操作系统接口呼叫到设备驱动程序中相应操作
		struct list_head list;				//链表
		dev_t dev;							//字符设备的设备号
		unsigned int count;					//次设备号的个数
   };
**********************************************************/
static struct cdev leds_cdev;			//定义一个字符设备对象
static dev_t leds_dev_t;				//字符设备节点的设备号
// }}}

// {{{ open
// int (*open)(struct inode *inode, struct file *filp)
// open方法提供给驱动程序以初始化的能力，从而为以后的操作完成初始化做准备。
// 初始化字符设备 - LED IO端口
// 全局变量, 为0表示未初始化; 1表示已初始化;
static int leds_status = 0;
static int s3c6410_leds_open(struct inode *nd, struct file *flip)
{
	unsigned long tmp;
	
	if(leds_status == 0)
	{
		//配置LED IO为输出
		tmp = ioread32(S3C64XX_GPKCON);
		tmp &= ~(0xFFFF0000);	// 清零GPKCON0[16:31]位
		tmp |= (0x11110000);	// 设置为输出
		iowrite32(tmp,S3C64XX_GPKCON);//写入GPKCON

		//熄灭LED灯
		tmp = readl(S3C64XX_GPKDAT);	// 旧的io读写, 不推荐使用; readb, readw, readl, writeb, writew, writel.
		tmp |= (0xF<<4);		//关闭LED灯
		iowrite32(tmp,S3C64XX_GPKDAT);
		leds_status = 1;
	}
	
	printk(DEVICE_NAME ", open char device.\n");
	return 0;
} // }}}

// {{{ close
static int s3c6410_leds_close(struct inode *nd, struct file *flip)
{
	printk(DEVICE_NAME ", close char device.\n");
	return 0;
} // }}}

// {{{ ioctl
// int ioctl(int fd,unsigned long cmd,...);
// ioctl是设备驱动程序中对设备的I/O通道进行管理的函数
// 用户程序通过命令码(cmd)告诉驱动程序想做什么, 至于怎么解释这些命令和怎么实现这些命令,这都是驱动程序要做的事情
// 参数:
//	cmd = 0, 关闭arg指定的LED灯;
//	cmd = 1, 打开arg指定的LED灯;
//	arg, (0~4), 其中0表示所有的LED灯, 1~4分别表示LED1~LED4
// 返回: 
// 	返回0或者错误-EINVAL
static int s3c6410_leds_ioctl(struct file*filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd){
		unsigned tmp;
		case 0://close, 这里close与open在同一段代码里处理; switch结构还要有, 用来扩展以后的case 2, case 3 ...
		case 1://open
		{
			// 判断参数arg
			if(arg > 4)
			{
				//参数错误
				return -EINVAL;		
			}
			else if(arg == 0) // 处理所有的LED灯
			{
				// LED1~4 -> GPKDAT[4:7]
				tmp = ioread32(S3C64XX_GPKDAT);		//读出LED1~LED4所在寄存器的值
				if(cmd == 1)
					tmp &= ~(0xF<<4);				//输出低电平, 点亮LED1~LED4
				else if(cmd == 0)					//若是关闭，则关闭掉LED1~LED4
					tmp |= (0xF<<4);				//输出高电平，熄灭LED1~LED4
				// 将改变的值重新写入对应寄存器
				iowrite32(tmp, S3C64XX_GPKDAT);
			}
			else
			{	
				// 处理单个LED灯
				tmp = ioread32(S3C64XX_GPKDAT);
				// arg = 1, LED1 -> GPKDAT4;
				// arg = 2, LED2 -> GPKDAT5;
				// arg = 3, LED3 -> GPKDAT6;
				// arg = 4, LED4 -> GPKDAT7;
				// arg = n, GPKDAT(3 + arg)
				tmp &= ~(1 << (3+arg));				//清除arg所对应的那一位值
				tmp |= ((!cmd) << (3+arg));			//写入新值, cmd=1, 写入0; cmd=0, 写入1;
				iowrite32(tmp, S3C64XX_GPKDAT);
			}
			return 0;
		}
		default:
			return -EINVAL;
	}
	return 0; //按照程序跳转逻辑, 应该不会执行, 这里只是为了对应static int返回值, 避免编译器报警告;
} // }}}

// {{{ file_operations
static struct file_operations S3C6410_LEDS_FOPS = 
{
										// #define THIS_MODULE (&__this_module); __this_module在编译模块时自动创建;
	.owner  =   THIS_MODULE,    		// 这是一个宏，推向编译模块时自动创建的__this_module变量,
	// 驱动函数映射
	.open = s3c6410_leds_open,					// 对于字符设备, open()为初始化;
	//.close = s3c6410_leds_close,			
	.release = s3c6410_leds_close,				// 对于字符设备, close()为关闭设备;
	.unlocked_ioctl  = 	s3c6410_leds_ioctl,		// ioctl()为命令控制;
	//	.ioctl = s3c6410_leds_ioctl,		//linux kernel 3.x 版本的内核删除了ioctl函数
	//	取代的是: long (*unlocked_ioctl) (struct file*, unsigned int, unsigned long);
	//			  long (*compat_ioctl) (struct file*, unsigned int, unsigned long);
};
// }}}

// {{{ module init
// 执行insmod命令时就会调用这个函数 
// static int __init init_module(void) 	//默认驱动初始化函数,不用默认函数,就要添加宏module_init()
// __init 宏, 只有静态链接驱动到内核时才有用, 表示将此函数代码放在".init.text"段中; 使用一次后释放这段内存;
static int __init s3c6410_leds_init(void)
{
	//在设备驱动程序注册的时候, 初始化LED1～LED4所对应的GPIO管脚为输出, 并熄灭LED1~LED4
	int ret; 		//用于存放函数调用返回值的变量

	printk(DEVICE_NAME ", char device init.\n");

	if(DEVICE_MAJOR)	// 设备号不为0, 注册指定设备编号
	{
		// MKDEV(int major,int minor) //通过主次设备号来生成dev_t类型的设备号
		leds_dev_t = MKDEV(DEVICE_MAJOR, 0);
		// int register_chrdev_region(dev_t from, unsigned count, const char *name) - register a range of device numbers
		ret = register_chrdev_region(leds_dev_t, 1, DEVICE_NAME);	//注册设备编号
		// 内核提供了三个函数来注册一组字符设备编号
		// register_chrdev_region()
		// alloc_chrdev_region()
		// register_chrdev()
		// 这三个函数都会调用一个共用的__register_chrdev_region() 函数来注册一组设备编号范围
		if(ret < 0)		// 注册失败
		{
			printk(KERN_WARNING "register device numbers error.\n");  
			return ret;  
		}
	}
	else				// 设备号为0，自动分配设备号并注册
	{
		//自动分配设备号
		// int alloc_chrdev_region(dev_t *dev,unsigned int firstminor,unsigned int count,char *name);
		// firstminor是请求的最小的次编号
		// count是请求的连续设备编号的总数
		// name为设备名
		// 返回值小于0表示分配失败
		int leds_major = 0;
		ret = alloc_chrdev_region(&leds_dev_t, 0, 1, DEVICE_NAME);  
		if(ret < 0)
		{  
			printk(KERN_WARNING "alloc char device number error\n");  
			return ret;  
		}
		leds_major = MAJOR(leds_dev_t);//MAJOR(dev) 获得主设备号  
		printk(KERN_ALERT "leds device major = %d.\n", leds_major);  
	}

	// 已静态创建cdev结构体变量leds_cdev
	//void cdev_init(struct cdev*cdev,struct file_operations *fops)
	cdev_init(&leds_cdev, &S3C6410_LEDS_FOPS);
	//int cdev_add(struct cdev *dev,dev_t num,unsigned int count)  
	cdev_add(&leds_cdev, leds_dev_t, 1); 

	// 第三种绑定设备号的方法, 直接绑定file_operations
	// register_chrdev(), 注册字符设备, 参数为主设备号, 设备名字, file_operations结构
	// 这样, 主设备号就和具体的file_operations结构关联起来
	// 操作主设备为LED_MAJOR的设备文件时，就会调用对应file_operations中的相关成员函数
	// 主设备号可以设为0，表示由内核自动分配主设备号
	//ret = register_chrdev(LED_MAJOR, DEVICE_NAME, &S3C6410_LEDS_FOPS);

	return 0;
} //}}}

//{{{ module exit
// 执行rmmod命令时就会调用这个函数 
// static int __exit cleanup_module(void) 	//默认驱动清除函数
// __exit 宏, 表示将此函数代码放在".exit.data"段中; 静态链接时没有使用,因为静态链接的驱动不能被卸载
static void __exit s3c6410_leds_exit(void)
{
	// 卸载驱动程序
	printk(DEVICE_NAME " module exit.\n");
	cdev_del(&leds_cdev);						//注销chr_dev对应的字符设备对象
	unregister_chrdev_region(leds_dev_t, 1);		//释放分配的设备号
} //}}}

// 这两行指定驱动程序的初始化函数和卸载函数
module_init(s3c6410_leds_init);
module_exit(s3c6410_leds_exit);
// {{{ Module Description
// 描述驱动程序的一些信息，不是必须的
MODULE_VERSION("0.0.1");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SeanXP#iseanxp+code@gmail.com");
MODULE_DESCRIPTION("Tiny6410 LED Driver");
// }}}
