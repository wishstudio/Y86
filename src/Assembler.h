/*
 *   Copyright (C) 2012 by Xiangyan Sun <wishstudio@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 3, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef AS_H
#define AS_H

#include <Qt>

#include "Memory.h"

#define OP_NOP      0x00
#define OP_HALT     0x01
#define OP_RRMOVL   0x02
#define OP_IRMOVL   0x03
#define OP_RMMOVL   0x04
#define OP_MRMOVL   0x05
#define OP_OPL      0x06
#define OP_JMP      0x07
#define OP_CALL     0x08
#define OP_RET      0x09
#define OP_PUSHL    0x0A
#define OP_POPL     0x0B
#define OP_CNT      0x0C

extern QString opNames[];

#define FUN_ADDL    0x00
#define FUN_SUBL    0x01
#define FUN_MULL    0x02
#define FUN_DIVL    0x03
#define FUN_MODL    0x04
#define FUN_ANDL    0x05
#define FUN_ORL     0x06
#define FUN_XORL    0x07
#define FUN_OPL_CNT 0x08

extern QString funOplNames[];

#define FUN_JMP     0x00
#define FUN_JLE     0x01
#define FUN_JL      0x02
#define FUN_JE      0x03
#define FUN_JNE     0x04
#define FUN_JGE     0x05
#define FUN_JG      0x06
#define FUN_JMP_CNT 0x07

extern QString funJmpNames[];

#define REG_EAX     0x00
#define REG_ECX     0x01
#define REG_EDX     0x02
#define REG_EBX     0x03
#define REG_ESI     0x04
#define REG_EDI     0x05
#define REG_ESP     0x06
#define REG_EBP     0x07
#define REG_NONE    0x08
#define REG_CNT     0x09

extern QString registerNames[];

class Assembler
{
public:
    static void compileFile(const QString &fileName, Memory *memory);
    static int startEIP();
    static int startESP();
};

#endif
