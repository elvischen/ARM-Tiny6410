Linux Module - LED Driver
====

裸机程序: [https://github.com/SeanXP/ARM-Tiny6410/tree/master/no-os/led/myled]

使用:

1. make(前提是有编译好的linux kernel, 并在Makefile中指定好路径)
2. 拷贝生成的leds_dev.ko文件到ARM
3. `insmod leds_dev.ko`
4. 交叉编译测试程序leds\_dev\_test.c
5. 拷贝至ARM
6. `./leds_dev_test 0 1`, 点亮所有LED灯

其他用法看驱动程序的ioctl()功能;

----

提供了两种不同的leds驱动编写方法
  
*  leds_dev.c    
	字符混杂设备miscdevice驱动编写    
	1. 自动创建设备节点，卸载后自动删除
	2. 代码中只需要调用`misc_register()`即可完成注册。
	3. 代码中只需要调用`misc_deregister()`即可完成卸载;

	
*  char_leds.c    
	标准字符设备驱动编写
	1. 手动创建设备节点，手动删除
	2. 代码中，使用`alloc_chrdev_region()`分配主次设备号，使用`cdev_init()`和`cdev_add()`来注册和添加字符设备
	3. 需要`cdev_del()`注销字符设备对象，需要`unregister_chrdev_region()`释放已分配的设备号。
	
	
----

##Linux驱动程序分类

对于linux的驱动程序来说，主要分为三种：   

1. miscdevice
2. platform_device
3. platform_driver

####混杂设备miscdevice

**在Linux驱动中把无法归类的五花八门的设备定义为混杂设备(用miscdevice结构体表述)。**    
Linux/内核所提供的miscdevice有很强的包容性。如NVRAM，看门狗，DS1286等实时时钟，字符LCD,AMD 768随机数发生器。   
**miscdevice共享一个主设备号MISC\_MAJOR(即10)，但次设备号不同。**

	[root@FriendlyARM/root]# ls -l /dev/ledsdev
	crw-rw----    1 root     root       10,  53 May  4 04:19 /dev/ledsdev


所有的miscdevice设备形成了一个链表，对设备访问时内核根据次设备号查找对应的miscdevice设备，然后调用其file_operations结构中注册的文件操作接口进行操作。   
miscdevice的API实现在drivers/char/misc.c中。 

	struct miscdevice  
	{
		int minor;
		const char *name;
		const struct file_operations *fops;
		struct list_head list;
		struct device *parent;
		struct device *this_device;
		const char *nodename;
		mode_t mode;
	};

minor是这个混杂设备的次设备号，若由系统自动配置，则可以设置为`MISC_DYNANIC_MINOR`，name是设备名；

	static struct miscdevice myfirst_led_dev = {
		.minor			=	 MISC_DYNAMIC_MINOR,
		.name			=	 DEVICE_NAME,
		.fops			=	 &S3C6410_LEDS_FOPS,
	};
	
