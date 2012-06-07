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

#ifndef CODELISTMODEL_H
#define CODELISTMODEL_H

#include <QAbstractListModel>
#include <QVector>

class CodeListModel: public QAbstractListModel
{
    Q_OBJECT

public:
    CodeListModel(QObject *parent = 0);

    void setMemory(QVector<QString> code, QVector<int> memoryRef, int startStack);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const { return 4; }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;

public slots:
    void updateDisplay();

private:
    QVector<int> m_memoryRef;
    int m_startStack;
    QVector<QString> m_code;
};

#endif
