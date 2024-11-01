// Compile:
// riscv32-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -nostartfiles -Tlink.ld -O0 -ffreestanding -fno-stack-protector  demo_2.c -o demo_2.elf

// Define stack top at 0x4000
#define STACK_TOP 0x4000

// Declare a global variable
int my_var;

// Minimal startup function to initialize .bss section
void _start() __attribute__((naked));
void _start() {
    asm volatile ("lui sp, 0x4"); // Set to 0x4000

    // Initialize .bss section to zero
    extern int _sbss, _ebss;
    int *bss = &_sbss;
    while (bss < &_ebss) {
        *bss++ = 0;
    }

    // Set a variable to a known value
    my_var = 42;

    // Infinite loop to prevent exit
    while (1);
}

// Main function (not necessary for minimal setup, but here for clarity)
void main() {
    // Do nothing (handled in _start)
}
