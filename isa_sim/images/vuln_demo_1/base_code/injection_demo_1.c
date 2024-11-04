/*
   Simple C++ startup routine to setup CRT
   SPDX-License-Identifier: Unlicense

   (https://five-embeddev.com/ | http://www.shincbm.com/) 

   Source: https://github.com/five-embeddev/riscv-scratchpad/blob/master/baremetal-startup-c/src/startup.c

*/

// Compile with:
// riscv32-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -nostartfiles -Tlink.ld -O0 -ffreestanding -fno-stack-protector injection_demo_1.c -o injection_demo_1.elf
// riscv32-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -nostartfiles -Tlink.ld -O0 injection_demo_1.c -o injection_demo_1.elf


// Decompile with:
// riscv32-unknown-elf-objdump -D injection_demo_1.elf > injection_demo_1.dump

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

// At this point we have a stack and global pointer, but no access to global variables.
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


void strcpy_one_by_one(char *dst, const char *src) {
    while (*src) {
        *dst = *src;
        dst++;
        src++;
    }
    *dst = '\0';
}

void memcpy_one_by_one_u8(uint8_t *dst, const uint8_t *src, size_t len) {
    for (size_t i = 0; i < len; i++) {
        *dst = *src;
        dst++;
        src++;
    }
}

// FIXME: Known bug - global variables are not currently supported with this memory/linker model.
// char opt_demo_string[] = "OPTIMIZ[b7020001130380051373f30fb3e262007390227b6ff0dfff]DONE";

void demo_partial_progression_with_gaps(void) {
    char demo_string[200];

    sim_puts("Starting partial progression demo...\n");

    // Copy the demo string to the stack.
    strcpy_one_by_one(demo_string, "OPTI");
    sim_puts("Loaded 'OPTI' to stack\n");
    strcpy_one_by_one(demo_string + 2, "MIZ");
    sim_puts("Loaded 'MIZ' to stack. Returning now.\n");
}


void demo_vulnerability_by_copying_trigger_and_payload_to_stack(void) {
    uint8_t demo_string[200];
    sim_puts("DEBUG: Expecting address of start of payload to be stored to 0x");
    sim_put_hex(((uint32_t) &demo_string) + 8);
    sim_puts("\n");

    sim_puts("Starting to memory load the optimization demo...\n");

    // Copy the demo string to the stack.
    // strcpy_one_by_one(demo_string, "OPTIMIZ[b7020001130380051373f30fb3e262007390227b6ff0dfff]DONE");
    memcpy_one_by_one_u8(
        demo_string,
        // "OPTIMIZ[b7020001130380051373f30fb3e262007390227b6ff0dfff]DONE"
        (uint8_t[]) {
            'O', 'P', 'T', 'I', 'M', 'I', 'Z', '[', // 8 bytes
            // Payload here (24 bytes):
            0xb7,
            0x70,
            0x02,
            0x20,
            0x00,
            0x00,
            0x01,
            0x11,
            0x13,
            0x30,
            0x03,
            0x38,
            0x80,
            0x00,
            0x05,
            0x51,
            0x13,
            0x37,
            0x73,
            0x3f,
            0xf3,
            0x30,
            0x0f,
            0xfb,
            // End of payload
            ']',
            'a',
            'b',
            'c',
            'd'
        },
        37
    );

    sim_puts("Optimization demo completed!\n");
}

int main(void) {
    sim_puts("Hello, World!\n");

    sim_puts("Calling demo_partial_progression_with_gaps()\n");
    demo_partial_progression_with_gaps();
    sim_puts("Returned from demo_partial_progression_with_gaps()\n");

    sim_puts("Calling demo_vulnerability_by_copying_trigger_and_payload_to_stack()\n");
    demo_vulnerability_by_copying_trigger_and_payload_to_stack();
    sim_puts("Returned from demo_vulnerability_by_copying_trigger_and_payload_to_stack()\n");

    return 0;
}
