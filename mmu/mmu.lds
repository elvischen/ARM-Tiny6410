SECTIONS {
    . = 0x50000000;
    
	.text : {
			start.o
			main.o
			clock.o
			leds.o
			sdram.o
			mmu.o
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
