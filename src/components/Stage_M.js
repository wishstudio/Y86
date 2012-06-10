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
    return ["M_icode", "M_ifun", "M_valP", "M_valA", "M_valE", "M_dstE", "M_dstM", "M_Bch"];
}

function outWires()
{
    return ["m_icode", "W_eip", "W_icode", "W_ifun", "W_dstE", "W_valE", "W_dstM", "W_valM"];
}

function bubble()
{
    writeWire("M_eip", -1);
    writeWire("M_icode", 0);
    writeWire("M_ifun", 0);
    writeWire("M_valP", 0);
    writeWire("M_valA", 0);
    writeWire("M_valE", 0);
    writeWire("M_Bch", 0);
    writeWire("M_dstE", REG_NONE);
    writeWire("M_dstM", REG_NONE);
}

function exception(id)
{
    clearAction();
    addAction("exception");
    writeWire("m_icode", OP_EXCEP);
    writeWire("W_eip", -1);
    writeWire("W_icode", OP_EXCEP);
    writeWire("W_ifun", id);
}

function cycle()
{
    writeWire("W_eip", readWire("M_eip"));
    var icode = readWire("M_icode");
    var ifun = readWire("M_ifun");
    var valP = readWire("M_valP");
    var dstE = readWire("M_dstE");
    var valA = readWire("M_valA");
    var valE = readWire("M_valE");
    var dstM = readWire("M_dstM");

    switch (icode)
    {
    case OP_RMMOVL:
        addAction("M4[valE] <- valA");
        if (!writeMemoryInt(valE, valA))
        {
            exception(EXCEP_MEM);
            return;
        }
        break;

    case OP_MRMOVL:
    case OP_INT:
        addAction("valM <- M4[valE]");
        if (!canReadMemoryInt(valE))
        {
            if (icode == OP_INT)
                exception(EXCEP_DBL);
            else
                exception(EXCEP_MEM);
            return;
        }
        writeWire("W_valM", readMemoryInt(valE));
        break;

    case OP_CALL:
        addAction("M4[valE] <- valP");
        if (!writeMemoryInt(valE, valP))
        {
            exception(EXCEP_MEM);
            return;
        }
        break;

    case OP_RET:
        addAction("valM <- M4[valA]");
        if (!canReadMemoryInt(valA))
        {
            exception(EXCEP_MEM);
            return;
        }
        writeWire("W_valM", readMemoryInt(valA));
        break;

    case OP_PUSHL:
        addAction("M4[valE] <- valA");
        if (!writeMemoryInt(valE, valA))
        {
            exception(EXCEP_MEM);
            return;
        }
        break;

    case OP_POPL:
        addAction("valM <- M4[valA]");
        if (!canReadMemoryInt(valA))
        {
            exception(EXCEP_MEM);
            return;
        }
        writeWire("W_valM", readMemoryInt(valA));
        break;

    case OP_EXCEP:
        icode = OP_NOP;
        break;
    }

    writeWire("m_icode", icode);
    writeWire("W_icode", icode);
    writeWire("W_ifun", ifun);
    writeWire("W_dstE", dstE);
    writeWire("W_valE", valE);
    writeWire("W_dstM", dstM);
}

function control()
{
    if (readWire("M_icode") == OP_HALT || readWire("W_icode") == OP_HALT)
        bubble();
    else if (readForwardingWire("m_icode") == OP_EXCEP)
        bubble();
}
