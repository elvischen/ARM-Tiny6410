/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < nand.h >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/10/20 >
    > Last Changed: 
    > Description:		S3C6410 Nand Flash Controller - Nand Flash (K9K8G08U0E)
****************************************************************/


#ifndef __nand__
#define __nand__

// 设置NAND Flash控制器
void nand_init(void);

// Read ID
static char nand_read_ID(void);

// 读一页，即2048byte
static int nandll_read_page (unsigned char *buf, unsigned long addr);

#endif /* __nand__ */
