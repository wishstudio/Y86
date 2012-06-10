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

#include <QFont>

#include "Assembler.h"
#include "Register.h"
#include "StackListModel.h"
#include "VM.h"

StackListModel::StackListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    startStack = 0;
}

void StackListModel::setStartStack(int startStack)
{
    emit layoutAboutToBeChanged();
    this->startStack = startStack;
    emit layoutChanged();
}

int StackListModel::rowCount(const QModelIndex &parent) const
{
    return (VM::memory()->addr() - startStack) / 4;
}

QVariant StackListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case 0:
            return "Address";

        case 1:
            return "Data";
        }
    }
    return QVariant();
}

QVariant StackListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        int row = index.row();
        int addr = VM::memory()->addr() - row * 4 - 4;
        if (index.column() == 0)
        {
            QString ret = QString::number(addr, 16).toUpper();
            while (ret.size() < 8)
                ret = ret.prepend("0");
            return ret;
        }
        else if (index.column() == 1)
        {
            QString ret;
            for (int i = addr; i < addr + 4; i++)
            {
                QString p = QString::number((unsigned char) VM::memory()->readChar(i), 16).toUpper();
                if (p.size() < 2)
                    p = p.prepend("0");
                if (!ret.isEmpty())
                    ret = ret.append(" ");
                ret = ret.append(p);
            }
            return ret;
        }
    }
    else if (role == Qt::FontRole)
    {
        QFont font("Monospace");
        font.setStyleHint(QFont::TypeWriter);
        return font;
    }
    return QVariant();
}

void StackListModel::updateDisplay()
{
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}
