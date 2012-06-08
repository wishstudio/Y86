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
    return ["F_predPC"];
}

function outWires()
{
    return ["F_predPC", "D_icode", "D_ifun", "D_rA", "D_rB", "D_valC", "D_valP", "F_eip", "D_eip"];
}

function cycle()
{
    var icode, ifun;
    if (readWire("D_icode") == OP_EXCEP || readWire("E_icode") == OP_EXCEP || readWire("M_icode") == OP_EXCEP || readWire("W_icode") == OP_EXCEP)
    {
        icode = OP_INT;
        /* the order matters */
        if (readWire("M_icode") == OP_EXCEP)
            ifun = readWire("M_ifun");
        else if (readWire("E_icode") == OP_EXCEP)
            ifun = readWire("E_ifun");
        else if (readWire("D_icode") == OP_EXCEP)
            ifun = readWire("D_ifun");
        writeWire("D_eip", -1);
        writeWire("D_icode", icode);
        writeWire("D_ifun", ifun);
        writeWire("F_eip", readWire("F_predPC"));
        writeWire("F_predPC", readWire("F_predPC"));
        return;
    }

    var eip;
    if (readWire("M_icode") == OP_JMP && !readWire("M_Bch"))
        eip = readWire("M_valP");
    else if (readWire("W_icode") == OP_RET || readWire("W_icode") == OP_INT)
        eip = readWire("W_valM");
    else
        eip = readWire("F_predPC");
    var a = readMemoryChar(eip);
    icode = (a & 0xF0) >> 4;
    ifun = a & 0x0F;
    writeWire("F_eip", eip);
    writeWire("D_eip", eip);
    writeWire("D_icode", icode);
    writeWire("D_ifun", ifun);
    addAction("icode:ifun <- M1[%eip]");

    switch (icode)
    {
    case OP_NOP:
    case OP_HALT:
    case OP_RET:
    case OP_INT:
        addAction("valP <- %eip + 1");
        writeWire("D_valP", eip + 1);
        writeWire("F_predPC", eip + 1);
        break;

    case OP_RRMOVL:
    case OP_OPL:
    case OP_PUSHL:
    case OP_POPL:
        addAction("rA:rB <- M1[%eip + 1]");
        addAction("valP <- %eip + 2");
        a = readMemoryChar(eip + 1);
        writeWire("D_rA", (a & 0xF0) >> 4);
        writeWire("D_rB", a & 0x0F);
        writeWire("D_valP", eip + 2);
        writeWire("F_predPC", eip + 2);
        break;

    case OP_IRMOVL:
    case OP_RMMOVL:
    case OP_MRMOVL:
        addAction("rA:rB <- M1[%eip + 1]");
        addAction("valC <- M4[%eip + 2]");
        addAction("valP <- %eip + 6");
        a = readMemoryChar(eip + 1);
        writeWire("D_rA", (a & 0xF0) >> 4);
        writeWire("D_rB", a & 0x0F);
        writeWire("D_valC", readMemoryInt(eip + 2));
        writeWire("D_valP", eip + 6);
        writeWire("F_predPC", eip + 6);
        break;

    case OP_JMP:
    case OP_CALL:
        addAction("valC <- M4[%eip + 1]");
        addAction("valP <- %eip + 5");
        var valC = readMemoryInt(eip + 1);
        writeWire("D_valC", valC);
        writeWire("D_valP", eip + 5);
        writeWire("F_predPC", valC);
        break;

    case OP_LIDT:
        addAction("valP <- %eip + 5");
        writeWire("D_valC", readMemoryInt(eip + 1));
        writeWire("D_valP", eip + 5);
        writeWire("F_predPC", eip + 5);
        break;
    }
}

function control()
{
    if (readWire("E_icode") == OP_MRMOVL || readWire("E_icode") == OP_POPL)
    {
        var E_dstM = readWire("E_dstM");
        if (readForwardingWire("d_srcA") == E_dstM || readForwardingWire("d_srcB") == E_dstM)
            stall();
    }
    else if (readWire("D_icode") == OP_RET || readWire("E_icode") == OP_RET || readWire("M_icode") == OP_RET)
        stall();
    else if (readWire("D_icode") == OP_INT || readWire("E_icode") == OP_INT || readWire("M_icode") == OP_INT)
        stall();
}
