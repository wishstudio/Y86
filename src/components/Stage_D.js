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
    return ["D_icode", "D_ifun", "D_valP", "D_rA", "D_rB", "D_valC"];
}

function outWires()
{
    return ["d_icode", "d_srcA", "d_srcB", "E_eip", "E_icode", "E_ifun", "E_valP", "E_valA", "E_valB", "E_valC", "E_dstE", "E_dstM"];
}

function bubble()
{
    writeWire("D_eip", -1);
    writeWire("D_icode", 0);
    writeWire("D_ifun", 0);
    writeWire("D_valP", 0);
    writeWire("D_rA", REG_NONE);
    writeWire("D_rB", REG_NONE);
    writeWire("D_valC", 0);
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
    writeWire("E_eip", readWire("D_eip"));
    var icode = readWire("D_icode");
    var ifun = readWire("D_ifun");
    var rA = readWire("D_rA");
    var rB = readWire("D_rB");
    var valP = readWire("D_valP");
    var valC = readWire("D_valC");

    /* calculate valA and valB */
    var srcA = REG_NONE, srcB = REG_NONE;
    switch (icode)
    {
    case OP_RRMOVL:
        addAction("valA <- R[rA]");
        srcA = rA;
        break;

    case OP_RMMOVL:
        addAction("valA <- R[rA]");
        addAction("valB <- R[rB]");
        srcA = rA;
        srcB = rB;
        break;

    case OP_MRMOVL:
        addAction("valB <- R[rB]");
        srcB = rB;
        break;

    case OP_OPL:
        addAction("valA <- R[rA]");
        addAction("valB <- R[rB]");
        srcA = rA;
        srcB = rB;
        break;

    case OP_CALL:
        addAction("valB <- R[%esp]");
        srcB = REG_ESP;
        break;

    case OP_RET:
        addAction("valA <- R[%esp]");
        addAction("valB <- R[%esp]");
        srcA = srcB = REG_ESP;
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

    case OP_INT:
        addAction("valB <- R[%idtr]");
        srcB = REG_IDTR;
        break;
    }
    if (srcA != REG_NONE)
    {
        var valA = fetchRegisterWithForwarding(srcA);
        writeWire("E_valA", valA);
    }
    writeWire("d_srcA", srcA);
    if (srcB != REG_NONE)
    {
        var valB = fetchRegisterWithForwarding(srcB);
        writeWire("E_valB", valB);
    }
    writeWire("d_srcB", srcB);

    /* calculate dstE/dstM */
    var dstE = REG_NONE, dstM = REG_NONE;
    switch (icode)
    {
    case OP_MRMOVL:
        dstM = rA;
        break;

    case OP_RRMOVL:
    case OP_IRMOVL:
        dstE = rB;
        break;

    case OP_OPL:
        if (ifun != FUN_CMPL)
            dstE = rB;
        break;

    case OP_CALL:
        dstE = REG_ESP;
        break;

    case OP_RET:
        dstE = REG_ESP;
        break;

    case OP_PUSHL:
        dstE = REG_ESP;
        break;

    case OP_POPL:
        dstE = REG_ESP;
        dstM = rA;
        break;

    case OP_LIDT:
        dstE = REG_IDTR;
        break;

    case OP_EXCEP:
        icode = OP_NOP;
        break;
    }
    writeWire("E_dstE", dstE);
    writeWire("E_dstM", dstM);

    writeWire("d_icode", icode);
    writeWire("E_icode", icode);
    writeWire("E_ifun", ifun);
    writeWire("E_valP", valP);
    writeWire("E_valC", valC);
}

function control()
{
    if (readWire("E_icode") == OP_MRMOVL || readWire("E_icode") == OP_POPL)
    {
        var E_dstM = readWire("E_dstM");
        if (readForwardingWire("d_srcA") == E_dstM || readForwardingWire("d_srcB") == E_dstM)
            stall();
    }
    else if (readWire("E_icode") == OP_JMP && !readForwardingWire("M_Bch"))
        bubble();
    else if (readWire("D_icode") == OP_RET || readWire("E_icode") == OP_RET || readWire("M_icode") == OP_RET)
        bubble();
    else if (readWire("D_icode") == OP_INT || readWire("E_icode") == OP_INT || readWire("M_icode") == OP_INT)
        bubble();
    else if (readForwardingWire("d_icode") == OP_EXCEP || readForwardingWire("e_icode") == OP_EXCEP || readForwardingWire("m_icode") == OP_EXCEP)
        bubble();
}
