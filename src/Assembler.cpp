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

QString opNames[] = {"nop", "halt", "rrmovl", "irmovl", "rmmovl", "mrmovl", "opl", "jmp", "call", "ret", "pushl", "popl", "lidt", "int", "excep"};
QString funOplNames[] = {"addl", "subl", "cmpl", "mull", "divl", "modl", "andl", "orl", "xorl"};
QString funJmpNames[] = {"jmp", "jle", "jl", "je", "jne", "jge", "jg"};
QString exceptionNames[] = {"divbz", "mem", "op", "reg", "dbl"};
QString registerNames[] = {"eax", "ecx", "edx", "ebx", "esi", "edi", "esp", "ebp", "none", "idtr", "eflags"};
QString eflagsNames[] = {"cf", "zf", "sf", "of"};

static enum tokenType {tkEOF, tkComma, tkColon, tkDot, tkRegister, tkMemory, tkNumber, tkLabel, tkLP, tkRP} tt;
static QMap<QString, int> symbolTable;
static QMap<int, QString> symbolLookupTable;
static QVector<QPair<QString, int> > patchList;
static QFile inFile;
static QTextStream inTextStream;
static Memory *memory;
static int startEIP;
static int stackSize;
static int startStack;
static QVector<int> memoryRef; // line number -> memory address
static QVector<QString> code;
static QString lastLine;

static int line_cnt, tr, tn;
static QString token;
static char ch;

static void error(QString error_message)
{
    qDebug("Compile error at line %d: %s\n", line_cnt + 1, qPrintable(error_message));
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
        if (ch == '\n')
        {
            code.push_back(lastLine);
            lastLine = "";
        }
        else
            lastLine = lastLine.append(ch);
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
                base = 16;
            }
            else if (isdigit(ch))
                base = 8;
            else
            {
                tn = 0;
                return;
            }
        }
        token = "";
        while ((ch >= '0' && ch <= '7') || (base > 8 && ch >= '0' && ch <= '9') || (base > 10 && ch >= 'a' && ch <= 'f'))
        {
            token = token + ch;
            getChar();
        }
        if (token.isEmpty())
            error("Number expected after 0 or 0x tag.");
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

