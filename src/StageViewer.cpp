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

#include <QGridLayout>

#include "StageViewer.h"
#include "VM.h"
#include "VMWorker.h"

StageViewer::StageViewer(int id, QWidget *parent)
    : QWidget(parent)
{
    this->id = id;
    inWires = VM::worker(id)->inWires();

    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(0);

    layout->addWidget(new QLabel("Actions", this), 0, 0);
    for (int i = 0; i < ROW_COUNT; i++)
    {
        actionLabels[i] = new QLabel("", this);
        layout->addWidget(actionLabels[i], i + 1, 0);
    }
    layout->addWidget(new QLabel("Wire outputs", this), 0, 1);
    typedef QLabel *PQLabel; /* compilation hack */
    wireLabels = new PQLabel[inWires.size()];
    for (int i = 0; i < inWires.size(); i++)
    {
        wireLabels[i] = new QLabel("", this);
        layout->addWidget(wireLabels[i], i % ROW_COUNT + 1, i / ROW_COUNT + 1);
    }

    setLayout(layout);
}

StageViewer::~StageViewer()
{
    delete[] wireLabels;
}

void StageViewer::updateDisplay()
{
    QStringList actions = VM::worker(id)->workerActions();
    for (int i = 0; i < ROW_COUNT; i++)
        actionLabels[i]->setText("");
    for (int i = 0; i < actions.size(); i++)
        actionLabels[i]->setText(actions.at(i));
    for (int i = 0; i < inWires.size(); i++)
    {
        const QString &wire = inWires.at(i);
        wireLabels[i]->setText(wire + ": " + QString::number(VM::wireForRead()->readWire(wire)));
    }
}
