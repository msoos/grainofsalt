/***************************************************************************
 *   Copyright (C) 2007 by Mate Soos *
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

#include "assert.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "limits.h"

#include "output_maker.h"
#include "debug.h"
#include "defines.h"
#include "polynomial.h"
#include "cipherdesc.h"

using boost::lexical_cast;
using std::make_pair;

OutputMaker::OutputMaker(const FuncDataHolder& _dataHolder) :
        dataHolder(_dataHolder)
{
}

void OutputMaker::create_poly_from_function(const uint time, Polynomial& final_poly, const FunctionData& function) const
{
    BOOST_FOREACH(const list<DataElement>& elemlist, function.functiondata) {
        Monomial mono;
        BOOST_FOREACH(const DataElement& elem, elemlist) {
            assert(!(elem.type == filter_type && elem.index != 0));
            assert(elem.type != output_type);
            mono *= make_pair(cpd.vars.get_array_var(elem.type, elem.which_of_type, elem.index + time), false);
        }
        assert(mono.deg() > 0);
        final_poly ^= mono;
    }
    if (function.inverted) final_poly.invert();
}

void OutputMaker::generate(EquationHolder& eqHolder, const uint time, const DataElement& type)
{
    assert(time < cpd.init_clock + cpd.outputs);

    const FunctionData* function;
    uint var, state;
    switch (type.type) {
    case output_type:
    case sr_type:
        assert(time < cpd.init_clock + cpd.outputs);
        break;
    case filter_type:
        break;
    case any_type:
    case help_type:
        throw("This eqtype is not allowed in OutputMaker::generate()");
    }

    switch (type.type) {
    case filter_type: {
        function = &dataHolder.filter_data[type.which_of_type];
        var = cpd.vars.get_array_var(filter_type, type.which_of_type, time);
        state = time;
        break;
    }

    case sr_type: {
        if (time >= cpd.init_clock) { //standard
            if (time < cpd.sr_shift[type.which_of_type]) {
                function = &dataHolder.sr_feedback_data_reversed[type.which_of_type];
                var = cpd.vars.get_array_var(sr_type, type.which_of_type, time);
                state = time;
            } else {
                function = &dataHolder.sr_feedback_data[type.which_of_type];
                var = cpd.vars.get_array_var(sr_type, type.which_of_type, cpd.sr_size[type.which_of_type] + time);
                state = cpd.sr_size[type.which_of_type] + time;
            }
            break;
        } else { //init
            if (time < cpd.sr_shift[type.which_of_type]) {
                function = &dataHolder.sr_feedback_data_reversed_init[type.which_of_type];
                var = cpd.vars.get_array_var(sr_type, type.which_of_type, time);
                state = time;
            } else {
                function = &dataHolder.sr_feedback_data_init[type.which_of_type];
                var = cpd.vars.get_array_var(sr_type, type.which_of_type, cpd.sr_size[type.which_of_type] + time);
                state = cpd.sr_size[type.which_of_type] + time;
            }
            break;
        }
    }

    case output_type: {
        assert(time >= cpd.init_clock);
        assert(time < cpd.init_clock + cpd.outputs);

        function = &dataHolder.output_data[type.which_of_type];
        var = cpd.vars.get_array_var(output_type, type.which_of_type, time - cpd.init_clock);
        state = time - cpd.init_clock;
        break;
    }
    default: {
        state = NULL;
        function = NULL;
        var = NULL;
        assert(false);
    }
    }

    string group_name;
    if (cpd.verbose) group_name = cpd.vars.get_varname_from_varnum(var);
    Polynomial& final_poly = *(new Polynomial(false));
    create_poly_from_function(time, final_poly, *function);

    eqHolder.add_eq(DataElement(type.type, type.which_of_type, state)
                    , Monomial(var, false)
                    , &final_poly);
}

