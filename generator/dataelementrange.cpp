/***************************************************************************
 *   Copyright (C) 2007-2011 by Mate SOOS   *
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

#include "dataelementrange.h"

#include "limits.h"
#include "assert.h"
#include "cipherdesc.h"

DataElementRange::DataElementRange(const element_type _type, const uint _which_of_type, const uint _var_from, const uint _var_to) :
        type(_type)
        , which_of_type(_which_of_type)
        , var_from(_var_from)
        , var_to(_var_to)
{
    switch (type) {
    case output_type:
        assert(which_of_type == std::numeric_limits<unsigned int>::max() || which_of_type == 0);
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

    assert(var_from <= var_to);
}


const bool DataElementRange::contains(const DataElement& d) const
{
    if (type != any_type && d.type != type) return false;
    if (which_of_type != std::numeric_limits<unsigned int>::max() && d.which_of_type != which_of_type) return false;
    if (var_from != std::numeric_limits<unsigned int>::max() && d.index < var_from) return false;
    if (var_to != std::numeric_limits<unsigned int>::max() && d.index > var_to) return false;

    return true;
}


