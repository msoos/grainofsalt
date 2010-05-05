/***************************************************************************
 *   Copyright (C) 2007 by Mate Soos  *
 *   soos@inrialpes.fr   *
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
#ifndef DEBUG_H
#define DEBUG_H

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iterator>
#include <sys/types.h>

using std::stringstream;
using std::string;
using std::vector;

///Used to store functions that are used in multiple places and that are stateless
class Debug
{
public:
    ///hexifies a vector of bools, returns the string
    static const string hexify(const vector<bool>& vec);
    
    static const string hexify_strange(const vector<bool>& vec);
    
    ///makes a vector of bool from a hexified string
    static const vector<bool> hexToBool_strange(const string& str);
    static const vector<bool> hexToBool(const string& str);
    ///concatenates(divided by commas) what is given to it into a string
    template<class T>
    static string concatenate(const vector<T>& vec);
    
    static vector<bool> stringToBool(const string& str);
    static const vector<bool> reverse(const vector<bool>& orig);
    static const string printbits(const vector<bool>& to_print);
};

template<class T>
string Debug::concatenate(const vector<T>& vec)
{
    stringstream s;
    copy(vec.begin(), vec.end(),
         std::ostream_iterator<T>(s, ","));
    return s.str();
}

#endif
