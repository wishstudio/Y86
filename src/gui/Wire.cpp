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

#include <QHash>

#include "Wire.h"

Wire::Wire()
{
}

Wire::~Wire()
{
}

void Wire::clearState()
{
    memset(used, 0, sizeof used);
}

bool Wire::state(const QString &_key)
{
    uint h = qHash(_key) % HASH_SIZE;
    while (key[h] != _key)
        h = (h + 1) % HASH_SIZE;
    return used[h];
}

void Wire::reserve(const QString &_key)
{
    uint h = qHash(_key) % HASH_SIZE;
    while (!key[h].isEmpty() && key[h] != _key)
        h = (h + 1) % HASH_SIZE;
    if (key[h].isEmpty())
        key[h] = _key;
}

int Wire::readWire(const QString &_key) const
{
    uint h = qHash(_key) % HASH_SIZE;
    while (key[h] != _key)
        h = (h + 1) % HASH_SIZE;
    return value[h];
}

void Wire::writeWire(const QString &_key, int _value)
{
    uint h = qHash(_key) % HASH_SIZE;
    while (key[h] != _key)
        h = (h + 1) % HASH_SIZE;
    value[h] = _value;
}
