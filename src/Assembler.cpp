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

#include <QVector>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "Assembler.h"

#define PAIR(ra, rb) (((ra) << 4) | rb)

QString opNames[] = {"nop", "halt", "rrmovl", "irmovl", "rmmovl", "mrmovl", "opl", "jmp", "call", "ret", "pushl", "popl"};
QString funOplNames[] = {"addl", "subl", "mull", "divl", "modl", "andl", "orl", "xorl"};
QString funJmpNames[] = {"jmp", "jle", "jl", "je", "jne", "jge", "jg"};
QString registerNames[] = {"eax", "ecx", "edx", "ebx", "esi", "edi", "esp", "ebp", "none"};

static enum tokenType {tkEOF, tkComma, tkColon, tkDot, tkRegister, tkMemory, tkNumber, tkLabel, tkLP, tkRP} tt;
static QMap<QString, int> symbolTable;
static QVector<QPair<QString, int> > patchList;
static QFile inFile;
static QTextStream inTextStream;
static Memory *memory;
static int startEIP;
static int stackSize;

static int line_cnt, tr, tn;
static QString token;
static char ch;

static void error(QString error_message)
{
    qDebug("Compile error at line %d: %s\n", line_cnt, qPrintable(error_message));
    exit(1);
}

static void getChar()
{
    if (inTextStream.atEnd())
        ch = -1;
    else
    {
        inTextStream >> ch;
        ch = tolower(ch);
    }
}

static void getToken()
{
    while (isspace(ch))
    {
        if (ch == '\n')
            line_cnt++;
        getChar();
    }
    if (ch == -1)
        tt = tkEOF;
    else if (ch == ';') /* comments */
    {
        getChar();
        while (ch != -1 && ch != '\n')
            getChar();
        getToken();
    }
    else if (ch == '%' || isalpha(ch) || ch == '_')
    {
        if (ch == '%')
        {
            tt = tkRegister;
            token = "";
        }
        else
        {
            tt = tkLabel;
            token = ch;
        }
        getChar();
        while (isalpha(ch) || isdigit(ch) || ch == '_')
        {
            token = token + ch;
            getChar();
        }
        if (tt == tkRegister)
        {
            tr = 8;
            for (int i = 0; i < 8; i++)
                if (token == registerNames[i])
                {
                    tr = i;
                    break;
                }
            if (tr == 8)
                error("Unknown register name.");
        }
    }
    else if (ch == '$' || isdigit(ch))
    {
        if (ch == '$')
        {
            tt = tkNumber;
            getChar();
            if (!isdigit(ch))
                error("Number expected after $.");
        }
        else
            tt = tkMemory;
        int base = 10;
        if (ch == '0')
        {
            getChar();
            if (ch == 'x' || ch == 'X')
            {
                getChar();
                if (!isdigit(ch))
                    error("Number expected after 0x.");
                base = 16;
            }
            else if (isdigit(ch))
                base = 8;
            else
                error("Number expected after 0.");
        }
        token = "";
        while ((ch >= '0' && ch <= '7') || (base > 8 && ch >= '0' && ch <= '9') || (base > 10 && ch >= 'a' && ch <= 'f'))
        {
            token = token + ch;
            getChar();
        }
        tn = token.toInt(NULL, base);
    }
    else
        switch (ch)
        {
        case '.': tt = tkDot; getChar(); break;
        case ',': tt = tkComma; getChar(); break;
        case ':': tt = tkColon; getChar(); break;
        case '(': tt = tkLP; getChar(); break;
        case ')': tt = tkRP; getChar(); break;
        default: error(QString("Unrecognized character '%1'.").arg(ch));
        }
}

static void expectRegister()
{
    if (tt != tkRegister)
        error("Register expected.");
    getToken();
}

static void expectLabel()
{
    if (tt != tkLabel)
        error("Label expected.");
    getToken();
}

static void expectNumber()
{
    if (tt != tkNumber)
        error("Number expected.");
    getToken();
}

static void expectComma()
{
    if (tt != tkComma)
        error("Comma expected.");
    getToken();
}

static void expectRP()
{
    if (tt != tkRP)
        error("')' expected.");
    getToken();
}

static void putAddr(QString label)
{
    if (symbolTable.count(label))
        memory->put(symbolTable.value(label));
    else
    {
        patchList.push_back(qMakePair(label, memory->addr()));
        memory->put(0);
    }
}

