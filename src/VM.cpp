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

#include "Assembler.h"
#include "VM.h"

#define STAGE_F    0
#define STAGE_D    1
#define STAGE_E    2
#define STAGE_M    3
#define STAGE_W    4

const QString stageNames[5] = {"F", "D", "E", "M", "W"};

Q_GLOBAL_STATIC(VM, d)

static int readInt(const QByteArray &bytes, int addr)
{
    return * (int *) bytes.constData();
}

VM::VM()
{
    m_memory = new Memory();
    m_reg = new Wire();
    m_wire = new Wire();
    m_nextWire = new Wire();
    m_workerSemaphore = new QSemaphore();
    m_monitorSemaphore = new QSemaphore();
}

VM::~VM()
{
    delete m_memory;
    delete m_reg;
    delete m_wire;
    delete m_nextWire;
}

void VM::init()
{
    for (int i = 0; i < WORKERS_COUNT; i++)
        d()->stageWorkers[i] = new VMWorker(i, QString(":/components/Stage_%1.js").arg(stageNames[i]));
}

VM *VM::self()
{
    return d();
}

VMWorker *VM::worker(int id)
{
    return d()->stageWorkers[id];
}

QSemaphore *VM::workerSemaphore()
{
    return d()->m_workerSemaphore;
}

QSemaphore *VM::monitorSemaphore()
{
    return d()->m_monitorSemaphore;
}

Memory *VM::memory()
{
    return d()->m_memory;
}

Wire *VM::reg()
{
    return d()->m_reg;
}

Wire *VM::wireForRead()
{
    return d()->m_wire;
}

Wire *VM::wireForWrite()
{
    return d()->m_nextWire;
}

void VM::reserveWire(const QString &wire)
{
    d()->m_wire->reserve(wire);
    d()->m_nextWire->reserve(wire);
}

void VM::loadObject(const QString &fileName)
{
    Assembler::compileFile(fileName, d()->m_memory);
    d()->m_wire->clear();
    d()->m_wire->writeWire("F_valP", Assembler::startEIP());
    d()->m_reg->clear();
    d()->m_reg->writeWire("esp", Assembler::startESP());
}

void VM::step()
{
}

void VM::startRunning()
{
}

void VM::run()
{
    m_nextWire->clearState();
    for (int i = 0; i < WORKERS_COUNT; i++)
        stageWorkers[i]->start();
    for (;;)
    {
        m_workerSemaphore->acquire(WORKERS_COUNT);
        m_wire->copyFrom(m_nextWire);
        m_nextWire->clearState();
        m_monitorSemaphore->release(WORKERS_COUNT);
    }
}
