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
    outWires = VM::worker(id)->outWires();

    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(0);

    layout->addWidget(new QLabel("Actions:", this), 0, 0);
    for (int i = 0; i < 4; i++)
    {
        actionLabels[i] = new QLabel("", this);
        layout->addWidget(actionLabels[i], 0, i + 1);
    }
    layout->addWidget(new QLabel("Wires:", this), 1, 0);
    for (int i = 0; i < 20; i++)
    {
        wireLabels[i] = new QLabel("", this);
        layout->addWidget(wireLabels[i], (i / 4) + 1, (i % 4) + 1);
    }

    setLayout(layout);
}

StageViewer::~StageViewer()
{
}

void StageViewer::updateDisplay(QStringList actionString)
{
    for (int i = 0; i < actionString.size(); i++)
        actionLabels[i]->setText(actionString.at(i));
    int j = 0;
    for (int i = 0; i < 20; i++)
        wireLabels[i]->setText("");
    foreach (QString wire, outWires)
        if (VM::wireForRead()->state(wire))
            wireLabels[j++]->setText(wire + ": " + QString::number(VM::wireForRead()->readWire(wire)));
}