static void compile()
{
    for (;;)
    {
        if (tt == tkEOF)
            break;
        else if (tt == tkDot)
        {
            getToken();
            QString label = token;
            expectLabel();
            if (label == "text")
            {
                if (startEIP == -1)
                    startEIP = memory->addr();
                memory->setAttr(false);
            }
            else if (label == "stacksize")
            {
                stackSize = tn;
                expectNumber();
            }
            else if (label == "rodata")
                memory->setAttr(false);
            else if (label == "data")
                memory->setAttr(true);
            else if (label == "origin")
            {
                int origin = tn;
                expectNumber();
                memory->setOrigin(origin);
            }
            else if (label == "reserve")
            {
                int reserveCount = tn;
                expectNumber();
                memory->setOrigin(memory->addr() + reserveCount);
            }
        }
        else if (tt == tkLabel)
        {
            QString label = token;
            getToken();
            if (tt == tkColon)
            {
                symbolTable.insert(label, memory->addr());
                getToken();
            }
            else if (label == "nop")
                memory->putChar(PAIR(OP_NOP, 0));
            else if (label == "halt")
                memory->putChar(PAIR(OP_HALT, 0));
            else if (label == "rrmovl")
            {
                int rA = tr;
                expectRegister();
                expectComma();
                int rB = tr;
                expectRegister();
                memory->putChar(PAIR(OP_RRMOVL, 0));
                memory->putChar(PAIR(rA, rB));
            }
            else if (label == "irmovl")
            {
                int num = tn;
                expectNumber();
                expectComma();
                int rB = tr;
                expectRegister();
                memory->putChar(PAIR(OP_IRMOVL, 0));
                memory->putChar(PAIR(REG_NONE, rB));
                memory->put(num);
            }
            else if (label == "rmmovl")
            {
                int rA = tr;
                expectRegister();
                expectComma();
                QString label = token;
                expectLabel();
                int rB;
                if (tt == tkLP)
                {
                    getToken();
                    rB = tr;
                    expectRegister();
                    expectRP();
                }
                else
                    rB = 8;
                memory->putChar(PAIR(OP_RMMOVL, 0));
                memory->putChar(PAIR(rA, rB));
                putAddr(label);
            }
            else if (label == "mrmovl")
            {
                QString label = token;
                expectLabel();
                int rB;
                if (tt == tkLP)
                {
                    getToken();
                    rB = tr;
                    expectRegister();
                    expectRP();
                }
                else
                    rB = 8;
                expectComma();
                int rA = tr;
                expectRegister();
                memory->putChar(PAIR(OP_MRMOVL, 0));
                memory->putChar(PAIR(rA, rB));
                putAddr(label);
            }
            else if (label == "call")
            {
                QString label = token;
                memory->putChar(PAIR(OP_CALL, 0));
                putAddr(label);
            }
            else if (label == "ret")
            {
                memory->putChar(PAIR(OP_RET, 0));
            }
            else if (label == "pushl")
            {
                int rA = tr;
                expectRegister();
                memory->putChar(PAIR(OP_PUSHL, 0));
                memory->putChar(PAIR(rA, REG_NONE));
            }
            else if (label == "popl")
            {
                int rA = tr;
                expectRegister();
                memory->putChar(PAIR(OP_POPL, 0));
                memory->putChar(PAIR(rA, REG_NONE));
            }
            else
            {
                int fun = -1;
                for (int i = 0; i < FUN_OPL_CNT; i++)
                    if (token == funOplNames[i])
                    {
                        fun = i;
                        break;
                    }
                if (fun != -1)
                {
                    int rA = tr;
                    expectRegister();
                    expectComma();
                    int rB = tr;
                    expectRegister();
                    memory->putChar(PAIR(OP_OPL, fun));
                    memory->putChar(PAIR(rA, rB));
                }
                else
                {
                    for (int i = 0; i < FUN_JMP_CNT; i++)
                        if (token == funJmpNames[i])
                        {
                            fun = i;
                            break;
                        }
                    if (fun != -1)
                    {
                        QString label = token;
                        expectLabel();
                        memory->putChar(PAIR(OP_JMP, fun));
                        putAddr(label);
                    }
                    else
                        error("Unrecognized token \"" + token + "\".");
                }
            }
        }
    }
}

void Assembler::compileFile(const QString &fileName, Memory *memory)
{
    ::memory = memory;
    ::memory->clear();
    line_cnt = 1;
    symbolTable.clear();
    patchList.clear();
    ::startEIP = -1;
    /* default stack size: 16KBytes */
    stackSize = 16384;
    inFile.setFileName(fileName);
    inFile.open(QIODevice::ReadOnly);
    inTextStream.setDevice(&inFile);
    getChar();
    getToken();
    compile();
    inFile.close();
    /* allocate stack space */
    memory->setOrigin(memory->addr() + stackSize);
}

int Assembler::startEIP()
{
    return ::startEIP;
}

int Assembler::startESP()
{
    return memory->addr();
}
