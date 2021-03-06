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

function exception(id)
{
    clearAction();
    addAction("exception");
    writeWire("F_eip", -1);
    writeWire("D_eip", -1);
    writeWire("D_icode", OP_EXCEP);
    writeWire("D_ifun", id);
}

function cycle()
{
    var icode, ifun;
    if (readWire("D_icode") == OP_EXCEP || readWire("E_icode") == OP_EXCEP || readWire("M_icode") == OP_EXCEP || readWire("W_icode") == OP_EXCEP)
    {
        icode = OP_INT;
        /* the order matters */
        if (readWire("W_icode") == OP_EXCEP)
            ifun = readWire("W_ifun");
        else if (readWire("M_icode") == OP_EXCEP)
            ifun = readWire("M_ifun");
        else if (readWire("E_icode") == OP_EXCEP)
            ifun = readWire("E_ifun");
        else if (readWire("D_icode") == OP_EXCEP)
            ifun = readWire("D_ifun");
        /* when a double fault occurs, just shut down the cpu */
        if (ifun == EXCEP_DBL)
        {
            writeWire("D_eip", -1);
            writeWire("D_icode", OP_HALT);
            writeWire("D_ifun", 0);
            writeWire("F_eip", -1);
            writeWire("F_predPC", readWire("F_predPC"));
            return;
        }
        writeWire("D_eip", -1);
        writeWire("D_icode", icode);
        writeWire("D_ifun", ifun);
        writeWire("F_eip", readWire("F_predPC"));
        writeWire("F_predPC", readWire("F_predPC"));
        return;
    }

    var eip;
    if (readWire("M_icode") == OP_JMP && !readWire("M_Bch"))
    {
        addAction("%eip <- M_valP");
        eip = readWire("M_valP");
    }
    else if (readWire("W_icode") == OP_RET || readWire("W_icode") == OP_INT)
    {
        addAction("%eip <- W_valM");
        eip = readWire("W_valM");
    }
    else
    {
        addAction("%eip <- F_predPC");
        eip = readWire("F_predPC");
    }
    if (!canExecuteMemoryChar(eip))
    {
        exception(EXCEP_MEM);
        return;
    }

    var a = readMemoryChar(eip);
    icode = (a & 0xF0) >> 4;
    ifun = a & 0x0F;
    writeWire("F_eip", eip);
    writeWire("D_eip", eip);
    writeWire("D_icode", icode);
    writeWire("D_ifun", ifun);
    addAction("fetch instruction");

    switch (icode)
    {
    case OP_NOP:
    case OP_HALT:
    case OP_RET:
    case OP_INT:
        writeWire("D_valP", eip + 1);
        writeWire("F_predPC", eip + 1);
        break;

    case OP_RRMOVL:
    case OP_OPL:
    case OP_PUSHL:
    case OP_POPL:
        if (!canExecuteMemoryChar(eip + 1))
        {
            exception(EXCEP_MEM);
            return;
        }
        a = readMemoryChar(eip + 1);
        writeWire("D_rA", (a & 0xF0) >> 4);
        writeWire("D_rB", a & 0x0F);
        writeWire("D_valP", eip + 2);
        writeWire("F_predPC", eip + 2);
        break;

    case OP_IRMOVL:
    case OP_RMMOVL:
    case OP_MRMOVL:
        if (!canExecuteMemoryChar(eip + 1) || !canExecuteMemoryInt(eip + 2))
        {
            exception(EXCEP_MEM);
            return;
        }
        a = readMemoryChar(eip + 1);
        writeWire("D_rA", (a & 0xF0) >> 4);
        writeWire("D_rB", a & 0x0F);
        writeWire("D_valC", readMemoryInt(eip + 2));
        writeWire("D_valP", eip + 6);
        writeWire("F_predPC", eip + 6);
        break;

    case OP_JMP:
    case OP_CALL:
        if (!canExecuteMemoryInt(eip + 1))
        {
            exception(EXCEP_MEM);
            return;
        }
        var valC = readMemoryInt(eip + 1);
        writeWire("D_valC", valC);
        writeWire("D_valP", eip + 5);
        writeWire("F_predPC", valC);
        break;

    case OP_LIDT:
        if (!canExecuteMemoryInt(eip + 1))
        {
            exception(EXCEP_MEM);
            return;
        }
        writeWire("D_valC", readMemoryInt(eip + 1));
        writeWire("D_valP", eip + 5);
        writeWire("F_predPC", eip + 5);
        break;

    default:
        exception(EXCEP_OP);
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
    else if (readWire("D_icode") == OP_HALT || readWire("E_icode") == OP_HALT || readWire("M_icode") == OP_HALT || readWire("W_icode") == OP_HALT)
        stall();
    else if (readWire("D_icode") == OP_RET || readWire("E_icode") == OP_RET || readWire("M_icode") == OP_RET)
        stall();
    else if (readWire("D_icode") == OP_INT || readWire("E_icode") == OP_INT || readWire("M_icode") == OP_INT)
        stall();
}
