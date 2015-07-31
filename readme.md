FriendlyARM Tiny6410
====================
核心板: Tiny6410 (1308版本)
底层板: TinyADK (1312B版本)

-----------------------------------------------------

下载程序:
1. 底层板S2选择NAND启动方式
2. 按住K1按钮，打开电源开关S1
3. 此时若串口连接COM0到PC，配置波特率为115200，就可以看到串口输出消息; 不进行串口连接，也可以在液晶屏上看到同样的消息;
4. 输出消息:

	Booting from NAND

	Superboot6410
	Ver: 141011
	CPU: S3C6410 533MHz
	RAM: 256MB
	NAND: 1GB(SLC) ID:ECD35195
	Touch Device: 1-wire
	LCD Type: S70(Auto)

	USB Mode: Waiting...
	Hello USB Loop

5. 底层板通过USB与PC连接，PC识别USB设备。打开MiniTools.exe软件，识别到ARM开发板;
6. 同时串口输出消息: 
	
	USB Mode: Connected

7. 选择不同模式，下载程序;

注意：**必须先给板子上电，然后在打开MiniTools.exe**, 否则MiniTools.exe会卡死无响应。
-----------------------------------------------------