static void expectRawNumber()
{
    if (tt != tkNumber && tt != tkMemory)
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

static void parseMemoryRef(int &reg, QString &label, int &imm)
{
    if (tt == tkMemory)
    {
        imm = tn;
        label = QString();
        getToken();
    }
    else if (tt == tkLabel)
    {
        label = token;
        getToken();
    }
    else if (tt == tkLP)
    {
        imm = 0;
        label = QString();
    }
    else
        error("Memory reference expected.");
    if (tt == tkLP)
    {
        getToken();
        reg = tr;
        expectRegister();
        expectRP();
    }
    else
        reg = REG_NONE;
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
        int current_addr = memory->addr(), current_line = line_cnt;
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
                memory->setAttr(false, true);
                continue;
            }
            else if (label == "rodata")
            {
                memory->setAttr(false, false);
                continue;
            }
            else if (label == "data")
            {
                memory->setAttr(true, false);
                continue;
            }
            else if (label == "stack")
            {
                ::stackSize = tn;
                expectRawNumber();
                continue;
            }
            else if (label == "reserve")
            {
                int reserveCount = tn;
                expectRawNumber();
                memory->setOrigin(memory->addr() + reserveCount);
            }
            else if (label == "db" || label == "dw" || label == "dd")
            {
                QString dtype = label;
                for (;;)
                {
                    if (tt == tkNumber)
                    {
                        if (dtype == "db")
                            memory->putChar(tn);
                        else if (dtype == "dw")
                            memory->putShort(tn);
                        else
                            memory->put(tn);
                    }
                    else if (tt == tkMemory)
                        memory->put(tn);
                    else if (tt == tkLabel)
                        putAddr(token);
                    else
                        error("Expect immediate value.");
                    getToken();
                    if (tt == tkComma)
                        getToken();
                    else
                        break;
                }
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
                continue;
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
                int num;
                QString label;
                if (tt == tkNumber || tt == tkMemory)
                    num = tn;
                else if (tt == tkLabel)
                    label = token;
                else
                    error("Expect immediate value.");
                getToken();
                expectComma();
                int rB = tr;
                expectRegister();
                memory->putChar(PAIR(OP_IRMOVL, 0));
                memory->putChar(PAIR(REG_NONE, rB));
                if (!label.isEmpty())
                    putAddr(label);
                else
                    memory->put(num);
            }
            else if (label == "rmmovl")
            {
                int rA = tr;
                expectRegister();
                expectComma();
                QString label;
                int imm, rB;
                parseMemoryRef(rB, label, imm);
                memory->putChar(PAIR(OP_RMMOVL, 0));
                memory->putChar(PAIR(rA, rB));
                if (!label.isEmpty())
                    putAddr(label);
                else
                    memory->put(imm);
            }
            else if (label == "mrmovl")
            {
                QString label;
                int imm, rB;
                parseMemoryRef(rB, label, imm);
                expectComma();
                int rA = tr;
                expectRegister();
                memory->putChar(PAIR(OP_MRMOVL, 0));
                memory->putChar(PAIR(rA, rB));
                if (!label.isEmpty())
                    putAddr(label);
                else
                    memory->put(imm);
            }
            else if (label == "call")
            {
                QString label = token;
                getToken();
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
            else if (label == "lidt")
            {
                memory->putChar(PAIR(OP_LIDT, 0));
                if (tt == tkMemory)
                    memory->put(tn);
                else if (tt == tkLabel)
                    putAddr(token);
                else
                    error("Expected immediate memory address.");
                getToken();
            }
            else if (label == "int")
            {
                int t = tn;
                expectNumber();
                memory->putChar(PAIR(OP_INT, t));
            }
            else
            {
                int fun = -1;
                for (int i = 0; i < FUN_OPL_CNT; i++)
                    if (label == funOplNames[i])
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
                        if (label == funJmpNames[i])
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
        for (int i = memoryRef.size(); i < current_line; i++)
            memoryRef.push_back(current_addr);
        memoryRef.push_back(current_addr);
    }
    for (int i = 0; i < patchList.size(); i++)
        memory->patch(patchList[i].second, symbolTable.value(patchList[i].first));
}

void Assembler::compileFile(const QString &fileName, Memory *memory)
{
    ::memory = memory;
    ::memory->clear();
    line_cnt = 0;
    symbolTable.clear();
    patchList.clear();
    ::startEIP = -1;
    ::memoryRef.clear();
    ::memoryRef.push_back(0);
    ::code.clear();
    ::lastLine.clear();
    inFile.setFileName(fileName);
    inFile.open(QIODevice::ReadOnly);
    inTextStream.setDevice(&inFile);
    /* default stack size: 0Bytes */
    ::stackSize = 0;
    getChar();
    getToken();
    compile();
    inFile.close();
    ::code.push_back(lastLine);
    /* allocate stack space */
    ::startStack = memory->addr();
    memory->setAttr(true, false);
    memory->setOrigin(memory->addr() + stackSize);
    /* generate symbolLookupTable */
    ::symbolLookupTable.clear();
    for (QMap<QString, int>::ConstIterator i = symbolTable.constBegin(); i != symbolTable.constEnd(); i++)
        ::symbolLookupTable.insert(i.value(), i.key());
}

int Assembler::startEIP()
{
    return ::startEIP;
}

int Assembler::startESP()
{
    return memory->addr();
}

int Assembler::startStack()
{
    return ::startStack;
}

QVector<int> Assembler::memoryRef()
{
    return ::memoryRef;
}

QVector<QString> Assembler::code()
{
    return ::code;
}

QMap<int, QString> Assembler::symbolLookupTable()
{
    return ::symbolLookupTable;
}
