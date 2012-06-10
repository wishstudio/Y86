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

#ifndef VM_H
#define VM_H

#include <QThread>
#include <QSemaphore>

#include "CodeListModel.h"
#include "Register.h"
#include "StackListModel.h"
#include "VMWorker.h"
#include "Memory.h"
#include "Wire.h"

#define WORKERS_COUNT     5

class VM: public QThread
{
    Q_OBJECT

public:
    VM();
    virtual ~VM();

    static void init();
    static VM *self();
    static CodeListModel *codeListModel();
    static StackListModel *stackListModel();
    static bool isHalted();
    static int cycleCount();
    static int instructionCount();
    static void increaseInstructionCount();
    static VMWorker *worker(int id);
    static int workerAddr(int id);
    static QSemaphore *workerSemaphore(int id);
    static QSemaphore *monitorSemaphore();
    static Memory *memory();
    static Register *reg();
    static Wire *wireForRead();
    static Wire *wireForWrite();
    static void reserveWire(const QString &wire);
    static int wireBits(const QString &wire);

    static void loadObject(const QString &fileName);
    static void step();
    static void startVM();
    static void stopVM();
    static void haltVM();
    static void setFrequency(int freq);

    void run();

signals:
    void updateDisplay();

private:
    QSemaphore *m_workerSemaphore[WORKERS_COUNT];
    QSemaphore *m_monitorSemaphore;
    CodeListModel *m_codeListModel;
    StackListModel *m_stackListModel;
    Memory *m_memory;
    Register *m_reg;
    Wire *m_wire, *m_nextWire;
    bool m_stop, m_halted;
    VMWorker *stageWorkers[WORKERS_COUNT];
    int m_workerAddr[WORKERS_COUNT];
    int m_frequency, m_cycleCount, m_instructionCount;

    QScriptEngine *m_engine;
};

extern const QString stageNames[];

#endif
