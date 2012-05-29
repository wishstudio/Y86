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

#include "CodeListModel.h"
#include "VM.h"

CodeListModel::CodeListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void CodeListModel::setMemoryRef(QVector<int> memoryRef, int startStack)
{
    m_memoryRef = memoryRef;
    m_startStack = startStack;
}

void CodeListModel::setCode(QVector<QString> code)
{
    m_code = code;
}

int CodeListModel::rowCount(const QModelIndex &parent) const
{
    return m_memoryRef.size();
}

QVariant CodeListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        int id = index.row();
        switch (index.column())
        {
        case 0:
        {
            if (id + 1 < m_memoryRef.size() && m_memoryRef.at(id) == m_memoryRef.at(id + 1))
                return QVariant();
            QString ret = QString::number(m_memoryRef.at(id), 16).toUpper();
            while (ret.size() < 8)
                ret = ret.prepend("0");
            return ret;
        }

        case 1:
        {
            int high;
            if (id == m_memoryRef.size() - 1)
                high = m_startStack;
            else
                high = m_memoryRef[id + 1];
            QString ret;
            for (int i = m_memoryRef[id]; i < high; i++)
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

        case 2:
        {
            if (id + 1 < m_memoryRef.size() && m_memoryRef.at(id) == m_memoryRef.at(id + 1))
                return QVariant();
            for (int i = 0; i < WORKERS_COUNT; i++)
                if (m_memoryRef.at(id) == VM::workerAddr(i))
                    return stageNames[i];
            return QVariant();
        }

        case 3:
            return m_code.at(id);
        }
    }
    return QVariant();
}

void CodeListModel::updateDisplay()
{
    emit dataChanged(createIndex(0, 0), createIndex(m_memoryRef.size(), 4));
}
