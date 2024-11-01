// Basic demo program for `ultraembedded/isa_sim` RISC-V simulator.
// Compile with:
// -march=rv32im_zicsr_a -> most capable supported ISA (base 32-bit integer, multiply, atomic, and CSR instructions)
// riscv32-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -nostartfiles -Tlink.ld demo_1.c -o demo_1.elf
// riscv32-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -nostartfiles -Ttext=0x2000 -Wl,--nmagic demo_1.c -o demo_1.elf

// With no alignment.
// riscv32-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -nostartfiles -Tlink.ld -Wl,--nmagic demo_1.c -o demo_1.elf


#define CSR_SIM_CTRL_EXIT (0 << 24)
#define CSR_SIM_CTRL_PUTC (1 << 24)

static inline void sim_exit(int exitcode)
{
    unsigned int arg = CSR_SIM_CTRL_EXIT | ((unsigned char)exitcode);
    asm volatile ("csrw dscratch,%0": : "r" (arg));
}

static inline void sim_putc(int ch)
{
    unsigned int arg = CSR_SIM_CTRL_PUTC | (ch & 0xFF);
    asm volatile ("csrw dscratch,%0": : "r" (arg));
}

// static inline void sim_puts(const char *s)
// {
//     while (*s)
//         sim_putc(*s++);
// }

/// _start() is the entry point for the program.
void _start() __attribute__((naked));
void _start()
{
    // Magic from the basic.elf example.
    // asm volatile (
    //     "lui r2, 0x5\n"
    //     "addi r2, r2, -776\n"
    //     "lui r5, 0x2\n"
    //     "addi r5, r5, 292\n"
    //     "csrw 0x305, r5\n"
    // );

    asm volatile ("lui sp, 0x4"); // Set to 0x4000
    asm volatile ("addi sp, sp, -776");
    asm volatile ("lui t0, 0x2");
    asm volatile ("addi t0, t0, 292");
    asm volatile ("csrw 0x305, t0");

    // Load address of _stack into sp register.
    // asm volatile ("la sp, _stack");
    // asm volatile ("la sp, 0x8000");
    // asm volatile ("li sp, 0x4000");
    // asm volatile ("lui sp, 0x4"); // Set to 0x4000
    // asm volatile ("li a0, 'Z'");



    // sim_puts("Hello, RISC-V!\n");
    // sim_putc('x');
    // sim_exit(0);
}

void my_code() {
    asm volatile ("li a0, 'Z'");
}

// void _start() __attribute__((naked));
// void _start()
// {
//     asm volatile (
//         "lui sp, 0x5000\n"      // Set up stack pointer
//         "li a0, 'Z'\n"          // Load character 'Z'
//         "csrw dscratch, a0\n"   // Output 'Z'
//         "li a0, 0\n"            // Exit code 0
//         "csrw dscratch, a0\n"   // Exit
//     );
// }
