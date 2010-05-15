/***************************************************************************
 *   Copyright (C) 2007 by Mate SOOS   *
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

#include "dataelement.h"
#include "assert.h"
#include "cipherdesc.h"

#include <sstream>

using std::cout;
using std::endl;
using std::stringstream;

DataElement::DataElement(const element_type _type, const uint _which_of_type, const uint _index):
        type(_type)
        , which_of_type(_which_of_type)
        , index(_index)
{
    switch (type) {
    case output_type:
        assert(which_of_type == std::numeric_limits<unsigned int>::max() || which_of_type < cpd.no_ciphers);
        break;
    case sr_type:
        assert(which_of_type == std::numeric_limits<unsigned int>::max() || which_of_type < cpd.sr_num);
        break;
    case filter_type:
        assert(which_of_type  == std::numeric_limits<unsigned int>::max() || which_of_type < cpd.filter_num);
        break;
    case help_type:
    case any_type:
        break;
    default:
        assert(false);
    }
}

bool DataElement::matches(const DataElement& e) const
{
    if (e.type != any_type && e.type != type) return false;
    if (e.which_of_type != std::numeric_limits<unsigned int>::max() && e.which_of_type != which_of_type) return false;
    if (e.index != std::numeric_limits<unsigned int>::max() && e.index != index) return false;

    return true;
}

bool DataElement::operator<(const DataElement& e) const
{
    if (type > e.type) return false;
    if (type < e.type) return true;

    if (which_of_type > e.which_of_type) return false;
    if (which_of_type < e.which_of_type) return true;

    if (index > e.index) return false;
    if (index < e.index) return true;

    //they are the same
    return false;
}

bool DataElement::operator==(const DataElement& e) const
{
    if (type != e.type || which_of_type != e.which_of_type || index != e.index) return false;
    return true;
}

bool DataElement::operator!=(const DataElement& e) const
{
    return !(*this == e);
}

ostream& operator << (ostream& os, const DataElement& s)
{
    switch (s.type) {
    case help_type:
        os << "help";
        break;
    case filter_type:
        os << "f";
        break;
    case sr_type:
        os << "sr";
        break;
    case output_type:
        os << "output";
        break;
    default:
        assert(false);
    }

    if (s.which_of_type != std::numeric_limits<unsigned int>::max()) os << s.which_of_type;
    if (s.index != std::numeric_limits<unsigned int>::max()) os << "[" << s.index << "]";

    return os;
}

string DataElement::get_desc() const
{
    stringstream ss;
    ss << *this;
    return ss.str();
}

ostream& operator << (ostream& os, const list<DataElement>& s)
{
    list<DataElement>::const_iterator it = s.begin();
    for (; it != s.end();) {
        os << *it;
        it++;
        if (it != s.end()) cout << "*";
    }

    return os;
}



