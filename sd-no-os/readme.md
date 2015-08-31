SD Boot - SD卡启动
==================

1. s3c6410启动过程分成BL0, BL1, BL2几个阶段
2. 其中BL0是固化在s3c6410内部的IROM中的, 该段程序根据所选择的启动模式从存储介质加载BL1.
3. s3c6410支持从多种存储介质中启动，nandflash, sd卡，sdhc卡，OneNand, MoviNand.... BL1和BL2存储于这些存储介质中。
4. BL1,BL2的存储位置固定;
