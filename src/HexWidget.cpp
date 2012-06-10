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

#include <QPainter>
#include <QFontMetrics>

#include "HexWidget.h"

HexWidget::HexWidget(QWidget *parent)
    : QWidget(parent)
{
    number = 0;
    font = QFont("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    changed = false;
    showChanges = false;
}

void HexWidget::paintEvent(QPaintEvent *)
{
    QString text = QString::number(number, 16).toUpper();
    while (text.size() < bits)
        text = text.prepend("0");

    QPainter painter(this);
    painter.setFont(font);
    painter.drawRect(0, 0, w - 1, h - 1);
    painter.setPen(showChanges && changed? Qt::red: Qt::black);
    painter.drawText(3, h - 2, text);
}

void HexWidget::setBits(int number)
{
    QString text;
    for (int i = 0; i < number; i++)
        text = text.append("0");
    QFontMetrics m = QFontMetrics(font);
    w = m.width(text) + 9;
    h = m.height();
    setFixedSize(w, h);
    bits = number;
}

void HexWidget::setNumber(unsigned int number)
{
    changed = this->number != number;
    this->number = number;
    emit update();
}

void HexWidget::setShowChanges(bool showChanges)
{
    this->showChanges = showChanges;
}
