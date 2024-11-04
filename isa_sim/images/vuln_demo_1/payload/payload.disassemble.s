
payload.elf:     file format elf32-littleriscv


Disassembly of section .text:

00000000 <_start>:
   0:	010002b7          	lui	t0,0x1000
   4:	05800313          	li	t1,88
   8:	0ff37313          	zext.b	t1,t1
   c:	0062e2b3          	or	t0,t0,t1

00000010 <loop>:
  10:	7b229073          	csrw	dscratch0,t0
  14:	ffdff06f          	j	10 <loop>
