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
#ifndef DATAELEMENT_H
#define DATAELEMENT_H

#include <sys/types.h>
#include <iostream>
#include <list>
#include <string>

enum element_type {sr_type, filter_type, output_type, help_type, any_type};

class FuncDataHolder;

using std::ostream;
using std::list;
using std::string;

/**
@brief Holds exactly one data piece: a filter, an output, an SR, etc.

Used throughout so that these variables can be intelligently stored and accessed. This way, these can be sorted, for instance.
UINT_MAX for which_of_type means that it matches any. any_type means that it matches any type. These special things are used for filtering.
*/
class DataElement
{
public:
    DataElement(const element_type type, const uint which_of_type, const uint index);
    bool operator<(const DataElement& e) const; ///<Used for sorting
    bool operator==(const DataElement& e) const; ///<Used for sorting
    bool operator!=(const DataElement& e) const; ///<Used for sorting

    string get_desc() const; ///<Get a string description of the contents
    bool matches(const DataElement& e) const; ///<The two are the "same" in that one encompasses the other

    friend class FuncDataHolder;
    friend ostream& operator << (ostream& os, const DataElement& s);

    element_type type; ///<The type of the element. Can be generic: any_type
    uint which_of_type; ///<Which of the type (e.g. sr1, sr2, etc.). Can be generic: UINT_MAX
    uint index; ///<Which index? Can be generic: UINT_MAX
private:
    typedef list<list<DataElement> >::iterator DataElementIter;
    typedef list<list<DataElement> >::const_iterator DataElementIterConst;
};


extern ostream& operator << (ostream& os, const list<DataElement>& s);

#endif
