/****************************************************************
    Copyright (C) 2015 Sean Guo. All rights reserved.
					      									  
    > File Name:         < watchdog.h >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2015/10/30 >
    > Last Changed: 
    > Description:
****************************************************************/


#ifndef __watchdog__
#define __watchdog__

#define WDT_BASE			(0x7E004000)
#define WTCON				( *((volatile unsigned long *)(WDT_BASE+0x00)) )
#define WTDAT				( *((volatile unsigned long *)(WDT_BASE+0x04)) )
#define WTCNT				( *((volatile unsigned long *)(WDT_BASE+0x08)) )
#define WTCLRINT			( *((volatile unsigned long *)(WDT_BASE+0x0C)) )

#endif /* __watchdog__ */
