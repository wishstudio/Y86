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

#define STAGE_F     0
#define STAGE_D     1
#define STAGE_E     2
#define STAGE_M     3
#define STAGE_W     4

#define TYPE_NONE       0
#define TYPE_ICODE      1
#define TYPE_IFUN_OP    2
#define TYPE_IFUN_JMP   3
#define TYPE_REGISTER   4
#define TYPE_EXCEPTION  5
#define TYPE_MEMORY     6

const QString stageNames[] = {"F", "D", "E", "M", "W"};

Q_GLOBAL_STATIC(VM, d)

static int readInt(const QByteArray &bytes, int addr)
{
    return * (int *) bytes.constData();
}

VM::VM()
{
    m_codeListModel = new CodeListModel();
    connect(this, SIGNAL(updateDisplay()), m_codeListModel, SLOT(updateDisplay()));
    m_stackListModel = new StackListModel();
    connect(this, SIGNAL(updateDisplay()), m_stackListModel, SLOT(updateDisplay()));

    m_memory = new Memory();
    m_reg = new Register();
    m_wire = new Wire();
    m_nextWire = new Wire();
    for (int i = 0; i < WORKERS_COUNT; i++)
        m_workerSemaphore[i] = new QSemaphore();
    m_monitorSemaphore = new QSemaphore();
    m_halted = true;
    m_cycleCount = 0;
    m_instructionCount = 0;
    m_symbolLookupTable.clear();

    QFile file(":/components/Definitions.js");
    file.open(QIODevice::ReadOnly);
    QScriptProgram program = QString(file.readAll());
    file.close();
    m_engine = new QScriptEngine();
    m_engine->globalObject().setProperty("TYPE_NONE", TYPE_NONE);
    m_engine->globalObject().setProperty("TYPE_ICODE", TYPE_ICODE);
    m_engine->globalObject().setProperty("TYPE_IFUN_OP", TYPE_IFUN_OP);
    m_engine->globalObject().setProperty("TYPE_IFUN_JMP", TYPE_IFUN_JMP);
    m_engine->globalObject().setProperty("TYPE_REGISTER", TYPE_REGISTER);
    m_engine->globalObject().setProperty("TYPE_EXCEPTION", TYPE_EXCEPTION);
    m_engine->globalObject().setProperty("TYPE_MEMORY", TYPE_MEMORY);
    for (int i = 0; i < OP_CNT; i++)
        m_engine->globalObject().setProperty(("OP_" + opNames[i]).toUpper(), i);
    m_engine->evaluate(program);
}

