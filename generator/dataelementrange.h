//
// C++ Interface: dataelementrange
//
// Description:
//
//
// Author: Mate Soos <mate.soos.mate@gmail.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DATAELEMENTRANGE_H
#define DATAELEMENTRANGE_H

#include "dataelement.h"

class DataElementRange
{
public:
    DataElementRange(const element_type _type, const uint _which_of_type, const uint _var_from, const uint _var_to);

    const element_type type;
    const uint which_of_type;
    const uint var_from;
    const uint var_to;

    const bool contains(const DataElement& d) const;
};

#endif
