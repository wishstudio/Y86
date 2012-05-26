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
    return ["D_icode", "D_ifun", "D_valP", "D_valA", "D_valB", "D_dstE"];
}

function outWires()
{
    return ["E_icode", "E_dstE", "E_valP", "E_valA", "E_valE"];
}

function cycle()
{
    var icode = readWire("D_icode");
    var ifun = readWire("D_ifun");
    var valP = readWire("D_valP");
    var valA = readWire("D_valA");
    var valB = readWire("D_valB");
    var dstE = readWirE("D_dstE");

    /* calculate valE */
    var valE;
    switch (icode)
    {
    case OP_RRMOVL:
        addAction("valE <- valA");
        valE = valA;
        break;

    case OP_IRMOVL:
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
        case FUN_SUBL: addAction("valE <- valB - valA"); valE = valB - valA; break;
        case FUN_MULL: addAction("valE <- valB * valA"); valE = valB * valA; break;
        case FUN_DIVL: addAction("valE <- valB / valA"); valE = valB / valA; break;
        case FUN_MODL: addAction("valE <- valB % valA"); valE = valB % valA; break;
        case FUN_ANDL: addAction("valE <- valB & valA"); valE = valB & valA; break;
        case FUN_ORL:  addAction("valE <- valB | valA"); valE = valB | valA; break;
        case FUN_XORL: addAction("valE <- valB ^ valA"); valE = valB ^ valA; break;
        }
        // TODO: SetCC
        break;

    case OP_JMP:
        // TODO
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
    }

    writeWire("E_icode", icode);
    writeWire("E_valP", valP);
    writeWire("E_dstE", dstE);
    writeWire("E_valE", valE);
}
