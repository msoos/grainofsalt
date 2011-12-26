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
#ifndef DATA_HOLDER_H
#define DATA_HOLDER_H

#include <vector>
#include <list>
#include <boost/filesystem/path.hpp>
#include "dataelement.h"

namespace bs = boost::filesystem;
using std::vector;

class FunctionData
{
public:
    FunctionData() : inverted(false) {};
    
    list<list<DataElement> > functiondata;
    bool inverted;
};


/**
@brief Holds the data for the functions used in the cipher

The cipher's functions need to be parsed up only once, so it's convenient to keep them in one place
*/
class FuncDataHolder
{
public:
    FuncDataHolder();

    vector<FunctionData> output_data; ///<The function that generates the output
    ///The functions that generate the feedback of the shift registers during initialisation. Forward version, i.e. to be used after the shifting
    vector<FunctionData> sr_feedback_data_init; 
    ///The functions that generate the feedback of the shift registers during initialisation. Reversed version, i.e. to be used  before the shifting
    vector<FunctionData> sr_feedback_data_reversed_init;
    ///The functions that generate the feedback of the shift registers during normal running. Forward version, i.e. to be used after the shifting
    vector<FunctionData> sr_feedback_data;
    ///The functions that generate the feedback of the shift registers during normal running. Reversed version, i.e. to be used  before the shifting
    vector<FunctionData> sr_feedback_data_reversed;

    vector<FunctionData> filter_data; ///<Filters' (i.e. f-s') functions are stored here

    /**
    @brief Reads in a function file
    @param[in] filename file's name to read
    @param[out] processed parsed function is put here
    */
    void read_file_multiline(const char* filename, FunctionData& processed) const;
    bs::path get_functions_dir() const; ///<Return the directory where the functions' definitions are located

private:
    /**
    @brief Calculates how to reverse-clock the function. This is used for shifting
    @param[in] normal The normal, non-reversed functions
    @param[out] reversed The reversed functions are put here
    */
    static void calculate_sr_feedback_reversed_s(const vector<FunctionData>& normal, vector<FunctionData>& reversed);

    typedef list<list<DataElement> >::iterator DataElementIter; ///<A helper to iterate through data elements
    typedef list<list<DataElement> >::const_iterator DataElementIterConst; ///<A helper to iterate through data elements
};

extern ostream& operator << (ostream& os, const FunctionData& data);

#endif
