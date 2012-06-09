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

#include <QFormLayout>

#include "Register.h"
#include "RegisterViewer.h"
#include "VM.h"

RegisterViewer::RegisterViewer(QWidget *parent)
    : QWidget(parent)
{
    QFormLayout *layout = new QFormLayout(this);
    for (int i = 0; i < REG_CNT; i++)
    {
        if (i == REG_NONE)
            continue;
        registerLabels[i] = new HexWidget(this);
        registerLabels[i]->setBits(8);
        layout->addRow(QString("%%1: ").arg(registerNames[i]), registerLabels[i]);
    }
    setLayout(layout);
}

void RegisterViewer::updateDisplay()
{
    for (int i = 0; i < REG_CNT; i++)
        if (i != REG_NONE)
            registerLabels[i]->setNumber(VM::reg()->readRegister(i));
}
