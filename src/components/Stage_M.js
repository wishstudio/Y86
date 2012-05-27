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
    return ["M_icode", "M_valP", "M_dstE", "M_valA", "M_valE"];
}

function outWires()
{
    return ["W_icode", "W_dstE", "W_valE", "W_valM"];
}

function cycle()
{
    var icode = readWire("M_icode");
    var valP = readWire("M_valP");
    var dstE = readWire("M_dstE");
    var valA = readWire("M_valA");
    var valE = readWire("M_valE");

    switch (icode)
    {
    case OP_RMMOVL:
        addAction("M4[valE] <- valA");
        writeMemory(valE, valA);
        break;

    case OP_MRMOVL:
        addAction("valM <- M4[valE]");
        writeWire("W_valM", readMemory(valE));
        break;

    case OP_CALL:
        addAction("M4[valE] <- valP");
        writeMemory(valE, valP);
        break;

    case OP_RET:
        addAction("valM <- M4[valA]");
        writeWire("W_valM", readMemory(valA));
        break;

    case OP_PUSHL:
        addAction("M4[valE] <- valA");
        writeMemory(valE, valA);
        break;

    case OP_POPL:
        addAction("valM <- M4[valA]");
        writeWire("W_valM", readMemory(valA));
        break;
    }

    writeWire("W_icode", icode);
    writeWire("W_dstE", dstE);
    writeWire("W_valE", valE);
}
