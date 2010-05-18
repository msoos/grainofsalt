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
#include "cipherdesc.h"
#include "defines.h"
#include "string.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>

#include "mystack.h"
#include "monomial.h"
#include "polynomial.h"

using namespace boost::spirit;
using boost::lexical_cast;
using std::ifstream;
using std::cout;
using std::endl;

CipherDesc cpd;

CipherDesc::CipherDesc() :
        outputs(0)
        , mixed_shifts(false)
        , noExtendedMonomial(false)
        , linearizeLinearizeable(false)
        , verbose(false)
        , mtrand(0x1234)
        , print_stats (false)
        , permutateClauses(false)
        , permutateVars(false)
        , propagateFacts(true)
        , cnfDir("satfiles")
        , fixed_state_init(false)
{
}

Monomial* CipherDesc::get_free_mixedMonos()
{
    boost::mutex::scoped_lock scoped_lock(mixedMonosStack_mutex);
    if (mixedMonosStack.empty()) {
        return new Monomial[Polynomial::maxMonos];
    }

    Monomial* tmp = mixedMonosStack.top();
    mixedMonosStack.pop();

    return tmp;
}

void CipherDesc::return_mixedMonos(Monomial* mixedMonos)
{
    boost::mutex::scoped_lock scoped_lock(mixedMonosStack_mutex);
    mixedMonosStack.push(mixedMonos);
}

MonoStack* CipherDesc::get_free_monoStack()
{
    boost::mutex::scoped_lock scoped_lock(monoStackStack_mutex);
    if (monoStackStack.empty()) {
        return new MonoStack;
    }

    MonoStack* tmp = monoStackStack.top();
    monoStackStack.pop();

    return tmp;
}

void CipherDesc::return_monoStack(MonoStack* monoStack)
{
    boost::mutex::scoped_lock scoped_lock(monoStackStack_mutex);
    monoStackStack.push(monoStack);
}

void CipherDesc::set_name(const string& _ciphername)
{
    ciphername = _ciphername;
    vars.full_reset();
    sr_num = 0;
    filter_num = 0;
    init_clock = 0;
    
    sr_size.clear();
    zero_out.clear();
    one_out.clear();
    tweakable.clear();
    sr_shift.clear();
    linearizable_sr_during_init.clear();
    linearizable_sr_during_norm.clear();
    mixed_shifts = false;
    no_ciphers = 1;

    ifstream file;
    bs::path filename = get_cipher_path();
    filename /= "config";
    file.open(filename.native_file_string().c_str());
    if (!file)
        throw("Cannot find cipher description file (" + string(ciphername) + ")!");

    uint line = 1;
    string lineread;
    while (std::getline(file, lineread)) { // read line by line
        switch (line) {
        case 1:
            //assert(lineread.substr(0, 10) == string("sr_size = "));
            if (!parse_vec(lineread, "sr_size", sr_size).full)
                throw("Cannot parse 'sr_size' line " + lexical_cast<string>(line) + ":" + "\n'" + lineread + "'\n");
            sr_num = sr_size.size();
            assert(sr_num > 0);

            zero_out.resize(sr_num);
            one_out.resize(sr_num);
            tweakable.resize(sr_num);

            cout << "Shift register sizes : ";
            copy(sr_size.begin(), sr_size.end(),
                 std::ostream_iterator<uint>(cout, ","));
            cout << endl;
            break;
        case 2:
            if (!parse_vec(lineread, "linearizable_sr_during_init", linearizable_sr_during_init).full)
                throw("Cannot parse 'linearizable_sr_during_init' line " + lexical_cast<string>(line) + ":" + "\n'" + lineread + "'");

            cout << "Linearizable SRs during init : ";
            copy(linearizable_sr_during_init.begin(), linearizable_sr_during_init.end(),
                 std::ostream_iterator<uint>(cout, ","));
            cout << endl;

            break;
        case 3:
            if (!parse_vec(lineread, "linearizable_sr_during_norm", linearizable_sr_during_norm).full)
                throw("Cannot parse 'linearizable_sr_during_norm' line " + lexical_cast<string>(line) + ":" + "\n'" + lineread + "'");

            cout << "Linearizable SRs during normal operation : ";
            copy(linearizable_sr_during_norm.begin(), linearizable_sr_during_norm.end(),
                 std::ostream_iterator<uint>(cout, ","));
            cout << endl;

            break;
        case 4:
            if (!parse_whatever(lineread, "filters", filter_num).full)
                throw("Cannot parse 'filters' line " + lexical_cast<string>(line) + ":" + "\n'" + lineread + "'");

            cout << "No. filters : " << filter_num << endl;
            break;
        case 5:
            if (!parse_whatever(lineread, "init_clock", init_clock).full)
                throw("Cannot parse 'init_clock' line " + lexical_cast<string>(line) + ":" + "\n'" + lineread + "'");

            cout << "Init clock : " << init_clock << endl;
            break;
        case 6:
            if (!parse_list(lineread, "tweakable", tweakable).full)
                throw("Cannot parse 'tweakable' line " + lexical_cast<string>(line) + ":" + "\n'" + lineread + "'");

            for (uint i = 0; i < sr_num; i++) if (tweakable[i].size() > 0) {
                    cout << "Tweakable : -> SR" << i <<":";
                    copy(tweakable[i].begin(), tweakable[i].end(),
                         std::ostream_iterator<uint>(cout, ","));
                    cout << endl;
                }
            break;
        case 7:
            if (!parse_list(lineread, "one_out", one_out).full)
                throw("Cannot parse 'one_out' line " + lexical_cast<string>(line) + ":" + "\n'" + lineread + "'");

            for (uint i = 0; i < sr_num; i++) if (one_out[i].size() > 0) {
                    cout << "One out : -> SR" << i <<":";
                    copy(one_out[i].begin(), one_out[i].end(),
                         std::ostream_iterator<uint>(cout, ","));
                    cout << endl;
                }
            break;
        case 8:
            if (!parse_list(lineread, "zero_out", zero_out).full)
                throw("Cannot parse 'zero_out' line " + lexical_cast<string>(line) + ":" + "\n'" + lineread + "'");

            for (uint i = 0; i < sr_num; i++) if (zero_out[i].size() > 0) {
                    cout << "Zero out : -> SR" << i <<":";
                    copy(zero_out[i].begin(), zero_out[i].end(),
                         std::ostream_iterator<uint>(cout, ","));
                    cout << endl;
                }
            break;
        }
        line++;
    }
    file.close();
    if (line <= 4) throw("config file is missing some lines! It must be at least 4 lines long!");

    check_within_limits(tweakable, "tweakable");
    check_within_limits(one_out, "one_out");
    check_within_limits(one_out, "zero_out");

    check_dont_clash(one_out, zero_out);
    check_dont_clash(one_out, tweakable);
    check_dont_clash(zero_out, tweakable);

    BOOST_FOREACH(uint sr, linearizable_sr_during_init)
        if (sr >= sr_num) throw("linearizable_sr_during_init is given an SR that doesn't exist. SR-s start numbering from zero!");

    BOOST_FOREACH(uint sr, linearizable_sr_during_norm)
        if (sr >= sr_num) throw("linearizable_sr_during_norm is given an SR that doesn't exist. SR-s start numbering from zero!");


    assert(outputs != 0);
    for (uint i = 0; i < sr_num; i++)
        vars.reserve_array(sr_type, i, sr_size[i] + init_clock + outputs );
    for (uint i = 0; i < filter_num; i++)
        vars.reserve_array(filter_type, i, init_clock + outputs);
    for (uint i = 0; i < no_ciphers; i++)
        vars.reserve_array(output_type, i, cpd.outputs);
    

    sr_shift.resize(sr_num, 0);
}

