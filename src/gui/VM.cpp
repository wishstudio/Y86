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

#include <QFile>

#include "VM.h"

static int readInt(const QByteArray &bytes, int addr)
{
    return * (int *) bytes.constData();
}

VM::VM()
{
    memory = new Memory();
    wire = new Wire();
}

VM::~VM()
{
    delete memory;
    delete wire;
}

void VM::loadObject(const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray content = file.readAll();
    file.close();
    if (QString(content.left(5)) != "YOBJ")
        return;
    int start_eip = readInt(content, 4);
    int start_esp = readInt(content, 8);
    int memorySize = readInt(content, 12);
    memory->initMemory(memorySize);

    for (int i = 20; i < content.size();)
    {
        int attr = readInt(content, i);
        int origin = readInt(content, i + 4);
        int length = readInt(content, i + 8);
        i += 12;
        if (attr & 2) // placeholder
            memory->initSegment(origin, length, attr);
        else
        {
            memory->initSegment(origin, content.mid(i, length), attr);
            i += length;
        }
    }
}
