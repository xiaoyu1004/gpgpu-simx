.equ CSR_WTID, 0xcc0
.equ CSR_LWID, 0xcc3
.equ CSR_NT, 0xfc0
.equ KERNEL_ARG_DEV_MEM_ADDR, 0x7ffff

.section .text
.global main
main:
    # tmc a5
    li a0, -1
    .insn s 0x6b, 0, x0, 0(a0)  
    csrr a4, CSR_WTID # get warp's thread id
    csrr a5, CSR_LWID # get core's local warp id
    csrr a1, CSR_NT # get the number of threads in a warp
    # get idx
    mul a5, a5, a1
    add a5, a5, a4
    # get init addr
    lui a4, KERNEL_ARG_DEV_MEM_ADDR
    # src ptr
    lw a3, 4(a4)
    # dst ptr 
    lw a4, 8(a4)
    # src offset -> idx * 4
    sll a5, a5, 0x2
    # idx * 4 + a3
    add a3, a3, a5
    # load memory
    lw a3, 0(a3)
    # store memory
    add a5, a4, a5
    sw a3, 0(a5)
    # bar -> wait for all warps to complete
    li a2,0
    li a5, 4
    .insn s 0x6b, 4, a2, 0(a5)
    # tmc a0
    li a0, 0
    .insn s 0x6b, 0, x0, 0(a0)