parse_info<const char*> CipherDesc::parse_vec(const string& str, const char* what, vector<uint>& vec)
{
    return parse(str.c_str(),(chseq<>(what) >> '=' >> ((uint_p[push_back_a(vec)] >> *(',' >> uint_p[push_back_a(vec)])) | end_p)), space_p);
}

parse_info<const char*> CipherDesc::parse_whatever(const string& str, char const* what, uint& equal)
{
    return parse(str.c_str(),(chseq<>(what)  >>'=' >> uint_p[assign_a(equal)]), space_p);
}

parse_info<const char*> CipherDesc::parse_list(const string& str, const char* what, vector<vector<uint> >& vec)
{
    vector<uint> which_sr;
    vector<uint> var_from;
    vector<uint> var_to;

    parse_info<const char*> result = parse(str.c_str(),(chseq<>(what) >> '=' >> *((chseq<>("sr") >> uint_p[push_back_a(which_sr)] >> ch_p('-') >> uint_p[push_back_a(var_from)] >> chseq<>("...") >> uint_p[push_back_a(var_to)]) | ',') >> end_p), space_p);

    if (result.full) {
        for (uint i = 0; i < which_sr.size(); i++) {
            uint sr = which_sr[i];
            for (uint var = var_from[i]; var <= var_to[i]; var++)
                vec[sr].push_back(var);
        }
    }

    return result;
}

void CipherDesc::check_within_limits(const vector<vector<uint> >& to_check, const string name) const
{
    assert(to_check.size() == sr_num);
    for (uint i = 0; i < to_check.size(); i++)
        BOOST_FOREACH(uint var, to_check[i]) if (var >= sr_size[i])
            throw("In config file, " + name +
                  " has an invalid range: SR" + lexical_cast<string>(i) +
                  " only has a size of " + lexical_cast<string>(sr_size[i]) + "!");
}

void CipherDesc::check_dont_clash(const vector<vector<uint> >& check1, const vector<vector<uint> >& check2)
{
    for (uint i = 0; i < check1.size(); i++)
        BOOST_FOREACH(uint var, check1[i])
        if (find(check2[i].begin(), check2[i].end(), var) != check2[i].end())
            throw("In config file, one_out, zero_out and tweakable cannot share variables!");
}

void CipherDesc::set_shift(uint which_sr, uint to_what)
{
    assert(to_what < cpd.init_clock + cpd.outputs);
    assert(which_sr < cpd.sr_num);
    if (cpd.init_clock > 0 && !cpd.tweakable[which_sr].empty() && to_what > 0)
        throw("If an SR is tweakable, and init_clock>0, it cannot be shifted. This is not really a limitation, since if something can be tweaked, this should be taken advantage of. You shifted SR" + lexical_cast<string>(which_sr) + " by " + lexical_cast<string>(to_what) + ". Set its shift to 0 or set init_clock to 0");

    sr_shift[which_sr] = to_what;
}

bs::path CipherDesc::get_cipher_path() const
{
    bs::path full_path( bs::initial_path<bs::path>() );

    full_path /= cpd.ciphername;

    return full_path;
}

const uint CipherDesc::random_state_init(MTRand& mtrand2)
{
    if (!fixed_state_init)
        return mtrand2.randInt();
    else
        return fixed_state_init_state;
}

const uint CipherDesc::get_fixed_state() const
{
    assert(fixed_state_init);
    return fixed_state_init_state;
}

void CipherDesc::set_fixed_state_init(const uint state)
{
    fixed_state_init = true;
    fixed_state_init_state = state;
}
