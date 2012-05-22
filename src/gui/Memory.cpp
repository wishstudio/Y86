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

#include "Memory.h"

Memory::Memory()
{
    mem = 0;
}

Memory::~Memory()
{
}

void Memory::initMemory(int size)
{
    mem = new char[size];
    int maskLen = (size + 31) / 32;
    attrMask = new int[maskLen];
    memset(attrMask, 0, maskLen * 4);
}

void Memory::initSegment(int origin, const QByteArray &content, int attr)
{
    for (int i = 0; i < content.size(); i++)
    {
        mem[i + origin] = content.at(i);
        if (attr)
            attrMask[(i + origin) / 32] |= (i + origin) % 32;
    }
}

void Memory::initSegment(int origin, int size, int attr)
{
    if (attr)
        for (int i = 0; i < size; i++)
            attrMask[(i + origin) / 32] |= (i + origin) % 32;
}

char Memory::readChar(int addr) const
{
    return mem[addr];
}

int Memory::readInt(int addr) const
{
    return *(int *) mem[addr];
}

bool Memory::writeInt(int addr, int value)
{
    for (int i = 0; i < 4; i++)
        if (!(attrMask[(i + addr) / 32] & ((i + addr) % 32)))
            return false;
    *(int *) &mem[addr] = value;
    return true;
}
