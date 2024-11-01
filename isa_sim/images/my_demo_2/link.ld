SECTIONS
{
    /* Code section at 0x2000 */
    .text 0x2000 :
    {
        *(.text)
    }

    /* Data section right after the code */
    .data :
    {
        *(.data)
    }

    /* BSS section for uninitialized variables */
    .bss :
    {
        _sbss = .;
        *(.bss)
        _ebss = .;
    }

    /* Stack starts at 0x4000 and grows downward */
    /* .stack (NOLOAD) : ORIGIN = 0x4000, LENGTH = 0x1000 */


    /* Define a stack region starting at 0x4000, but just reserve the space */
    . = 0x4000;
    _stack_top = .;
}
