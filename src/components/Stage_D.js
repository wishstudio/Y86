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
    return ["D_icode", "D_ifun", "D_valP", "D_rA", "D_rB", "D_valC", "E_dstE", "E_dstM", "M_dstE", "M_valE", "M_dstM", "W_dstE", "W_valE", "W_dstM", "W_valM"];
}

function outWires()
{
    return ["E_icode", "E_ifun", "E_valP", "E_valA", "E_valB", "E_valC", "E_dstE", "E_dstM"];
}

function bubble()
{
    clearAction();
    addAction("bubble");
    writeWire("E_icode", OP_NOP);
    writeWire("E_dstE", REG_NONE);
    writeWire("E_dstM", REG_NONE);
}

function fetchRegisterWithForwarding(reg)
{
    if (reg == readWire("E_dstE")) return readForwardingWire("M_valE");
    if (reg == readWire("M_dstM")) return readForwardingWire("W_valM");
    if (reg == readWire("M_dstE")) return readWire("M_valE");
    if (reg == readWire("W_dstM")) return readWire("W_valM");
    if (reg == readWire("W_dstE")) return readWire("W_valE");
    return readRegister(reg);
}

function cycle()
{
    var icode = readWire("D_icode");
    var ifun = readWire("D_ifun");
    var valP = readWire("D_valP");
    var rA = readWire("D_rA");
    var rB = readWire("D_rB");
    var valC = readWire("D_valC");

    /* calculate valA and valB */
    var srcA = REG_NONE, srcB = REG_NONE;
    switch (icode)
    {
    case OP_RRMOVL:
    case OP_RMMOVL:
        addAction("valA <- R[rB]");
        srcA = rA;
        break;

    case OP_OPL:
        addAction("valA <- R[rA]");
        addAction("valB <- R[rB]");
        srcA = rA;
        srcB = rB;
        break;

    case OP_PUSHL:
        addAction("valA <- R[rA]");
        addAction("valB <- R[%esp]");
        srcA = rA;
        srcB = REG_ESP;
        break;

    case OP_POPL:
        addAction("valA <- R[%esp]");
        addAction("valB <- R[%esp]");
        srcA = srcB = REG_ESP;
        break;
    }
    if (srcA != REG_NONE)
    {
        if (srcA == readWire("E_dstM")) /* stall one cycle for a memory load */
        {
            bubble();
            return;
        }
        var valA = fetchRegisterWithForwarding(srcA);
        writeWire("E_valA", valA);
    }
    if (srcB != REG_NONE)
    {
        if (srcB == readWire("E_dstM")) /* stall one cycle for a memory load */
        {
            bubble();
            return;
        }
        var valB = fetchRegisterWithForwarding(srcB);
        writeWire("E_valB", valB);
    }

    /* calculate dstE/dstM */
    var dstE = REG_NONE, dstM = REG_NONE;
    switch (icode)
    {
    case OP_MRMOVL:
    case OP_POPL:
        dstM = rA;
        break;

    case OP_RRMOVL:
    case OP_IRMOVL:
    case OP_OPL:
        dstE = rB;
        break;
    }
    writeWire("E_dstE", dstE);
    writeWire("E_dstM", dstM);

    writeWire("E_icode", icode);
    writeWire("E_ifun", ifun);
    writeWire("E_valP", valP);
    writeWire("E_valC", valC);
}
