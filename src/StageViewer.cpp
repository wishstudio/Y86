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
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);

    this->id = id;
    inWires = VM::worker(id)->inWires();

    QGridLayout *layout = new QGridLayout(this);
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(5);
    layout->setMargin(3);

    typedef HexWidget *PHexWidget; /* compilation hack */
    wireLabels = new PHexWidget[inWires.size()];
    for (int i = 0; i < inWires.size(); i++)
    {
        QLabel *label = new QLabel(omitStageName(inWires.at(i)), this);
        label->setFont(font);
        wireLabels[i] = new HexWidget(this);
        wireLabels[i]->setBits(VM::wireBits(omitStageName(inWires.at(i))));
        layout->addWidget(label, 0, i, Qt::AlignLeft);
        layout->addWidget(wireLabels[i], 1, i, Qt::AlignLeft);
        layout->setColumnStretch(i, 0);
    }
    layout->setColumnStretch(inWires.size() - 1, 1);

    QHBoxLayout *actionsLayout = new QHBoxLayout();
    for (int i = 0; i < MAX_ACTIONS; i++)
    {
        actionLabels[i] = new QLabel(this);
        actionLabels[i]->setFont(font);
        actionsLayout->addWidget(actionLabels[i]);
    }
    layout->addLayout(actionsLayout, 2, 0, 1, inWires.size());

    setLayout(layout);
}

StageViewer::~StageViewer()
{
    delete[] wireLabels;
}

void StageViewer::updateDisplay()
{
    QStringList actions = VM::worker(id)->workerActions();
    for (int i = 0; i < MAX_ACTIONS; i++)
        actionLabels[i]->setText("");
    for (int i = 0; i < actions.size(); i++)
        actionLabels[i]->setText(actions.at(i));

    for (int i = 0; i < inWires.size(); i++)
        wireLabels[i]->setNumber(VM::wireForRead()->readWire(inWires.at(i)));
}

QString StageViewer::omitStageName(const QString &wire)
{
    int p = wire.indexOf("_");
    if (p == -1)
        return wire;
    else
        return wire.mid(p + 1);
}
