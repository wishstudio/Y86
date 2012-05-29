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
    return ["W_icode", "W_dstE", "W_valE", "W_valM"];
}

function outWires()
{
    return [];
}

function cycle()
{
    var icode = readWire("W_icode");
    var dstE = readWire("W_dstE");
    var valE = readWire("W_valE");
    var dstM = readWire("W_dstM");
    var valM = readWire("W_valM");

    switch (icode)
    {
    case OP_RRMOVL:
    case OP_IRMOVL:
    case OP_OPL:
    case OP_RET:
    case OP_PUSHL:
        addAction("R[dstE] <- valE");
        writeRegister(dstE, valE);
        break;

    case OP_MRMOVL:
        addAction("R[dstM] <- valM");
        writeRegister(dstM, valM);
        break;

    case OP_POPL:
        addAction("R[%esp] <- valE");
        addAction("R[dstM] <- valM");
        writeRegister(REG_ESP, valE);
        writeRegister(dstM, valM);
        break;
    }
}

function control()
{
}
