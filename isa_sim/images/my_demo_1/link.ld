SECTIONS
{
    . = 0x2000;

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        . = ALIGN(4);
    }

    /* Define the end of the program and set the stack pointer */
    _end = 0x8000;
    /* PROVIDE(_stack = 0x4000);  /* 4KB stack */
}

MEMORY {
  rom (rx)   : ORIGIN = 0x2000,     LENGTH = 16K
  ram (rw!x) : ORIGIN = 0x3000,     LENGTH = 10K
}

