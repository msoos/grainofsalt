//
// C++ Implementation: dataelementrange
//
// Description:
//
//
// Author: Mate Soos <mate.soos@inrialpes.fr> (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
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


