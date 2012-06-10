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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define FREQUENCY_COUNT     4

#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>

#include "RegisterViewer.h"
#include "StageViewer.h"
#include "VM.h"

class MainWindow: public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void openFile();
    void start();
    void step();
    void reset();
    void changeFrequency();
    void updateDisplay();
    void stopped();

private:
    QPushButton *startButton, *stepButton, *resetButton;
    QRadioButton *frequencyButton[FREQUENCY_COUNT];
    RegisterViewer *registerViewer;
    StageViewer *stageViewer[WORKERS_COUNT];
    QLabel *fileNameLabel;
    QLabel *cycleCountLabel, *instructionCountLabel;
    QString fileName;
};

#endif
