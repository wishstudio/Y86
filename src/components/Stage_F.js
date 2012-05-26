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

function inWires()
{
    return ["F_valP"];
}

function outWires()
{
    return ["F_icode", "F_ifun", "F_rA", "F_rB", "F_valC", "F_valP"];
}

function cycle()
{
    var eip = readWire("F_valP");
    var a = readMemoryChar(eip);
    var icode = (a & 0xF0) >> 4;
    var ifun = a & 0x0F;
    writeWire("F_icode", icode);
    writeWire("F_ifun", ifun);
    addAction("icode:ifun <- M1[%eip]")

    switch (icode)
    {
    case OP_NOP:
    case OP_HALT:
    case OP_RET:
        writeWire("F_valP", eip + 1);
        addAction("valP <- %eip + 1");
        break;

    case OP_RRMOVL:
    case OP_OPL:
    case OP_PUSHL:
    case OP_POPL:
        a = readMemoryChar(eip + 1);
        writeWire("F_rA", (a & 0xF0) >> 4);
        writeWire("F_rB", a & 0x0F);
        writeWire("F_valP", eip + 2);
        addAction("rA:rB <- M1[%eip + 1]")
        addAction("valP <- %eip + 2");
        break;

    case OP_IRMOVL:
    case OP_RMMOVL:
    case OP_MRMOVL:
        a = readMemoryChar(eip + 1);
        writeWire("F_rA", (a & 0xF0) >> 4);
        writeWire("F_rB", a & 0x0F);
        writeWire("F_valC", readMemoryInt(eip + 2));
        writeWire("F_valP", eip + 6);
        addAction("rA:rB <- M1[%eip + 1]");
        addAction("valC <- M4[%eip + 2]");
        addAction("valP <- %eip + 6");
        break;

    case OP_JMP:
    case OP_CALL:
        writeWire("F_valC", readMemoryInt(eip + 1));
        writeWire("F_valP", eip + 5);
        addAction("valC <- M4[%eip + 1]");
        addAction("valP <- %eip + 5");
        break;
    }
}
