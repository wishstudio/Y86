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

#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cctype>

#include "Segment.h"

/* Object file format (numbers are little-endian)

  char header[5]: "YYOBJ"
  int start_eip, start_esp
  int memory_size;
  int segment_count;

  for every segment:

  int type; // 0 - read only, 1 - read write
  int origin; // origin address
  int length; // in bytes, the segment will be placed at [origin, origin + segment_length - 1] in memory
  char content[]; // content

  supported segments:
  */

#define REGISTER_PAIR(ra, rb) (((ra) << 4) | rb)

enum tokenType {tkEOF, tkComma, tkDot, tkRegister, tkNumber, tkLabel, tkLP, tkRP} tt;
std::string registerName[] = {"eax", "ecx", "edx", "ebx", "esi", "edi", "esp", "ebp"};
std::map<std::string, std::pair<int, int> > symbolTable;
std::vector<std::pair<std::pair<std::string, int>, std::pair<int, int> > > patchList;
// label, line number, segment id, segment offset
std::vector<Segment> segments;

int line_cnt, tr, tn;
std::string token;
char ch;

void error(std::string error_message)
{
    fprintf(stderr, "Compile error at line %d: %s\n", line_cnt, error_message.c_str());
    exit(1);
}

void getToken()
{
    while (isspace(ch))
    {
        if (ch == '\n')
            line_cnt++;
        ch = getchar();
    }
    if (ch == '%' || isalpha(ch) || ch == '_')
    {
        if (ch == '%')
        {
            tt = tkRegister;
            token = "";
        }
        else
        {
            tt = tkLabel;
            token = (char) tolower(ch);
        }
        ch = getchar();
        while (isalpha(ch) || isdigit(ch) || ch == '_')
        {
            token = token + (char) tolower(ch);
            ch = getchar();
        }
        if (tt == tkRegister)
        {
            tr = 8;
            for (int i = 0; i < 8; i++)
                if (token == registerName[i])
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
            ch = getchar();
            if (!isdigit(ch))
                error("Number expected after $.");
        }
        int base = 10;
        if (ch == '0')
        {
            ch = getchar();
            if (ch == 'x')
            {
                ch = getchar();
                if (!isdigit(ch))
                    error("Number expected after 0x.");
                base = 16;
            }
            else if (isdigit(ch))
            {
                base = 8;
            }
            else
                error("Number expected after 0.");
        }
        token = ch;
        ch = getchar();
        while (isdigit(ch) || (tolower(ch) >= 'a' && tolower(ch) <= 'f'))
        {
            token = token + ch;
            ch = getchar();
        }
        // TODO
    }
    switch (ch)
    {
    case '.': tt = tkDot; ch = getchar(); break;
    case ',': tt = tkComma; ch = getchar(); break;
    case '(': tt = tkLP; ch = getchar(); break;
    case ')': tt = tkRP; ch = getchar(); break;
    }
}

void expectRegister()
{
    if (tt != tkRegister)
        error("Register expected.");
    getToken();
}

void expectLabel()
{
    if (tt != tkLabel)
        error("Label expected.");
    getToken();
}

void expectNumber()
{
    if (tt != tkNumber)
        error("Number expected.");
    getToken();
}

void expectComma()
{
    if (tt != tkComma)
        error("Comma expected.");
    getToken();
}

void expectRP()
{
    if (tt != tkRP)
        error("')' expected.");
    getToken();
}

void putAddr(std::string label)
{
    if (symbolTable.count(label))
    {
        std::pair<int, int> addr = symbolTable.at(label);
        segments.back().put(segments.at(addr.first).origin() + addr.second);
    }
    else
    {
        patchList.push_back(std::make_pair(std::make_pair(label, line_cnt), std::make_pair(segments.size() - 1, segments.back().addr())));
        segments.back().put(0);
    }
}

void compile()
{
    while (tt == tkDot)
    {
        getToken();
        if (token == "text")
        {
        }
    }

    segments.push_back(Segment(0));

    for (;;)
    {
        if (tt == tkLabel)
        {
            if (token == "nop")
            {
                getToken();
                segments.back().putChar(0x00);
            }
            else if (token == "halt")
            {
                getToken();
                segments.back().putChar(0x10);
            }
            else if (token == "rrmovl")
            {
                getToken();
                int rA = tr;
                expectRegister();
                expectComma();
                int rB = tr;
                expectRegister();
                segments.back().putChar(0x20);
                segments.back().putChar(REGISTER_PAIR(rA, rB));
            }
            else if (token == "irmovl")
            {
                getToken();
                int num = tn;
                expectNumber();
                expectComma();
                int rB = tr;
                expectRegister();
                segments.back().putChar(0x30);
                segments.back().putChar(REGISTER_PAIR(8, rB));
                segments.back().put(num);
            }
            else if (token == "rmmovl")
            {
                getToken();
                int rA = tr;
                expectRegister();
                expectComma();
                std::string label = token;
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
                segments.back().putChar(0x40);
                segments.back().putChar(REGISTER_PAIR(rA, rB));
                putAddr(label);
            }
            else if (token == "mrmovl")
            {
                getToken();
                std::string label = token;
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
                segments.back().putChar(0x50);
                segments.back().putChar(REGISTER_PAIR(rA, rB));
                putAddr(label);
            }
            else if (token == "call")
            {
                getToken();
                std::string label = token;
                segments.back().putChar(0x80);
                putAddr(label);
            }
            else if (token == "ret")
            {
                getToken();
                segments.back().putChar(0x90);
            }
            else if (token == "pushl")
            {
                getToken();
                int rA = tr;
                expectRegister();
                segments.back().putChar(0xA0);
                segments.back().putChar(REGISTER_PAIR(rA, 8));
            }
            else if (token == "popl")
            {
                getToken();
                int rA = tr;
                expectRegister();
                segments.back().putChar(0xB0);
                segments.back().putChar(REGISTER_PAIR(rA, 8));
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
        puts("Usage: as <y86 assembler file>");
    std::string infile(argv[1]);
    int p = infile.find(".ya");
    std::string outfile = infile.replace(p, 3, ".yo");
    freopen(infile.c_str(), "r", stdin);
    freopen(outfile.c_str(), "w", stdout);
    line_cnt = 1;
    ch = getchar();
    getToken();
    compile();
    return 0;
}
