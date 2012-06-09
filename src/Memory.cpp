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
    writeMask.clear();
    executeMask.clear();
    currentWriteMask = false;
    currentExecuteMask = false;
}

int Memory::addr() const
{
    return mem.size();
}

void Memory::setOrigin(int origin)
{
    int oldOrigin = mem.size();
    mem.resize(origin);
    writeMask.resize(origin / 32 + 1);
    executeMask.resize(origin / 32 + 1);
    for (int i = oldOrigin; i < origin; i++)
    {
        mem[i] = 0;
        if (currentWriteMask)
            writeMask[i / 32] |= 1 << (i % 32 - 1);
        else
            writeMask[i / 32] &= ~(1 << (i % 32 - 1));
        if (currentExecuteMask)
            executeMask[i / 32] |= 1 << (i % 32 - 1);
        else
            executeMask[i / 32] &= ~(1 << (i % 32 - 1));
    }
}

void Memory::setAttr(bool canWrite, bool canExecute)
{
    currentWriteMask = canWrite;
    currentExecuteMask = canExecute;
}

void Memory::putChar(char value)
{
    mem.push_back(value);
    writeMask.resize(mem.size() / 32 + 1);
    executeMask.resize(mem.size() / 32 + 1);
    if (currentWriteMask)
        writeMask[mem.size() / 32] |= 1 << (mem.size() % 32 - 1);
    else
        writeMask[mem.size() / 32] &= ~(1 << (mem.size() % 32 - 1));
    if (currentExecuteMask)
        executeMask[mem.size() / 32] |= 1 << (mem.size() % 32 - 1);
    else
        executeMask[mem.size() / 32] &= ~(1 << (mem.size() % 32 - 1));
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

bool Memory::canReadChar(int addr) const
{
    return addr >= 0 && addr < mem.size();
}

bool Memory::canReadInt(int addr) const
{
    return addr >= 0 && addr + 3 < mem.size();
}

bool Memory::canExecuteChar(int addr) const
{
    return canReadChar(addr) && (executeMask[addr / 32] & (1 << (addr % 32)));
}

bool Memory::canExecuteInt(int addr) const
{
    if (!canReadInt(addr))
        return false;
    for (int i = 0; i < 4; i++)
        if (!(executeMask[(i + addr) / 32] & (1 << ((i + addr) % 32))))
            return false;
    return true;
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
        if (!(writeMask[(i + addr) / 32] & (1 << ((i + addr) % 32))))
            return false;
    *(int *) (mem.data() + addr) = value;
    return true;
}
