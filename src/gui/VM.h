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

#ifndef VM_H
#define VM_H

#include "VMWorker.h"
#include "Memory.h"
#include "Wire.h"

#define STAGE_F    0
#define STAGE_D    1
#define STAGE_E    2
#define STAGE_M    3
#define STAGE_W    4

class VM
{
public:
    VM();
    virtual ~VM();

    void loadObject(const QString &fileName);

private:
    Memory *memory;
    Wire *wire;
    VMWorker *stageWorkers[5];
};

#endif
