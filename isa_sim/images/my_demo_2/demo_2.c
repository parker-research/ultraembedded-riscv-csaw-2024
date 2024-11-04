/*
   Simple C++ startup routine to setup CRT
   SPDX-License-Identifier: Unlicense

   (https://five-embeddev.com/ | http://www.shincbm.com/) 

   Source: https://github.com/five-embeddev/riscv-scratchpad/blob/master/baremetal-startup-c/src/startup.c

*/

// Compile with:
// riscv32-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -nostartfiles -Tlink.ld -O0 -ffreestanding -fno-stack-protector  demo_2.c -o demo_2.elf
// riscv32-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -nostartfiles -Tlink.ld -O0 demo_2.c -o demo_2.elf

#include <stdint.h>
#include <string.h>


#define CSR_SIM_CTRL_EXIT (0 << 24)
#define CSR_SIM_CTRL_PUTC (1 << 24)

static inline void sim_exit(int exitcode)
{
    const unsigned int arg = CSR_SIM_CTRL_EXIT | ((unsigned char)exitcode);
    asm volatile ("csrw dscratch,%0": : "r" (arg));
}

static inline void sim_putc(int ch)
{
    const unsigned int arg = CSR_SIM_CTRL_PUTC | (ch & 0xFF);
    asm volatile ("csrw dscratch,%0": : "r" (arg));
}

static inline void sim_puts(const char *s)
{
    while (*s)
    {
        sim_putc(*s);
        s++;
    }
}

static inline void sim_put_hex(unsigned int val)
{
    const char *hex = "0123456789abcdef";
    for(int i=0;i<8;i++)
    {
        sim_putc(hex[(val >> 28) & 0xF]);
        val <<= 4;
    }
}


// Generic C function pointer.
typedef void(*function_t)(void) ;

// These symbols are defined by the linker script.
// See linker.lds
extern uint8_t metal_segment_bss_target_start;
extern uint8_t metal_segment_bss_target_end;
extern const uint8_t metal_segment_data_source_start;
extern uint8_t metal_segment_data_target_start;
extern uint8_t metal_segment_data_target_end;
extern const uint8_t metal_segment_itim_source_start;
extern uint8_t metal_segment_itim_target_start;
extern uint8_t metal_segment_itim_target_end;

extern function_t __init_array_start;
extern function_t __init_array_end;
extern function_t __fini_array_start;
extern function_t __fini_array_end;

// This function will be placed by the linker script according to the section
// Raw function 'called' by the CPU with no runtime.
extern void _enter(void)  __attribute__ ((naked, section(".text.metal.init.enter")));

// Entry and exit points as C functions.
extern void _start(void) __attribute__ ((noreturn));
void _Exit(int exit_code) __attribute__ ((noreturn,noinline));

// Standard entry point, no arguments.
extern int main(void);

// The linker script will place this in the reset entry point.
// It will be 'called' with no stack or C runtime configuration.
// NOTE - this only supports a single hart.
// tp will not be initialized
void _enter(void) {
    // Setup SP and GP
    // The locations are defined in the linker script
    __asm__ volatile  (
        ".option push;"
        // The 'norelax' option is critical here.
        // Without 'norelax' the global pointer will
        // be loaded relative to the global pointer!
         ".option norelax;"
        "la    gp, __global_pointer$;"
        ".option pop;"
        "la    sp, _sp;"
        "jal   zero, _start;"
        :  /* output: none %0 */
        : /* input: none */
        : /* clobbers: none */); 
    // This point will not be executed, _start() will be called with no return.
}

// At this point we have a stack and global poiner, but no access to global variables.
void _start(void) {

    // Init memory regions
    // Clear the .bss section (global variables with no initial values)
    memset((void*) &metal_segment_bss_target_start,
           0, 
           (&metal_segment_bss_target_end - &metal_segment_bss_target_start));

    // Initialize the .data section (global variables with initial values)
    memcpy((void*)&metal_segment_data_target_start,
           (const void*)&metal_segment_data_source_start,
           (&metal_segment_data_target_end - &metal_segment_data_target_start));

    // Initialize the .itim section (code moved from flash to SRAM to improve performance)
    memcpy((void*)&metal_segment_itim_target_start,
           (const void*)&metal_segment_itim_source_start,
           (&metal_segment_itim_target_end - &metal_segment_itim_target_start));

    // Call constructors
    for (const function_t* entry=&__init_array_start; 
         entry < &__init_array_end;
         ++entry) {
        (*entry)();
    }

    int rc = main();

    // Call destructors
    for (const function_t* entry=&__fini_array_start; 
         entry < &__fini_array_end;
         ++entry) {
        (*entry)();
    }


    _Exit(rc);
}

// This should never be called. Busy loop with the CPU in idle state.
void _Exit(int exit_code) {
    sim_exit(exit_code);

    (void)exit_code;

    // Halt
    while (1) {
        __asm__ volatile ("wfi");
    }
}

uint32_t demo_function(uint8_t x) {
    return 42 * x;
}

int main(void) {
    sim_puts("Hello, World!\n");

    uint8_t x = 12;
    x += 3;
    x *= 6;
    sim_puts("After math, x = ");
    sim_put_hex(x);
    sim_puts("\n");

    sim_puts("Calling demo_function()\n");

    uint32_t demo_function_result = demo_function(x);
    sim_puts("Return value from demo_function: ");
    sim_put_hex(demo_function_result);
    sim_puts("\n");

    for (uint8_t i = 0; i < 10; i++) {
        sim_puts("Loop demo #");
        sim_put_hex(i);
        sim_puts("\n");
    }

    return 0;
}
