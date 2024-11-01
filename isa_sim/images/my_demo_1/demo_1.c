// Compile:
// riscv32-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -nostartfiles -Tlink.ld -O0 -ffreestanding -fno-stack-protector  demo_1.c -o demo_1.elf


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

// Declare a global variable
int my_var;

void main() {
    const char *s = "Hello, World!\n";

    int counter = 0;
    
    while(1) {
        sim_puts(s);
        counter++;

        sim_puts("Counter: 0x");
        sim_put_hex(counter);
        sim_puts("\n");

        if (counter == 10) {
            sim_exit(0);
        }
    }
}

// Minimal startup function to initialize .bss section
void _enter() __attribute__((naked));
void _enter() {
    asm volatile ("lui sp, 0x4"); // Set to 0x4000
    asm volatile ("addi sp, sp, 0x104"); // Add on to get it to the max stack size (based on `STACK_SIZE` in `link.ld`)

    // Set a variable to a known value
    my_var = 42;

    char x = 90;

    main();
}
