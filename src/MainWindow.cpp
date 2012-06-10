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
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
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
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(5);
    layout->addLayout(toolsLayout, 0, 0);

    QTreeView *memoryViewer = new QTreeView(this);
    memoryViewer->setModel(VM::codeListModel());
    memoryViewer->setColumnWidth(0, 100);
    memoryViewer->setColumnWidth(1, 200);
    memoryViewer->setColumnWidth(2, 40);
    layout->addWidget(memoryViewer, 1, 0, 4, 1);

    QTreeView *stackViewer = new QTreeView(this);
    stackViewer->setModel(VM::stackListModel());
    stackViewer->setFixedWidth(220);
    layout->addWidget(stackViewer, 1, 1);

    registerViewer = new RegisterViewer(this);
    connect(VM::self(), SIGNAL(updateDisplay()), registerViewer, SLOT(updateDisplay()));
    layout->addWidget(registerViewer, 2, 1, Qt::AlignLeft);

    QGroupBox *stageGroup = new QGroupBox(this);
    stageGroup->setTitle("Stages");
    QGridLayout *stageLayout = new QGridLayout();
    stageLayout->setVerticalSpacing(0);
    for (int i = 0; i < WORKERS_COUNT; i++)
    {
        stageViewer[i] = new StageViewer(i, this);
        connect(VM::self(), SIGNAL(updateDisplay()), stageViewer[i], SLOT(updateDisplay()));
        stageLayout->addWidget(stageViewer[i], i, 0, Qt::AlignLeft);
    }
    stageGroup->setLayout(stageLayout);
    layout->addWidget(stageGroup, 3, 1, Qt::AlignLeft);

    setLayout(layout);
    setMinimumSize(1250, 700);
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
