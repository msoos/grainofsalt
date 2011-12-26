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

#include "assert.h"
#include <boost/foreach.hpp>

#include "cipher.h"
#include "defines.h"
#include "cipherdesc.h"
#include "debug.h"

using std::cout;
using std::endl;

bool Cipher::calc_func(const uint time, const vector<vector<bool> >& sr_state, const vector<vector<bool> >& filter_state, const FunctionData& data)
{
    assert(sr_state.size() == cpd.sr_num);
    assert(filter_state.size() == cpd.filter_num);
    bool ret = data.inverted;

    BOOST_FOREACH(const list<DataElement>& elemlist, data.functiondata) {
        bool part = true;
        BOOST_FOREACH(const DataElement& elem, elemlist) switch (elem.type) {
        case sr_type:
            assert(elem.which_of_type < cpd.sr_num);
            assert(sr_state[elem.which_of_type].size() > elem.index + time);

            part &= sr_state[elem.which_of_type][elem.index + time];
            break;
        case filter_type:
            assert(elem.which_of_type < cpd.filter_num);
            assert(filter_state[elem.which_of_type].size() > elem.index + time);
            assert(elem.index == 0);

            part &= filter_state[elem.which_of_type][elem.index + time];
            break;
        case any_type:
        case help_type:
        case output_type:
            assert(false && "Not allowed");
        }
        ret ^= part;
    }

    return ret;
}

vector<vector<bool> > Cipher::calculate_filters_and_states(vector<vector<bool> >& sr_state, const FuncDataHolder& dataHolder) {
    vector<vector<bool> > filter_state(cpd.filter_num);

    if (cpd.verbose) for (uint i = 0; i < cpd.sr_num; i++) {
        std::cout << endl;
        cout << "Giving SR" << i  << " state: 0x" <<  Debug::hexify(sr_state[i]) << endl;
        cout << "in bit:" << Debug::printbits(sr_state[i]) << endl;
    }
    

    for (uint time = 0; time < cpd.init_clock; time++) {
        assert(dataHolder.filter_data.size() == cpd.filter_num);
        for (uint i = 0; i < cpd.filter_num; i++) {
            assert(filter_state[i].size() == time);
            bool bit = calc_func(time, sr_state, filter_state, dataHolder.filter_data[i]);
            filter_state[i].push_back(bit);
        }

        assert(dataHolder.sr_feedback_data_init.size() == cpd.sr_num);
        for (uint i = 0; i < cpd.sr_num; i++) {
            assert(sr_state[i].size() == cpd.sr_size[i] + time);
            bool bit = calc_func(time, sr_state, filter_state, dataHolder.sr_feedback_data_init[i]);
            sr_state[i].push_back(bit);
        }
    }

    for (uint time = cpd.init_clock; time < cpd.init_clock + cpd.outputs; time++) {
        assert(dataHolder.filter_data.size() == cpd.filter_num);
        for (uint i = 0; i < cpd.filter_num; i++) {
            assert(filter_state[i].size() == time);
            bool bit = calc_func(time, sr_state, filter_state, dataHolder.filter_data[i]);
            filter_state[i].push_back(bit);
        }

        assert(dataHolder.sr_feedback_data.size() == cpd.sr_num);
        for (uint i = 0; i < cpd.sr_num; i++) {
            assert(sr_state[i].size() == cpd.sr_size[i] + time);
            bool bit = calc_func(time, sr_state, filter_state, dataHolder.sr_feedback_data[i]);
            sr_state[i].push_back(bit);
        }
    }

    return filter_state;
}

void Cipher::output_for_input(const uint cipher_no, const vector<vector<bool> >& sr_state, const vector<vector<bool> >& filter_state, const FuncDataHolder& dataHolder, map<DataElement, bool>& given_data)
{
    for (uint time = cpd.init_clock; time < cpd.init_clock + cpd.outputs; time++) {
        bool output = calc_func(time, sr_state, filter_state, dataHolder.output_data[cipher_no]);
        given_data[DataElement(output_type, cipher_no, time - cpd.init_clock)] = output;
    }
}
