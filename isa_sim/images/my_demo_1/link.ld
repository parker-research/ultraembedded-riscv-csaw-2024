/* Source: https://stackoverflow.com/questions/77892754/linker-script-for-riscv-cpu-on-fpga-stack-is-not-where-i-expect-it-to-be */

ENTRY(_enter)

BRAM_SIZE = 1024;

MEMORY {
    INSTR(RX)  : ORIGIN =0x00002000 , LENGTH = BRAM_SIZE 
    DATA(RWX)  : ORIGIN =0x00004000 , LENGTH = BRAM_SIZE
}

STACK_SIZE = 0x100; /* THIS IS REALLY IMPORTANT !!!!!!!!!!!!!!!! */

/* Section Definitions */
SECTIONS
{
    .text 0x2000 :
    {
        KEEP(*(.vectors .vectors.*))
        *(.text*)
        *(.rodata*)
    } > INSTR

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        *(.bss*)
        *(COMMON)
    } > DATA

    .data :
    {
        *(.data*);
    } > DATA AT >INSTR

    /* stack section */
    .stack (NOLOAD):
    {
        . = ALIGN(8);
        . = . + STACK_SIZE;
        . = ALIGN(8);
    } > DATA

    _end = . ;

    PROVIDE(__stack_top = ORIGIN(RAM) + LENGTH(RAM));
}
