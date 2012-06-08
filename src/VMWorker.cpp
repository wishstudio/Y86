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

static QScriptValue int32(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    return QScriptValue(engine, context->argument(0).toInt32());
}

static QScriptValue uint32(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    return QScriptValue(engine, context->argument(0).toUInt32());
}

static QScriptValue debug(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    qDebug("%s", qPrintable(context->argument(0).toString()));
    return engine->undefinedValue();
}

static QScriptValue readWire(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    return VM::wireForRead()->readWire(context->argument(0).toString());
}

static QScriptValue readForwardingWire(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    return VM::wireForWrite()->readForwardingWire(context->argument(0).toString());
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

static QScriptValue clearAction(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0)
        return engine->undefinedValue();
    VM::worker(engine->globalObject().property("__id").toInt32())->clearWorkerActions();
    return engine->undefinedValue();
}

static QScriptValue addAction(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1)
        return engine->undefinedValue();
    VM::worker(engine->globalObject().property("__id").toInt32())->addWorkerAction(context->argument(0).toString());
    return engine->undefinedValue();
}

/* a convenient function for stalling a pipeline stage. */
/* just make sure the input wires do not change */
static QScriptValue stall(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 0)
        return engine->undefinedValue();
    int id = engine->globalObject().property("__id").toInt32();
    VM::worker(id)->clearWorkerActions();
    VM::worker(id)->addWorkerAction("stall");
    QStringList inWires = VM::worker(id)->inWires();
    foreach (QString wire, inWires)
        VM::wireForWrite()->writeWire(wire, VM::wireForRead()->readWire(wire));
    /* don't need to copy eip on stage F when stalling */
    if (id > 0)
        VM::wireForWrite()->writeWire(stageNames[id] + "_eip", VM::wireForRead()->readWire(stageNames[id] + "_eip"));
    return engine->undefinedValue();
}

VMWorker::VMWorker(int id, const QString &fileName)
{
    this->id = id;

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QScriptProgram program = QString(file.readAll());
    file.close();

    engine = new QScriptEngine();
    QScriptValue global = engine->globalObject();
    global.setProperty("__id", id, QScriptValue::ReadOnly);
    global.setProperty("int32", engine->newFunction(int32));
    global.setProperty("uint32", engine->newFunction(uint32));
    global.setProperty("debug", engine->newFunction(debug));
    global.setProperty("readWire", engine->newFunction(readWire));
    global.setProperty("readForwardingWire", engine->newFunction(readForwardingWire));
    global.setProperty("writeWire", engine->newFunction(writeWire));
    global.setProperty("readRegister", engine->newFunction(readRegister));
    global.setProperty("writeRegister", engine->newFunction(writeRegister));
    global.setProperty("readMemoryChar", engine->newFunction(readMemoryChar));
    global.setProperty("readMemoryInt", engine->newFunction(readMemoryInt));
    global.setProperty("writeMemoryInt", engine->newFunction(writeMemoryInt));
    global.setProperty("clearAction", engine->newFunction(clearAction));
    global.setProperty("addAction", engine->newFunction(addAction));
    global.setProperty("stall", engine->newFunction(stall));

    /* instruction */
    for (int i = 0; i < OP_CNT; i++)
        global.setProperty("OP_" + opNames[i].toUpper(), i);

    /* operation */
    for (int i = 0; i < FUN_OPL_CNT; i++)
        global.setProperty("FUN_" + funOplNames[i].toUpper(), i);

    /* Jcc */
    for (int i = 0; i < FUN_JMP_CNT; i++)
        global.setProperty("FUN_" + funJmpNames[i].toUpper(), i);

    /* exceptions */
    for (int i = 0; i < EXCEP_CNT; i++)
        global.setProperty("EXCEP_" + exceptionNames[i].toUpper(), i);

    /* registers */
    for (int i = 0; i < REG_CNT; i++)
        global.setProperty("REG_" + registerNames[i].toUpper(), i);

    /* eflags */
    global.setProperty("REG_EFLAGS", REG_NONE);
    for (int i = 0; i < EFLAGS_CNT; i++)
        global.setProperty("EFLAGS_" + eflagsNames[i].toUpper(), i);
    engine->evaluate(program);

    QScriptValue in = global.property("inWires").call();
    int in_cnt = in.property("length").toInt32();
    for (int i = 0; i < in_cnt; i++)
        m_inWires.push_back(in.property(i).toString());

    QScriptValue out = global.property("outWires").call();
    int out_cnt = out.property("length").toInt32();
    for (int i = 0; i < out_cnt; i++)
        m_outWires.push_back(out.property(i).toString());

    foreach (QString wire, m_inWires)
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

void VMWorker::clearWorkerActions()
{
    actions.clear();
}

void VMWorker::addWorkerAction(const QString &action)
{
    actions.push_back(action);
}

QStringList VMWorker::workerActions()
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
        VM::monitorSemaphore()->release();
        VM::workerSemaphore(id)->acquire();
        engine->globalObject().property("control").call();
        VM::monitorSemaphore()->release();
        VM::workerSemaphore(id)->acquire();
        if (shouldStop)
            break;
    }
}
