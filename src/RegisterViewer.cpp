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

#include <QVBoxLayout>

#include "Register.h"
#include "RegisterViewer.h"
#include "VM.h"

RegisterViewer::RegisterViewer(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    for (int i = 0; i < REG_NONE; i++)
    {
        registerLabels[i] = new QLabel(QString("%%1: ").arg(registerNames[i]), this);
        layout->addWidget(registerLabels[i]);
    }

    setLayout(layout);
}

void RegisterViewer::updateDisplay()
{
    for (int i = 0; i < REG_NONE; i++)
        registerLabels[i]->setText(QString("%%1: %2").arg(registerNames[i]).arg(VM::reg()->readRegister(i)));
}
