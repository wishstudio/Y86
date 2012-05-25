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

#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    VM::init();

    QPushButton *openFileButton = new QPushButton("Open assembly...");
    connect(openFileButton, SIGNAL(clicked()), SLOT(openFile()));
    layout->addWidget(openFileButton);

    for (int i = 0; i < WORKERS_COUNT; i++)
    {
        stageViewer[i] = new StageViewer(i, this);
        layout->addWidget(stageViewer[i]);
    }

    setLayout(layout);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open assembly...", QString(), "All files (*.*)");
    if (QFile::exists(fileName))
        VM::loadObject(fileName);
}
