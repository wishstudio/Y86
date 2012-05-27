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

#ifndef VMWORKER_H
#define VMWORKER_H

#include <QThread>
#include <QScriptEngine>
#include <QStringList>

#include "Memory.h"
#include "Wire.h"

class VMWorker: public QThread
{
    Q_OBJECT

public:
    VMWorker(int id, const QString &fileName);
    virtual ~VMWorker();

    QStringList inWires() const { return m_inWires; }
    QStringList outWires() const { return m_outWires; }
    void stopWorker();
    void clearWorkerActions();
    void addWorkerAction(const QString &action);
    QStringList workerActions();
    void cycle();
    void run();

private:
    int id;
    QScriptEngine *engine;
    bool shouldStop;
    QStringList actions;
    QStringList m_inWires, m_outWires;
};

#endif
