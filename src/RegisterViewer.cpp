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
#include <QLabel>

#include "Register.h"
#include "RegisterViewer.h"
#include "VM.h"

RegisterViewer::RegisterViewer(QWidget *parent)
    : QGroupBox(parent)
{
    setTitle("Registers");

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);

    QGridLayout *layout = new QGridLayout(this);
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(5);

    for (int i = 0; i < 8; i++)
    {
        QLabel *label = new QLabel(QString("%%1").arg(registerNames[i]), this);
        label->setFont(font);

        registerLabels[i] = new HexWidget(this);
        registerLabels[i]->setBits(8);
        layout->addWidget(label, i / 4 * 2, i % 4);
        layout->addWidget(registerLabels[i], i / 4 * 2 + 1, i % 4);
    }

    {
        QLabel *label = new QLabel(QString("%%1").arg(registerNames[REG_IDTR]), this);
        label->setFont(font);

        registerLabels[REG_IDTR] = new HexWidget(this);
        registerLabels[REG_IDTR]->setBits(8);
        layout->addWidget(label, 0, 4);
        layout->addWidget(registerLabels[REG_IDTR], 1, 4);
    }

    {
        QLabel *label = new QLabel(QString("%%1").arg(registerNames[REG_EFLAGS]), this);
        label->setFont(font);

        registerLabels[REG_EFLAGS] = new HexWidget(this);
        registerLabels[REG_EFLAGS]->setBits(8);
        layout->addWidget(label, 2, 4);
        layout->addWidget(registerLabels[REG_EFLAGS], 3, 4);
    }
    setLayout(layout);
}

void RegisterViewer::updateDisplay()
{
    for (int i = 0; i < REG_CNT; i++)
        if (i != REG_NONE)
            registerLabels[i]->setNumber(VM::reg()->readRegister(i));
}
