/***************************************************************************
 *   Copyright (C) 2007-2011 by Mate SOOS *
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
#ifndef CIPHER_H
#define CIPHER_H

#include <vector>
#include <list>
#include <map>
#include "gmp.h"

#include "dataelement.h"
#include "data_holder.h"

using std::map;
using std::vector;
using std::list;

/**
@brief Used to generate the output from the state. 

It actually executes the cipher the way it was meant to be executed.
*/
class Cipher
{
public:
    /**
    @brief Given the state of the cipher, calculates its output
    
    The sr_state is filled up with all the neccessary state in time, needed to calculate the requested number of output bits
    @param[in,out] sr_state The state of the cipher given. This is modified to add the states of the cipher as it is clocked for the number of times it is specified in CipherDesc 's "init_clock" + "outputs"
    @param dataHolder The functions of the cipher are stored here
    @param[out] given_data The output of the cipher is directed here
    */
    static void output_for_input(const uint cipher_no, const vector<vector<bool> >& sr_state, const vector<vector<bool> >& filter_state, const FuncDataHolder& dataHolder, map<DataElement, bool>& given_data);
    
    
    static vector<vector<bool> > calculate_filters_and_states(vector<vector<bool> >& sr_state, const FuncDataHolder& dataHolder);

private:
    /**
    @brief Calculates the output of exactly one boolean function
    @param[in] time The output of the function at this time
    @param[in] filter_state the filter functions' outputs until this time
    @param[in] sr_state the state of the cipher until this time
    @return The output of the function calculated
    */
    static bool calc_func(const uint time, const vector<vector<bool> >& sr_state, const vector<vector<bool> >& filter_state, const FunctionData& data);
};

#endif
