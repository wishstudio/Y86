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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTreeView>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    VM::init();

    QPushButton *openFileButton = new QPushButton("Open assembly...", this);
    connect(openFileButton, SIGNAL(clicked()), SLOT(openFile()));
    QPushButton *startButton = new QPushButton("Start", this);
    connect(startButton, SIGNAL(clicked()), SLOT(start()));
    QPushButton *stepButton = new QPushButton("Step", this);
    connect(stepButton, SIGNAL(clicked()), SLOT(step()));
    QHBoxLayout *toolsLayout = new QHBoxLayout();
    toolsLayout->addWidget(openFileButton);
    toolsLayout->addWidget(startButton);
    toolsLayout->addWidget(stepButton);

    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(0);
    layout->addLayout(toolsLayout, 0, 0);

    for (int i = 0; i < WORKERS_COUNT; i++)
    {
        stageViewer[i] = new StageViewer(i, this);
        connect(VM::self(), SIGNAL(updateDisplay()), stageViewer[i], SLOT(updateDisplay()));
        layout->addWidget(stageViewer[i], i + 1, 0);
    }
    QTreeView *memoryViewer = new QTreeView(this);
    memoryViewer->setModel(VM::codeListModel());
    layout->addWidget(memoryViewer, 0, 1, 4, 1);

    registerViewer = new RegisterViewer(this);
    connect(VM::self(), SIGNAL(updateDisplay()), registerViewer, SLOT(updateDisplay()));
    layout->addWidget(registerViewer, 4, 1, 2, 1);

    setLayout(layout);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open assembly...", QString(), "All files (*.*)");
    if (QFile::exists(fileName))
        VM::loadObject(fileName);
}

void MainWindow::start()
{
}

void MainWindow::step()
{
    VM::step();
}