VM::~VM()
{
    delete m_codeListModel;
    delete m_stackListModel;
    delete m_memory;
    delete m_reg;
    delete m_wire;
    delete m_nextWire;
    for (int i = 0; i < WORKERS_COUNT; i++)
        delete m_workerSemaphore[i];
    delete m_monitorSemaphore;
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

int VM::workerAddr(int id)
{
    return d()->m_workerAddr[id];
}

QSemaphore *VM::workerSemaphore(int id)
{
    return d()->m_workerSemaphore[id];
}

QSemaphore *VM::monitorSemaphore()
{
    return d()->m_monitorSemaphore;
}

CodeListModel *VM::codeListModel()
{
    return d()->m_codeListModel;
}

StackListModel *VM::stackListModel()
{
    return d()->m_stackListModel;
}

bool VM::isHalted()
{
    return d()->m_halted;
}

int VM::cycleCount()
{
    return d()->m_cycleCount;
}

int VM::instructionCount()
{
    return d()->m_instructionCount;
}

void VM::increaseInstructionCount()
{
    d()->m_instructionCount++;
}

Memory *VM::memory()
{
    return d()->m_memory;
}

Register *VM::reg()
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

int VM::wireBits(const QString &wire)
{
    return d()->m_engine->globalObject().property("getBits").call(QScriptValue(), QScriptValueList() << wire).toInt32();
}

QString VM::wireDescription(int icode, const QString &wire, int value)
{
    int type = d()->m_engine->globalObject().property("getType").call(QScriptValue(), QScriptValueList() << icode << wire).toInt32();
    switch (type)
    {
    case TYPE_ICODE:
        if (value >= 0 && value < OP_CNT)
            return opNames[value];
        break;

    case TYPE_IFUN_OP:
        if (value >= 0 && value < FUN_OPL_CNT)
            return funOplNames[value];
        break;

    case TYPE_IFUN_JMP:
        if (value >= 0 && value < FUN_JMP_CNT)
            return funJmpNames[value];
        break;

    case TYPE_REGISTER:
        if (value >= 0 && value < REG_CNT)
            return registerNames[value];
        break;

    case TYPE_EXCEPTION:
        if (value >= 0 && value < EXCEP_CNT)
            return exceptionNames[value];
        break;

    case TYPE_MEMORY:
        if (d()->m_symbolLookupTable.contains(value))
            return d()->m_symbolLookupTable.value(value);
        break;
    }
    return QString();
}

bool VM::loadObject(const QString &fileName)
{
    if (!Assembler::compileFile(fileName, d()->m_memory))
    {
        d()->clearVM();
        d()->m_codeListModel->setMemory(Assembler::code(), Assembler::memoryRef(), Assembler::startStack());
        d()->m_stackListModel->setStartStack(Assembler::startStack());
        d()->m_symbolLookupTable.clear();
        emit d()->updateDisplay();
        return false;
    }

    /* power up our machine */
    d()->m_wire->clear();
    d()->m_reg->clear();
    /* eip and esp */
    for (int i = 0; i < WORKERS_COUNT; i++)
        d()->m_wire->writeWire(stageNames[i] + "_eip", -2);
    d()->m_wire->writeWire("F_predPC", Assembler::startEIP());
    d()->m_reg->writeRegister(REG_ESP, Assembler::startESP());
    d()->m_reg->writeRegister(REG_IDTR, 0xFFFF0000);
    /* clear forwarding registers */
    d()->m_wire->writeWire("E_dstE", REG_NONE);
    d()->m_wire->writeWire("M_dstE", REG_NONE);
    d()->m_wire->writeWire("W_dstE", REG_NONE);
    d()->m_codeListModel->setMemory(Assembler::code(), Assembler::memoryRef(), Assembler::startStack());
    d()->m_stackListModel->setStartStack(Assembler::startStack());

    for (int i = 0; i < WORKERS_COUNT; i++)
    {
        d()->m_workerAddr[i] = -2;
        d()->stageWorkers[i]->clearWorkerActions();
    }
    d()->m_halted = false;
    d()->m_cycleCount = 0;
    d()->m_instructionCount = 0;
    d()->m_symbolLookupTable = Assembler::symbolLookupTable();
    emit d()->updateDisplay();
    return true;
}

void VM::step()
{
    while (d()->isRunning());
    d()->m_stop = true;
    d()->start();
}

void VM::startVM()
{
    d()->m_stop = false;
    d()->start();
}

void VM::stopVM()
{
    d()->m_stop = true;
}

void VM::haltVM()
{
    d()->m_stop = true;
    d()->m_halted = true;
}

void VM::setFrequency(int freq)
{
    d()->m_frequency = freq;
}

void VM::run()
{
    m_nextWire->clearState();
    for (int i = 0; i < WORKERS_COUNT; i++)
        stageWorkers[i]->start();
    for (;;)
    {
        m_monitorSemaphore->acquire(WORKERS_COUNT);
        /* sync and run control() */
        for (int i = 0; i < WORKERS_COUNT; i++)
            m_workerSemaphore[i]->release();
        m_monitorSemaphore->acquire(WORKERS_COUNT);
        m_workerAddr[0] = m_nextWire->readWire(stageNames[0] + "_eip");
        for (int i = 1; i < WORKERS_COUNT; i++)
            m_workerAddr[i] = m_wire->readWire(stageNames[i] + "_eip");
        m_wire->copyFrom(m_nextWire);
        m_nextWire->clearState();
        m_cycleCount++;
        emit updateDisplay();
        if (m_stop)
        {
            for (int i = 0; i < WORKERS_COUNT; i++)
            {
                stageWorkers[i]->stopWorker();
                m_workerSemaphore[i]->release();
                /* wait for stage worker to stop */
                while (!stageWorkers[i]->isRunning())
                    /* spin lock */;
            }
            break;
        }
        if (m_frequency)
            msleep(1000 / m_frequency);
        for (int i = 0; i < WORKERS_COUNT; i++)
            m_workerSemaphore[i]->release();
    }
}

void VM::clearVM()
{
    m_memory->clear();
    m_reg->clear();
    m_wire->clear();
    m_nextWire->clear();
    m_halted = true;
    m_cycleCount = 0;
    m_instructionCount = 0;
    m_symbolLookupTable.clear();
}
