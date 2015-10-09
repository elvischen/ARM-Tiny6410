SECTIONS {
    . = 0xc0000000;
    
	.text : {
			start.o
			sdram.o
			mmu.o
			clock.o
			* (.text)
	}

    . = ALIGN(4);
    .rodata :
    {
    	* (.rodata)
    }

    . = ALIGN(4);
	.data :
	{
		* (.data)
	}

    . = ALIGN(4);
    bss_start = .;
	.bss :
	{
		* (.bss)    
		* (.common)
	}
    bss_end = .;
}
