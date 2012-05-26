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
    return ["E_icode", "E_valP", "E_dstE", "E_valA", "E_valE"];
}

function outWires()
{
    return ["M_icode", "M_dstE", "M_valE", "M_valM"];
}

function cycle()
{
    var icode = readWire("E_icode");
    var valP = readWire("E_valP");
    var dstE = readWire("E_dstE");
    var valA = readWire("E_valA");
    var valE = readWire("E_valE");

    switch (icode)
    {
    case OP_RMMOVL:
        addAction("M4[valE] <- valA");
        writeMemory(valE, valA);
        break;

    case OP_MRMOVL:
        addAction("valM <- M4[valE]");
        writeWire("M_valM", readMemory(valE));
        break;

    case OP_CALL:
        addAction("M4[valE] <- valP");
        writeMemory(valE, valP);
        break;

    case OP_RET:
        addAction("valM <- M4[valA]");
        writeWire("M_valM", readMemory(valA));
        break;

    case OP_PUSHL:
        addAction("M4[valE] <- valA");
        writeMemory(valE, valA);
        break;

    case OP_POPL:
        addAction("valM <- M4[valA]");
        writeWire("M_valM", readMemory(valA));
        break;
    }

    writeWire("M_icode", icode);
    writeWire("M_dstE", dstE);
    writeWire("M_valE", valE);
}
