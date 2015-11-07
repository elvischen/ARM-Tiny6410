linux module - helloworld.ko
====

	[root@FriendlyARM/]# insmod helloworld.ko
	helloworld: module license 'DUAL BSD/GPL' taints kernel.
	Disabling lock debugging due to kernel taint
	Hello World enter the hello module

	[root@FriendlyARM/]# lsmod
	helloworld 778 0 - Live 0xbf000000 (P)

	[root@FriendlyARM/]# rmmod helloworld
	rmmod: chdir(/lib/modules): No such file or directory

注意，加载时是加载模块，因此insmod命令中指定的是模块文件名(helloworld.ko);     
卸载时是卸载模块，rmmod命令中指定的是模块名(helloworld).

卸载内核模块时报错，是因为现在的内核模块在插入卸载时都会要转到/lib/modules/内核版本号/ 这个目录里。
所以只要建立这个目录并且把要使用的模块.ko文件复制到这个目录就行了。

	mkdir -p /lib/modules/$(uname -r)

----

	[root@FriendlyARM/]# rmmod helloworld
	Hello World exit the hello module
	rmmod: module 'helloworld' not found

卸载掉模块，虽然成功，但是每次都多一句：

	rmmod: module '模块名' not found    

经测试得知rmmod: module '模块名' not found 来自于busybox , 并不是来自kernel      
解决方案如下：    

使用另一份源码生成rmmod命令,就可以没有任何提示的卸载ko模块了   
