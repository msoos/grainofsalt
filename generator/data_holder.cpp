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

#include "assert.h"
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <algorithm>
#include <iostream>
#include <map>
#include <fstream>
#include <boost/spirit/core.hpp>

#include "data_holder.h"
#include "debug.h"
#include "cipherdesc.h"

using boost::lexical_cast;
using std::ifstream;
using std::make_pair;
using std::cout;
using std::endl;

//#define DEBUG
//#define DO_SIMPLIFY

FuncDataHolder::FuncDataHolder()
{
    sr_feedback_data.resize(cpd.sr_num);
    if (cpd.init_clock > 0) sr_feedback_data_init.resize(cpd.sr_num);

    for (uint i = 0; i < cpd.sr_num; i++) {
        bs::path filename;
        filename = get_functions_dir() / ("sr" + lexical_cast<string>(i)) / "feedback.txt";
        read_file_multiline(filename.file_string().c_str(), sr_feedback_data[i]);

        if (cpd.init_clock > 0)  {
            bs::path filename;
            filename = get_functions_dir() / ("sr" + lexical_cast<string>(i)) / "feedback_init.txt";
            read_file_multiline(filename.file_string().c_str(), sr_feedback_data_init[i]);
        }
    }

    calculate_sr_feedback_reversed_s(sr_feedback_data, sr_feedback_data_reversed);
    if (cpd.init_clock > 0)
        calculate_sr_feedback_reversed_s(sr_feedback_data_init, sr_feedback_data_reversed_init);

    filter_data.resize(cpd.filter_num);
    for (uint i = 0; i < cpd.filter_num; i++) {
        bs::path filename;
        filename = get_functions_dir() / ("f" + lexical_cast<string>(i) + ".txt");
        read_file_multiline(filename.file_string().c_str(), filter_data[i]);
#ifdef DO_SIMPLIFY
        simplify(filter_data[i]);
#endif
    }

    output_data.resize(cpd.no_ciphers);
    for (uint i = 0; i < cpd.no_ciphers; i++) {
        assert(i == 0);
        bs::path filename;
        filename = get_functions_dir() / ("output.txt");
        read_file_multiline(filename.file_string().c_str(), output_data[i]);
#ifdef DO_SIMPLIFY
        simplify(output_data[i]);
#endif
    }
}

void FuncDataHolder::read_file_multiline(const char* filename, FunctionData& processed) const
{
    ifstream file;
    file.open(filename);
    //if ( !file ) throw("OOps, file " + string(filename) + " could not be opened!");
    if (cpd.verbose)
        cout << "reading file: " << filename << endl;

    string lineread;
    while (getline(file, lineread)) {
        bool inversion_on_line = false;
        list<DataElement> tmp;
        string::iterator it = lineread.begin();
        while ( it != lineread.end()) {
            element_type type;
            uint which_of_type;
            uint var;

            parse_info<string::iterator> info;

            info = parse(it, lineread.end(), (*space_p >> chseq<>("sr") >> uint_p[assign_a(which_of_type)] >> '-' >> uint_p[assign_a(var)]));

            if (info.hit) {
                type = sr_type;
                if (which_of_type  >= cpd.sr_num)
                    throw("The SR this file refers to doesn't exist!");
                it = info.stop;

                tmp.push_back(DataElement(type, which_of_type, var));
                continue;
            }

            info = parse(it, lineread.end(), (*space_p >> chseq<>("f") >> uint_p[assign_a(which_of_type)]));
            var = 0;
            if (info.hit) {
                type = filter_type;
                if (which_of_type  >= cpd.filter_num)
                    throw("The filter this file refers to doesn't exist!");
                it = info.stop;

                tmp.push_back(DataElement(type, which_of_type, var));
                continue;
            }
            
            info = parse(it, lineread.end(), (*space_p >> chseq<>("x") >> *space_p));
            if (info.hit) {
                if (!tmp.empty()) throw("You cannot have inversion and a monomial in the same line in the function description");
                inversion_on_line = true;
                break;
            }
            
            throw("data file contains bad filter/shift register specifier on line:\n" + lineread + "\npart " + *it + " ! (maybe trailing space?)");
        }
        if (!tmp.empty()) processed.functiondata.push_back(tmp);
        else if (inversion_on_line) processed.inverted ^= true;
        else throw("Ooops, something bad with parsing the function file. Maybe an empty line?");
    }

    processed.functiondata.sort();
    file.close();

    if (processed.functiondata.empty() && processed.inverted == false)
        throw("File didn't contain a single thing!");

    if (cpd.verbose) cout << processed << endl;
}

void FuncDataHolder::calculate_sr_feedback_reversed_s(const vector<FunctionData>& normal, vector<FunctionData>& reversed)
{
    reversed.resize(cpd.sr_num);

    for (uint i = 0; i < cpd.sr_num; i++) {
        cout << "Reversing (init?) feedback for sr" << i << endl;
        if (normal[i].functiondata.empty()) {
            reversed[i] = normal[i];
            continue;
        }
        
        map<const uint, DataElement* const> changeable;
        FunctionData tmp(normal[i]);
        BOOST_FOREACH(list<DataElement>& l, tmp.functiondata) {
            if (l.size() > 1) continue;
            DataElement& d = *l.begin();
            if (d.type == sr_type && d.index == 0)
                changeable.insert(make_pair(d.which_of_type,&d));
        }
        uint assign_to;
        if (changeable.find(i) == changeable.end()) {
            if (changeable.empty())
                throw("Feedback function cannot be reversed, not even in mixed mode!");

            DataElement& d = *changeable.begin()->second;
            d.index = cpd.sr_size[i];
            d.which_of_type = i;
            cpd.mixed_shifts = true;
            assign_to = changeable.begin()->first;
        } else {
            map<const uint, DataElement* const>::iterator it = changeable.find(i);
            DataElement& d = *it->second;
            d.index = cpd.sr_size[i];
            assign_to = i;
        }
        assert(assign_to < cpd.sr_num);

        cout << "-->assigning to feedback SR" << i << " the reverse for feeback SR" << assign_to << endl;
        if (!reversed[assign_to].functiondata.empty())
            throw("Some problem with the automatic feedback function reversing. Probably the mixed-up assigns don't form a cross!");
        reversed[assign_to] = tmp;
    }

    if (cpd.mixed_shifts) for (uint i = 0; i < cpd.sr_num; i++) {
            assert(!reversed[i].functiondata.empty());
#ifdef DEBUG
            cout << "-------------" << endl <<"normal[" << i << "]  :" << sr_feedback_data[i] << endl;
            cout << "-------------" << endl <<"reversed[" << i << "]:" << sr_feedback_data_reversed[i] << endl << endl;
#endif
        }
}

bs::path FuncDataHolder::get_functions_dir() const
{
    bs::path dirname(cpd.get_cipher_path());
    dirname /= "functions";

    return dirname;
}

ostream& operator << (ostream& os, const FunctionData& data)
{
    typedef list<list<DataElement> >::const_iterator DataElementIterConst;

    for (DataElementIterConst it = data.functiondata.begin(); it != data.functiondata.end();) {
        os << *it;
        it++;
        if (it != data.functiondata.end()) os << " + ";
    }
    if (data.inverted)
        os << " + true";

    return os;
}
