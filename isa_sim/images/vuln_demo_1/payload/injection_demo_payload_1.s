# injection_demo_payload_1.s

# // Equivalent C Code:
# while (1) {
#     // Display 'X' character repeatedly in simulation.
#     sim_putc('X');
# }

    .section .text
    .global _start

_start:
    li t0, 0x01000000   # Load CSR_SIM_CTRL_PUTC constant (1 << 24) into t0
    li t1, 88           # Load ASCII value of 'X' into t1
    andi t1, t1, 0xFF   # Mask to ensure only the lower 8 bits are used
    or t0, t0, t1       # Combine CSR_SIM_CTRL_PUTC with 'X' in t0
loop:
    
    csrw dscratch, t0   # Write the argument to reg to display during simulation
    j loop              # Repeat indefinitely

# TODO: Future example: jump back in the code to where it was before.

# Build with:
# Assemble the source code to an object file
# riscv32-unknown-elf-as -o payload.o injection_demo_payload_1.s

# Link the object file to create an ELF, specifying the start address as 0
# riscv32-unknown-elf-ld -Ttext 0 -o payload.elf payload.o

# Disassemble the ELF file to verify the code, viewing both instructions and addresses
# riscv32-unknown-elf-objdump -d payload.elf

# Convert the ELF file to a raw binary file
# riscv32-unknown-elf-objcopy -O binary payload.elf payload.bin

# View the raw binary file in hexadecimal format
# hexdump -C payload.bin
