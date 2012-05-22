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

#include "segment.h"

Segment::Segment(int _origin)
    : m_origin(_origin)
{
}

int Segment::addr() const
{
    return mem.size();
}

void Segment::patch(int addr, int value)
{
    *((int *) &mem[addr]) += value;
}

void Segment::put(int value)
{
    for (int i = 0; i < 4; i++)
        mem.push_back(*(((char *) &value) + i));
}

void Segment::putChar(char value)
{
    mem.push_back(value);
}
