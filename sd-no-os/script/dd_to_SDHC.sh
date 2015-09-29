#################################################################
#   Copyright (C) 2015 Sean Guo. All rights reserved.
#														  
#	> File Name:        < dd_to_SDHC.sh >
#	> Author:           < Sean Guo >		
#	> Mail:             < iseanxp+code@gmail.com >		
#	> Created Time:     < 2015/09/29 >
#	> Last Changed: 
#	> Description:		将编译的.bin文件通过dd命令烧写到SDHC的BL1区域;
#################################################################

#!/bin/bash

#					!!! 执行脚本前，必须明确指定要烧写的.bin文件(bin_file)与设备号(dev_SDHC) !!!
#					!!! 执行脚本前，必须明确指定要烧写的.bin文件(bin_file)与设备号(dev_SDHC) !!!
#					!!! 执行脚本前，必须明确指定要烧写的.bin文件(bin_file)与设备号(dev_SDHC) !!!
# 1. 指定SDHC在linux下的映射设备; 通过$ lsblk命令查看;
dev_SDHC="/dev/sdb" # 需要用户指定
bin_file="sd.bin"   # 指定要烧写的文件

echo 'SDHC ->' $dev_SDHC
echo "-------------- lsblk $dev_SDHC--------------"
lsblk | grep `basename $dev_SDHC`
echo '-------------------------------------------'

# 2. 通过fdisk查看SDHC的大小;
echo '[WARNING] fdisk, need sudoer authority'
echo "-------------- fdisk $dev_SDHC--------------"
#sudo fdisk -l | grep "Disk $dev_SDHC"
fdisk_string=`sudo fdisk -l | grep "Disk $dev_SDHC"` 
echo $fdisk_string

# eg. 
#Disk /dev/sdb: 7.3 GiB, 7822376960 bytes, 15278080 sectors
echo '-------------------------------------------'

# 3. 计算SDHC的BL的起始位置 = 总扇区数 - (1025 + 1 + 16);
echo "SDHC BL1 = (All_Sectors - (1025 + 1 + 16))"
Sectors=`echo $fdisk_string | cut -d ' ' -f 7`
echo "Sectors:$Sectors"
((BL1_Start_Sector=$Sectors-(1025 + 1 + 16)))
echo "BL1 Start Sector:$BL1_Start_Sector"
echo '-------------------------------------------'

#4. 通过dd将$bin_file烧写到$dev_SDHC中
echo "-------------- dd $dev_SDHC--------------"
echo "dd $bin_file to SDHC($dev_SDHC)"
echo "sudo dd if=$bin_file of=$dev_SDHC seek=$BL1_Start_Sector bs=512"
sudo dd if=$bin_file of=$dev_SDHC seek=$BL1_Start_Sector bs=512
