/* ---- Original Script: /opt/riscv32i/riscv32-unknown-elf/lib/ldscripts/elf32lriscv.x ---- */
/* Default linker script, for normal executables */
/* Copyright (C) 2014-2017 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */
OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(main)
SECTIONS
{
  /* . = 0x80000000; */
  .text : { *(.text) }
}
