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

#include <QButtonGroup>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTreeView>

#include "MainWindow.h"

int frequencies[FREQUENCY_COUNT] = {1, 2, 5, 0};

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    VM::init();
    connect(VM::self(), SIGNAL(updateDisplay()), SLOT(updateDisplay()));

    QGroupBox *controlGroup = new QGroupBox(this);
    controlGroup->setTitle("Controls");
    QGridLayout *controlLayout = new QGridLayout();

    QPushButton *openFileButton = new QPushButton("Open...", this);
    connect(openFileButton, SIGNAL(clicked()), SLOT(openFile()));
    fileNameLabel = new QLabel(this);
    QButtonGroup *frequencyGroup = new QButtonGroup(this);
    QHBoxLayout *frequencyLayout = new QHBoxLayout();
    frequencyLayout->setSpacing(0);
    for (int i = 0; i < FREQUENCY_COUNT; i++)
    {
        frequencyButton[i] = new QRadioButton(this);
        if (frequencies[i])
            frequencyButton[i]->setText(QString("%1Hz").arg(frequencies[i]));
        else
            frequencyButton[i]->setText("No Limit");
        connect(frequencyButton[i], SIGNAL(clicked()), SLOT(changeFrequency()));
        frequencyGroup->addButton(frequencyButton[i]);
        frequencyLayout->addWidget(frequencyButton[i]);
    }
    frequencyButton[0]->setChecked(true);
    VM::setFrequency(1);
    startButton = new QPushButton("Start", this);
    connect(startButton, SIGNAL(clicked()), SLOT(start()));
    stepButton = new QPushButton("Step", this);
    connect(stepButton, SIGNAL(clicked()), SLOT(step()));
    resetButton = new QPushButton("Reset", this);
    connect(resetButton, SIGNAL(clicked()), SLOT(reset()));

    QHBoxLayout *counterLayout = new QHBoxLayout();
    cycleCountLabel = new QLabel(this);
    instructionCountLabel = new QLabel(this);
    counterLayout->addWidget(cycleCountLabel);
    counterLayout->addWidget(instructionCountLabel);

    controlLayout->addWidget(openFileButton, 0, 0);
    controlLayout->addWidget(fileNameLabel, 0, 1, 1, 2);
    controlLayout->addLayout(frequencyLayout, 1, 0, 1, 3);
    controlLayout->addWidget(startButton, 2, 0);
    controlLayout->addWidget(stepButton, 2, 1);
    controlLayout->addWidget(resetButton, 2, 2);
    controlLayout->addLayout(counterLayout, 3, 0, 1, 3);
    controlGroup->setLayout(controlLayout);

    QGridLayout *layout = new QGridLayout(this);
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(5);
    layout->addWidget(controlGroup, 0, 2, Qt::AlignLeft);

    QTreeView *memoryViewer = new QTreeView(this);
    memoryViewer->setModel(VM::codeListModel());
    memoryViewer->setColumnWidth(0, 100);
    memoryViewer->setColumnWidth(1, 200);
    memoryViewer->setColumnWidth(2, 40);
    layout->addWidget(memoryViewer, 0, 0, 4, 1);

    QTreeView *stackViewer = new QTreeView(this);
    stackViewer->setModel(VM::stackListModel());
    stackViewer->setFixedWidth(220);
    layout->addWidget(stackViewer, 0, 1);

    registerViewer = new RegisterViewer(this);
    connect(VM::self(), SIGNAL(updateDisplay()), registerViewer, SLOT(updateDisplay()));
    layout->addWidget(registerViewer, 1, 1, 1, 2);

    QGroupBox *stageGroup = new QGroupBox(this);
    stageGroup->setTitle("Stages");
    QVBoxLayout *stageLayout = new QVBoxLayout();
    for (int i = 0; i < WORKERS_COUNT; i++)
    {
        stageViewer[i] = new StageViewer(i, this);
        connect(VM::self(), SIGNAL(updateDisplay()), stageViewer[i], SLOT(updateDisplay()));
        stageLayout->addWidget(stageViewer[i]);
    }
    stageGroup->setLayout(stageLayout);
    layout->addWidget(stageGroup, 2, 1, 1, 2);

    updateDisplay();

    setLayout(layout);
    setMinimumSize(1250, 700);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open assembly...", QString(), "All files (*.*)");
    if (QFile::exists(fileName))
    {
        fileNameLabel->setText(QFileInfo(fileName).baseName());
        this->fileName = fileName;
        VM::loadObject(fileName);
    }
}

void MainWindow::start()
{
    VM::startVM();
}

void MainWindow::step()
{
    if (!VM::self()->isRunning())
        VM::step();
}

void MainWindow::reset()
{
    VM::loadObject(fileName);
}

void MainWindow::changeFrequency()
{
    for (int i = 0; i < FREQUENCY_COUNT; i++)
        if (sender() == frequencyButton[i])
        {
            VM::setFrequency(frequencies[i]);
            return;
        }
}

void MainWindow::updateDisplay()
{
    startButton->setText("Pause");
    startButton->setDisabled(VM::isHalted());
    stepButton->setDisabled(VM::isHalted());
    resetButton->setDisabled(fileName.isEmpty());
    cycleCountLabel->setText(QString("Cycle: %1").arg(VM::cycleCount()));
    instructionCountLabel->setText(QString("Inst: %1").arg(VM::instructionCount()));
}

void MainWindow::stopped()
{
}
