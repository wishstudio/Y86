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
    return ["E_icode", "E_ifun", "E_valP", "E_valA", "E_valB", "E_valC", "E_dstE", "E_dstM"];
}

function outWires()
{
    return ["e_icode", "M_eip", "M_icode", "M_ifun", "M_valP", "M_valA", "M_dstE", "M_valE", "M_dstM", "M_Bch"];
}

function bubble()
{
    writeWire("E_eip", -1);
    writeWire("E_icode", 0);
    writeWire("E_ifun", 0);
    writeWire("E_valP", 0);
    writeWire("E_valA", 0);
    writeWire("E_valB", 0);
    writeWire("E_valC", 0);
    writeWire("E_dstE", REG_NONE);
    writeWire("E_dstM", REG_NONE);
}

function cycle()
{
    var icode = readWire("E_icode");
    var ifun = readWire("E_ifun");
    var valP = readWire("E_valP");
    var valA = readWire("E_valA");
    var valB = readWire("E_valB");
    var valC = readWire("E_valC");
    var dstE = readWire("E_dstE");
    var dstM = readWire("E_dstM");

    /* calculate valE */
    var valE = 0;
    switch (icode)
    {
    case OP_RRMOVL:
        addAction("valE <- valA");
        valE = valA;
        break;

    case OP_IRMOVL:
    case OP_LIDT:
        addAction("valE <- valC");
        valE = valC;
        break;

    case OP_RMMOVL:
    case OP_MRMOVL:
        addAction("valE <- valB + valC");
        valE = valB + valC;
        break;

    case OP_OPL:
        switch (ifun)
        {
        case FUN_ADDL: addAction("valE <- valB + valA"); valE = valB + valA; break;
        case FUN_SUBL: addAction("valE <- valB - valA"); valE = valB - valA; valA = -valA; break;
        case FUN_CMPL: addAction("temp <- valB - valA"); valE = valB - valA; valA = -valA; break;
        case FUN_MULL: addAction("valE <- valB * valA"); valE = valB * valA; break;
        case FUN_DIVL:
            addAction("valE <- valB / valA");
            /* divide by zero */
            if (valA == 0)
            {
                addAction("exception");
                writeWire("e_icode", OP_EXCEP);
                writeWire("M_icode", OP_EXCEP);
                writeWire("M_ifun", EXCEP_DIVBZ);
                writeWire("M_eip", -1);
                return;
            }
            valE = valB / valA;
            break;
        case FUN_MODL: addAction("valE <- valB % valA"); valE = valB % valA; break;
        case FUN_ANDL: addAction("valE <- valB & valA"); valE = valB & valA; break;
        case FUN_ORL:  addAction("valE <- valB | valA"); valE = valB | valA; break;
        case FUN_XORL: addAction("valE <- valB ^ valA"); valE = valB ^ valA; break;
        }
        valE = int32(valE);

        if (ifun != FUN_MULL && ifun != FUN_DIVL && ifun != FUN_MODL)
        {
            addAction("set eflags");
            var cf = int32(ifun >= FUN_ANDL? 0: uint32(valE) < uint32(valA));
            var zf = int32((valE == 0));
            var sf = int32((valE < 0));
            var of = int32(ifun >= FUN_ANDL? 0: ((valA < 0) === (valB < 0)) && ((valE < 0) !== (valA < 0)));
            var eflags = (cf << EFLAGS_CF) | (zf << EFLAGS_ZF) | (sf << EFLAGS_SF) | (of << EFLAGS_OF);
            writeRegister(REG_EFLAGS, eflags);
        }
        break;

    case OP_JMP:
        addAction("test eflags");
        var eflags = readRegister(REG_EFLAGS);
        var cf = (eflags & (1 << EFLAGS_CF)) >> EFLAGS_CF;
        var zf = (eflags & (1 << EFLAGS_ZF)) >> EFLAGS_ZF;
        var sf = (eflags & (1 << EFLAGS_SF)) >> EFLAGS_SF;
        var of = (eflags & (1 << EFLAGS_OF)) >> EFLAGS_OF;

        var Bch = int32(ifun == FUN_JMP);
        Bch |= int32(ifun == FUN_JLE) & ((sf ^ of) | zf);
        Bch |= int32(ifun == FUN_JL) & (sf ^ of);
        Bch |= int32(ifun == FUN_JE) & (zf);
        Bch |= int32(ifun == FUN_JNE) & (!zf);
        Bch |= int32(ifun == FUN_JGE) & (!(sf ^ of));
        Bch |= int32(ifun == FUN_JG) & (!(sf ^ of) & !zf);
        writeWire("M_Bch", Bch);
        break;

    case OP_CALL:
        addAction("valE <- valB - 4");
        valE = valB - 4;
        break;

    case OP_RET:
        addAction("valE <- valB + 4");
        valE = valB + 4;
        break;

    case OP_PUSHL:
        addAction("valE <- valB - 4");
        valE = valB - 4;
        break;

    case OP_POPL:
        addAction("valE <- valB + 4");
        valE = valB + 4;
        break;

    case OP_INT:
        addAction("valE <- valB + ifun * 4");
        valE = valB + ifun * 4;
        break;

    case OP_EXCEP:
        icode = OP_NOP;
        break;
    }

    writeWire("e_icode", icode);
    writeWire("M_icode", icode);
    writeWire("M_valP", valP);
    writeWire("M_valA", valA);
    writeWire("M_dstE", dstE);
    writeWire("M_valE", valE);
    writeWire("M_dstM", dstM);
    writeWire("M_eip", readWire("E_eip"));
}

function control()
{
    if (readWire("E_icode") == OP_MRMOVL || readWire("E_icode") == OP_POPL)
    {
        var E_dstM = readWire("E_dstM");
        if (readForwardingWire("d_srcA") == E_dstM || readForwardingWire("d_srcB") == E_dstM)
            bubble();
    }
    else if (readWire("E_icode") == OP_HALT || readWire("M_icode") == OP_HALT || readWire("W_icode") == OP_HALT)
        bubble();
    else if (readWire("E_icode") == OP_JMP && !readForwardingWire("M_Bch"))
        bubble();
    else if (readForwardingWire("e_icode") == OP_EXCEP || readForwardingWire("m_icode") == OP_EXCEP)
        bubble();
}
