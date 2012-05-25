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
    static VMWorker *worker(int id);
    static QSemaphore *workerSemaphore();
    static QSemaphore *monitorSemaphore();
    static Memory *memory();
    static Wire *reg();
    static Wire *wireForRead();
    static Wire *wireForWrite();
    static void reserveWire(const QString &wire);

    static void loadObject(const QString &fileName);
    static void step();
    static void startRunning();

    void run();

private:
    QSemaphore *m_workerSemaphore, *m_monitorSemaphore;
    Memory *m_memory;
    Wire *m_reg, *m_wire, *m_nextWire;
    VMWorker *stageWorkers[WORKERS_COUNT];
};

#endif
