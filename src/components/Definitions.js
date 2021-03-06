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

function getBits(name)
{
    if (name == "icode" || name == "ifun" || name == "Bch" || name == "rA" || name == "rB" || name == "dstE" || name == "dstM")
        return 1;
    return 8;
}

function getType(icode, name)
{
    if (name == "icode")
        return TYPE_ICODE;
    else if (name == "ifun")
    {
        switch (icode)
        {
        case OP_OP:
            return OP_IFUN_OP;

        case OP_JMP:
            return OP_IFUN_JMP;

        case OP_INT:
        case OP_EXCEP:
            return OP_EXCEPTION;
        }
    }
    else if (name == "rA" || name == "rB" || name == "dstE" || name == "dstM")
        return TYPE_REGISTER;
    else if (name == "valP" || name == "predPC")
        return TYPE_MEMORY;
    return TYPE_NONE;
}
