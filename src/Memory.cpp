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
}

Memory::~Memory()
{
}

void Memory::clear()
{
    mem.clear();
    attrMask.clear();
}

int Memory::addr() const
{
    return mem.size();
}

void Memory::setOrigin(int origin)
{
    int oldOrigin = mem.size();
    mem.resize(origin);
    attrMask.resize((origin + 31) / 32);
    for (int i = oldOrigin; i < origin; i++)
    {
        mem[i] = 0;
        if (currentAttr)
            attrMask[i / 32] |= 1 << (i % 32);
        else
            attrMask[i / 32] &= ~(1 << (i % 32));
    }
}

void Memory::setAttr(bool attr)
{
    currentAttr = attr;
}

void Memory::putChar(char value)
{
    mem.push_back(value);
    attrMask.resize((mem.size() + 31) / 32);
    if (currentAttr)
        attrMask[mem.size() / 32] |= 1 << (mem.size() % 32);
    else
        attrMask[mem.size() / 32] &= ~(1 << (mem.size() % 32));
}

void Memory::putShort(short value)
{
    char *p = (char *) &value;
    for (int i = 0; i < 2; i++)
        putChar(p[i]);
}

void Memory::put(int value)
{
    char *p = (char *) &value;
    for (int i = 0; i < 4; i++)
        putChar(p[i]);
}

void Memory::patch(int addr, int value)
{
    char *p = (char *) &value;
    for (int i = 0; i < 4; i++)
        mem[addr + i] = p[i];
}

char Memory::readChar(int addr) const
{
    return mem[addr];
}

int Memory::readInt(int addr) const
{
    return *(int *) (mem.data() + addr);
}

bool Memory::writeInt(int addr, int value)
{
    for (int i = 0; i < 4; i++)
        if (!(attrMask[(i + addr) / 32] & (1 << ((i + addr) % 32))))
            return false;
    *(int *) (mem.data() + addr) = value;
    return true;
}
