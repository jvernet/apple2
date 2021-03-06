/*
 * Apple // emulator for *ix
 *
 * This software package is subject to the GNU General Public License
 * version 3 or later (your choice) as published by the Free Software
 * Foundation.
 *
 * Copyright 2013-2015 Aaron Culliney
 *
 */

#ifndef _CPU_REGS_H_
#define _CPU_REGS_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cpu.h"
#include "glue-offsets.h"

#define X_Reg           %bl             /* 6502 X register in %bl  */
#define Y_Reg           %bh             /* 6502 Y register in %bh  */
#define A_Reg           %cl             /* 6502 A register in %cl  */
#define F_Reg           %ch             /* 6502 flags in %ch       */
#define PC_Reg          %si             /* 6502 Program Counter    */
#define PC_Reg_L        %sil            /* 6502 PC low             */
#define PC_Reg_H        %sih            /* 6502 PC high            */
#define EffectiveAddr   %di             /* Effective address       */
#define EffectiveAddr_L %dil            /* Effective address low   */
#define EffectiveAddr_H %dih            /* Effective address high   */

#define X86_CF_Bit 0x0                  /* x86 carry               */
#define X86_AF_Bit 0x4                  /* x86 adj (nybble carry)  */

#if __LP64__
#   define SZ_PTR           8
#   define ROR_BIT          63
// x86_64 registers
#   define _XBP             %rbp        /* x86_64 base ptr/ scratch*/
#   define _XSP             %rsp        /* x86_64 stack pointer    */
#   define _XDI             %rdi
#   define _XSI             %rsi
#   define _XAX             %rax        /* scratch                 */
#   define _XBX             %rbx        /* scratch2                */
// full-length Apple ][ registers
#   define XY_Reg_X         %rbx        /* 6502 X&Y flags          */
#   define AF_Reg_X         %rcx        /* 6502 F&A flags          */
#   define reg_args         %rdx
#   define PC_Reg_X         %rsi        /* 6502 Program Counter    */
#   define EffectiveAddr_X  %rdi        /* Effective address       */
// full-length assembly instructions
#   define addLQ            addq
#   define andLQ            andq
#   define callLQ           callq
#   define decLQ            decq
#   define leaLQ            leaq
#   define orLQ             orq
#   define movLQ            movq
#   define movzbLQ          movzbq
#   define movzwLQ          movzwq
#   define popaLQ           popaq
#   define popLQ            popq
#   define pushaLQ          pushaq
#   define pushfLQ          pushfq
#   define pushLQ           pushq
#   define rorLQ            rorq
#   define shlLQ            shlq
#   define shrLQ            shrq
#   define subLQ            subq
#   define testLQ           testq
#   define xorLQ            xorq
#else
#   define SZ_PTR           4
#   define ROR_BIT          31
// x86 registers
#   define _XBP             %ebp        /* x86 base ptr / scratch  */
#   define _XSP             %esp        /* x86 stack pointer       */
#   define _XDI             %edi
#   define _XSI             %esi
#   define _XAX             %eax        /* scratch                 */
#   define _XBX             %ebx        /* scratch2                */
// full-length Apple ][ registers
#   define XY_Reg_X         %ebx        /* 6502 X&Y flags          */
#   define AF_Reg_X         %ecx        /* 6502 F&A flags          */
#   define reg_args         %edx
#   define PC_Reg_X         %esi        /* 6502 Program Counter    */
#   define EffectiveAddr_X  %edi        /* Effective address       */
// full-length assembly instructions
#   define addLQ            addl
#   define andLQ            andl
#   define callLQ           calll
#   define decLQ            decl
#   define leaLQ            leal
#   define orLQ             orl
#   define movLQ            movl
#   define movzbLQ          movzbl
#   define movzwLQ          movzwl
#   define popaLQ           popal
#   define popLQ            popl
#   define pushaLQ          pushal
#   define pushfLQ          pushfl
#   define pushLQ           pushl
#   define rorLQ            rorl
#   define shlLQ            shll
#   define shrLQ            shrl
#   define subLQ            subl
#   define testLQ           testl
#   define xorLQ            xorl
#endif

/* Symbol naming issues */
#if NO_UNDERSCORES
#   define ENTRY(x)         .globl x; .balign 16; x##:
#   define CALL(x)          x
#else
#   define ENTRY(x)         .globl _##x##; .balign 16; _##x##:
#   define CALL(x)          _##x
#endif

#define MOVB_IND(BASE,OFF,REG) \
                            movLQ   BASE(reg_args), _XBP; \
                            movb    (_XBP,OFF,1), REG;

#define CALL_IND(BASE,OFF) \
                            movLQ   BASE(reg_args), _XBP; \
                            callLQ  *(_XBP,OFF,SZ_PTR);

#define JUMP_IND(BASE,OFF) \
                            movLQ   BASE(reg_args), _XBP; \
                            jmp     *(_XBP,OFF,SZ_PTR);

#endif // whole file

