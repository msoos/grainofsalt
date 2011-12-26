/***************************************************************************
 *   Copyright (C) 2007-2011 by Mate Soos *
 *   soos.mate@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "debug.h"
#include "assert.h"
#include "output_maker.h"
#include "string.h"
#include "defines.h"
#include <gmp.h>
#include <fstream>
#include <boost/foreach.hpp>
#include <string>
using std::string;

using std::cout;
using std::endl;

const string Debug::hexify_strange(const vector<bool>& vec)
{
    assert(vec.size() % 4 == 0);
    static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    string ret;
    const uint vecsize = vec.size();
    for (int i = vecsize-1; i >= 0;) {
        uint tmp = 0;
        for (int i2 = 0; i2 < 4; i2++)
            if (i-i2 >= 0) tmp |= ((uint)vec[i-i2]) << (3-i2);

        assert(tmp < 16);
        ret += hex[tmp];
        i -= 4;
    }

    return ret;
}

const string Debug::hexify(const vector<bool>& vec)
{
    vector<bool> vec2 = vec;
    if (vec2.size()%4 != 0) {
        uint num = 4-(vec2.size()%4);
        for (uint i = 0; i < num; i++)
            vec2.push_back(false);
    }
    assert(vec2.size()%4==0);
    
    static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    
    string ret;
    const uint vecsize = vec2.size();
    for (int i = 0; i < vecsize;) {
        uint tmp = 0;
        for (int i2 = 0; i2 < 4; i2++)
            tmp |= ((uint)vec2[i+i2]) << (3-i2);
            
            assert(tmp < 16);
        ret += hex[tmp];
        i += 4;
    }
    
    return ret;
}

vector<bool> Debug::stringToBool(const string& str)
{
    vector<bool> out;
    for (uint i = 0; i < str.length(); i++) {
        if (str[i] == '0')
            out.push_back(false);
        else if (str[i] == '1')
            out.push_back(true);
        else
            assert(false);
    }
    
    return out;
}

void make_lowercase(std::string& s) {
    BOOST_FOREACH(char& c, s)
        c = std::tolower(c);
}

const vector<bool> Debug::hexToBool_strange(const string& str)
{
    string str2(str);
    make_lowercase(str2);
    
    vector<bool> ret;
    static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    for (int i = str2.length()-1; i >= 0; i--) {
        const char c = str2[i];
        const char* at = std::find(hex, hex+16, c);
        if (at >= hex+16)
            throw "problem of hexadecimal string conversion at '" + str2.substr(i,1) + "' in hex string '" + str2 + "' !";
        uint att = at - hex;
        for (uint i2 = 0; i2 < 4; i2++)
            ret.push_back((att >> i2) & 0x1);
    }

    return ret;
}

const vector<bool> Debug::hexToBool(const string& str)
{
    string str2(str);
    make_lowercase(str2);
    
    vector<bool> ret;
    static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    
    for (int i = 0; i < str2.length(); i++) {
        const char c = str2[i];
        const char* at = std::find(hex, hex+16, c);
        if (at >= hex+16)
            throw "problem of hexadecimal string conversion at '" + str2.substr(i,1) + "' in hex string '" + str2 + "' !";
        uint att = at - hex;
        for (int i2 = 3; i2 >= 0; i2--)
            ret.push_back((att >> i2) & 0x1);
    }
    
    return ret;
}

const string Debug::printbits(const vector<bool>& to_print)
{
    string ret;
    
    for (uint i = 0; i < to_print.size(); i++) {
        if (to_print[i]) ret +="1";
        else ret += "0";
    }
    
    return ret;
}

const vector<bool> Debug::reverse(const vector<bool>& orig)
{
    vector<bool> ret(orig);
    
    //cout << "hexify before:" <<hexify(ret) << endl;
    for (uint i = 0; i < ret.size()/2; i++) {
        bool tmp = ret[i];
        ret[i] = ret[ret.size()-1-i];
        ret[ret.size()-1-i] = tmp;
        //cout << "here " << i << "," << ret.size()-1-i << endl;
    }
    //cout << "hexify after:" <<hexify(ret) << endl;
    
    return ret;
}




