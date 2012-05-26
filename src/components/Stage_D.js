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
    return ["F_icode", "F_ifun", "F_valP", "F_rA", "F_rB", "F_valC"];
}

function outWires()
{
    return ["D_icode", "D_ifun", "D_valP", "D_valA", "D_valB", "D_valC", "D_dstE"];
}

function cycle()
{
    var icode = readWire("F_icode");
    var ifun = readWire("F_ifun");
    var valP = readWire("F_valP");
    var srcA = readWire("F_rA");
    var srcB = readWire("F_rB");
    var valC = readWire("F_valC");

    /* calculate valA and valB */
    switch (icode)
    {
    case OP_RRMOVL:
    case OP_RMMOVL:
    case OP_MRMOVL:
    case OP_OPL:
        addAction("valA <- R[rA]");
        addAction("valB <- R[rB]");
        writeWire("D_valA", readRegister(srcA));
        writeWire("D_valB", readRegister(srcB));
        break;

    case OP_PUSHL:
        addAction("valA <- R[rA]");
        addAction("valB <- R[%esp]");
        writeWire("D_valA", readRegister(srcA));
        writeWire("D_valB", readRegister(REG_ESP));
        break;

    case OP_POPL:
        addAction("valA <- R[%esp]");
        addAction("valB <- R[%esp]");
        writeWire("D_valA", readRegister(REG_ESP));
        writeWire("D_valB", readRegister(REG_ESP));
        break;
    }

    /* calculate dstE */
    switch (icode)
    {
    case OP_MRMOVL:
    case OP_POPL:
        writeWire("D_dstE", srcA);
        break;

    case OP_RRMOVL:
    case OP_IRMOVL:
    case OP_OPL:
        writeWire("D_dstE", srcB);
        break;
    }

    writeWire("D_icode", icode);
    writeWire("D_ifun", ifun);
    writeWire("D_valP", valP);
    writeWire("D_valC", valC);
}
