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
#include <QSemaphore>

#include "Assembler.h"

#include "VM.h"
#include "VMWorker.h"

static QScriptValue debug(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    qDebug("%s\n", qPrintable(context->argument(0).toString()));
    return engine->undefinedValue();
}

static QScriptValue readWire(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    return VM::wireForRead()->readWire(context->argument(0).toString());
}

static QScriptValue writeWire(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2)
        return engine->undefinedValue();
    VM::wireForWrite()->writeWire(context->argument(0).toString(), context->argument(1).toInt32());
    return engine->undefinedValue();
}

static QScriptValue readRegister(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    return VM::reg()->readRegister(context->argument(0).toInt32());
}

static QScriptValue writeRegister(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2)
        return engine->undefinedValue();
    VM::reg()->writeRegister(context->argument(0).toInt32(), context->argument(1).toInt32());
    return engine->undefinedValue();
}

static QScriptValue readMemoryChar(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    return VM::memory()->readChar(context->argument(0).toInt32());
}

static QScriptValue readMemoryInt(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    return VM::memory()->readInt(context->argument(0).toInt32());
}

static QScriptValue writeMemoryInt(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2)
        return engine->undefinedValue();
    return VM::memory()->writeInt(context->argument(0).toInt32(), context->argument(1).toInt32());
}

static QScriptValue addAction(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    VM::worker(engine->globalObject().property("__id").toInt32())->addWorkerAction(context->argument(0).toString());
    return engine->undefinedValue();
}

VMWorker::VMWorker(int id, const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QScriptProgram program = QString(file.readAll());
    file.close();

    engine = new QScriptEngine();
    QScriptValue global = engine->globalObject();
    global.setProperty("__id", id, QScriptValue::ReadOnly);
    global.setProperty("readWire", engine->newFunction(readWire));
    global.setProperty("writeWire", engine->newFunction(writeWire));
    global.setProperty("readRegister", engine->newFunction(readRegister));
    global.setProperty("writeRegister", engine->newFunction(writeRegister));
    global.setProperty("readMemoryChar", engine->newFunction(readMemoryChar));
    global.setProperty("readMemoryInt", engine->newFunction(readMemoryInt));
    global.setProperty("writeMemoryInt", engine->newFunction(writeMemoryInt));
    global.setProperty("addAction", engine->newFunction(addAction));

    global.setProperty("OP_NOP", OP_NOP);
    global.setProperty("OP_HALT", OP_HALT);
    global.setProperty("OP_RRMOVL", OP_RRMOVL);
    global.setProperty("OP_IRMOVL", OP_IRMOVL);
    global.setProperty("OP_RMMOVL", OP_RMMOVL);
    global.setProperty("OP_MRMOVL", OP_MRMOVL);
    global.setProperty("OP_OP", OP_OP);
    global.setProperty("OP_JMP", OP_JMP);
    global.setProperty("OP_CALL", OP_CALL);
    global.setProperty("OP_RET", OP_RET);
    global.setProperty("OP_PUSHL", OP_PUSHL);
    global.setProperty("OP_POPL", OP_POPL);
    engine->evaluate(program);

    QScriptValue in = global.property("inWires").call();
    int in_cnt = in.property("length").toInt32();
    for (int i = 0; i < in_cnt; i++)
        m_inWires.push_back(in.property(i).toString());

    QScriptValue out = global.property("outWires").call();
    int out_cnt = in.property("length").toInt32();
    for (int i = 0; i < out_cnt; i++)
        m_outWires.push_back(out.property(i).toString());

    foreach (const QString &wire, m_inWires)
        VM::reserveWire(wire);

    foreach (QString wire, m_outWires)
        VM::reserveWire(wire);
}

VMWorker::~VMWorker()
{
    delete engine;
}

void VMWorker::stopWorker()
{
    shouldStop = true;
}

void VMWorker::addWorkerAction(const QString &action)
{
    actions.push_back(action);
}

QStringList VMWorker::workerActions() const
{
    return actions;
}

void VMWorker::run()
{
    shouldStop = false;
    for (;;)
    {
        actions.clear();
        engine->globalObject().property("cycle").call();
        VM::workerSemaphore()->release();
        VM::monitorSemaphore()->acquire();
        if (shouldStop)
            break;
    }
}
