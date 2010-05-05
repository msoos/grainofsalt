/******************************************************************************************[Main.C]
MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
CryptoMiniSat -- Copyright (c) 2009 Mate Soos

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include <ctime>
#include <cstring>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <vector>
using std::vector;

//=================================================================================================
// DIMACS Parser:

#define CHUNK_LIMIT 1048576

class StreamBuffer
{
    FILE *  in;
    char    buf[CHUNK_LIMIT];
    int     pos;
    int     size;

    void assureLookahead() {
        if (pos >= size) {
            pos  = 0;
            size = fread(buf, 1, sizeof(buf), in);
        }
    }

public:
    StreamBuffer(FILE * i) : in(i), pos(0), size(0) {
        assureLookahead();
    }

    int  operator *  () {
        return (pos >= size) ? EOF : buf[pos];
    }
    void operator ++ () {
        pos++;
        assureLookahead();
    }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template<class B>
static void skipWhitespace(B& in)
{
    while ((*in >= 9 && *in <= 13) || *in == 32)
        ++in;
}

template<class B>
static void skipLine(B& in)
{
    for (;;) {
        if (*in == EOF || *in == '\0') return;
        if (*in == '\n') {
            ++in;
            return;
        }
        ++in;
    }
}

template<class B>
static void untilEnd(B& in, char* ret)
{
    for (;;) {
        if (*in == EOF || *in == '\0') return;
        if (*in == '\n') {
            return;
        }
        *ret = *in;
        ret++;
        *ret = '\0';
        ++in;
    }
}


template<class B>
static int parseInt(B& in)
{
    int     val = 0;
    bool    neg = false;
    skipWhitespace(in);
    if      (*in == '-') neg = true, ++in;
    else if (*in == '+') ++in;
    if (*in < '0' || *in > '9') printf("PARSE ERROR! Unexpected char: %c\n", *in), exit(3);
    while (*in >= '0' && *in <= '9')
        val = val*10 + (*in - '0'),
              ++in;
    return neg ? -val : val;
}

inline std::string stringify(uint x)
{
    std::ostringstream o;
    o << x;
    return o.str();
}

template<class B>
static void parseString(B& in, std::string& str)
{
    str.clear();
    skipWhitespace(in);
    while (*in != ' ' && *in != '\n' && *in != EOF) {
        str += *in;
        ++in;
    }
}

template<class B>
static void readClause(B& in, vector<lbool>& solved)
{
    int parsed_lit;
    uint32_t var;
    for (;;) {
        parsed_lit = parseInt(in);
        if (parsed_lit == 0) break;
        var = abs(parsed_lit)-1;
        if (solved.size() < var+1)
            solved.resize(var+1, l_Undef);
        solved[var] = (parsed_lit < 0) ? l_False : l_True;
    }
}

template<class B>
static bool match(B& in, const char* str)
{
    for (; *str != 0; ++str, ++in)
        if (*str != *in)
            return false;
    return true;
}


template<class B>
static void parse_DIMACS_main(B& in, vector<lbool>& solved)
{
    string str;
    
    for (;;) {
        skipWhitespace(in);
        switch (*in) {
        case EOF:
            return;
        case 'c':
            ++in;
            skipLine(in);
            break;
        case 'v':
            ++in;
            readClause(in, solved);
            skipLine(in);
            break;
        case 's':
            ++in;
            parseString(in, str);
            if (str != "SATISFIABLE") {
                std::cout << "not satisfiable!!" << std::endl;
                exit(-1);
            }
            break;
        default:
            skipLine(in);
            break;
        }
    }
}

// Inserts problem into solver.
//
static void parse_DIMACS(FILE * input_stream, vector<lbool>& solved)
{
    StreamBuffer in(input_stream);
    parse_DIMACS_main(in, solved);
}

