链接器脚本.lds
=======================

对于.lds脚本文件，它定义了整个程序编译之后的链接过程，决定了一个可执行程序的各个段的存储位置。

一个可执行程序通常是由：代码段、数据段、bss段构成的。    
同样，在用于链接这个程序的链接器脚本.lds文件中，就要配置这几个段。

-----------------
### 官方定义

GNU官方网站上对.lds文件形式的完整描述：

	SECTIONS{
	...
	secname start BLOCK(align) (NOLOAD) : AT (ladr)
		{contents} >region :phdr =fill
	...
	}

secname 和 contents 是必须的，其他的都是可选的。
下面挑几个常用的看看：

1. secname：段名
2. contents：决定哪些内容放在本段，可以是整个目标文件，也可以是目标文件中的某段（代
码段、数据段等）
3. start：本段连接（运行）的地址，如果没有使用 AT（ldadr），本段存储的地址也是 start。
GNU 网站上说 start 可以用任意一种描述地址的符号来描述。
4. AT（ldadr）：定义本段存储（加载）的地址。

注意：    
区分start(运行地址)与AT(存储地址)的区别：    
1. 存储地址，表示该段secname的contents应该存储在最后链接文件的哪个地方，AT中的值为最后生成映像文件的偏移地址，
如果不指定AT，则表示直接在上一段的后面存储该段的contents;
2. 运行地址，表示该段secname的contents应该在什么地址运行；如果运行地址与存储地址不在相同的位置，
则需要先从最后链接好的文件中，将该段的contents拷贝到对应的运行地址，然后跳转到该位置执行代码。

例如:

	SECTIONS { 
	  firtst  	0x00000000 : { head.o init.o nand.o}
	  second 	0x30000000 : AT(4096) { main.o }
	} 
 
secname: firtst, contents: 'head.o init.o nand.o', start: 0x00000000;    
secname: second, contents: 'main.o', start: 0x30000000, At: 4096;     

第一段为firtst段，存放head.o/init.o/nand.o，运行地址为0x00000000, 由于没有AT指定存储地址，那么存储地址同运行地址；    
第二段为second段，存放main.o, AT(4096)表示第二段并不直接跟在第一段后面，而是放在整个映像文件偏移0x1000地址的地方。
而运行地址为0x30000000, 则当要执行main.o的程序时，先从映像文件的0x1000地址开始，拷贝main.o的内容至0x30000000，
然后跳转到0x30000000开始执行相应代码；


编写好的.lds文件，在用arm-linux-ld连接命令时, 带-Tfilename来调用执行。    
如 arm-linux-ld -Tnand.lds head.o init.o nand.o main.o -o nand_elf           
也可用-Ttext参数直接指定连接地址如：          
arm-linux-ld -Ttext 0x30000000 head.o init.o nand.o main.o -o nand_elf     

---------------------------------------------

例如：一个uboot.lds链接器脚本

	SECTIONS
	{
		. = 0x00000000;   	 定位当前地址为0地址
		. = ALIGN(4);        代码以四字节对齐
		.text      :         指定代码段.text
		{
			cpu/arm920t/start.o (.text)  代码的第一个代码段
			*(.text)         			 其他代码段
		}
		. = ALIGN(4);
		.rodata : { *(.rodata) }  		 指定只读数据段
		. = ALIGN(4);
		.data : { *(.data) }             指定读写数据段
		. = ALIGN(4);
		.got : { *(.got) }               指定got段，got段式是uboot自定义的一个段，非标准段

		__u_boot_cmd_start = .;          其赋值为当前位置，即起始位置
		.u_boot_cmd : { *(.u_boot_cmd) } u_boot_cmd段，Uboot把所有的uboot命令放在该段。
		__u_boot_cmd_end = .;            把其赋值为当前位置，即结束位置
		. = ALIGN(4);
		__bss_start = .;                 把__bss_start赋值为当前位置，即bss段的开始位置
		.bss : { *(.bss) }               指定bss段
		_end = .;                        把_end赋值为当前位置，即bss段的结束位置
	}

1. 其中用了大量的对齐ALIGN(4)， 即表示接下来要存入映像文件的段，需要保持其在映像文件中的偏移地址是四字节对齐的。     
2. 里面用了不少符号记录当前的位置，例如 __u_boot_cmd_start, __u_boot_cmd_end, __bss_start, _end,
这些符号都是在汇编程序中会用到的。例如在汇编程序进行清零bss段操作时，根据__bss_start/_end就知道bss段的位置。

------------------------------------
