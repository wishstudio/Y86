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

#ifndef WIRE_H
#define WIRE_H

// A thread-safe class for holding pipeline registers

#include <QString>

#define HASH_SIZE 4096

class Wire
{
public:
    Wire();
    virtual ~Wire();

    void clear();
    void clearState();
    bool state(const QString &_key);
    void copyFrom(Wire *src);
    void reserve(const QString &_key);
    int readWire(const QString &_key) const;
    void writeWire(const QString &_key, int _value);

private:
    QString key[HASH_SIZE];
    int value[HASH_SIZE];
    bool used[HASH_SIZE];
};

#endif
