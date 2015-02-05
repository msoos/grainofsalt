/***************************************************************************
 *   Copyright (C) 2007-2011 by Mate Soos *
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

#include <boost/filesystem/operations.hpp>
#include <boost/thread/thread.hpp>
#include <boost/spirit/home/classic/core.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <fstream>

#include "bestbits.h"
#include "cipherdesc.h"
#include "extendedmonomial.h"
#include "equationstosat.h"
#include "grain-of-salt.h"
#include "dataelement.h"

using boost::lexical_cast;
using std::cout;
using std::endl;
using std::ifstream;

BestBits::BestBits(GrainOfSalt* _grain, const bool _sr_best_bits_from_file, const uint _guess_bits) :
        sr_best_bits_from_file(_sr_best_bits_from_file)
        , guess_bits(_guess_bits)
        , grain(_grain)
{
    reload_best_bits();
}

bs::path BestBits::get_best_bit_dir()
{
    bs::path full_path(cpd.get_cipher_path());

    full_path /= "best-bits-output" + lexical_cast<string>(cpd.outputs) + "-init" + lexical_cast<string>(cpd.init_clock);

    return full_path;
}

bs::path BestBits::get_best_bit_filename()
{
    bs::path full_path(get_best_bit_dir());

    stringstream filename;
    for (uint i = 0; i < cpd.sr_num; i++) {
        filename << "shift" << cpd.sr_shift[i];
        if (i != cpd.sr_num-1) filename << "-";
    }
    full_path /= filename.str();

    return full_path;
}

void BestBits::write_best_bits_file() const
{
    assert(varstoguess.size() == guess_bits);

    bs::path dir(get_best_bit_dir());
    if (!exists(dir))
        bs::create_directory(get_best_bit_dir());


    bs::path filename(get_best_bit_filename());
    if (bs::exists(filename) && !bs::is_regular(filename))
        throw("best bits not a regular file: " + filename.file_string());

    ofstream file;
    file.open( filename.file_string().c_str());
    if (!file.is_open())
        throw("Cannot open best bits file (" + filename.file_string() + ") for writing!");
    BOOST_FOREACH(const DataElement& v, varstoguess)
        file << v << endl;

    file.close();
}

void BestBits::reload_best_bits()
{
    uint cumulated_sr_size = 0;
    uint cumulated_free_sr_positions = 0;
    for (uint i = 0; i < cpd.sr_num; i++) {
        cumulated_sr_size += cpd.sr_size[i];

        //protect from underflow
        assert(cpd.sr_size[i] >= cpd.tweakable[i].size() + cpd.one_out[i].size() + cpd.zero_out[i].size());

        cumulated_free_sr_positions +=  cpd.sr_size[i] - cpd.tweakable[i].size() - cpd.one_out[i].size() - cpd.zero_out[i].size();
    }

    varstoguess.clear();

    if (guess_bits > cumulated_sr_size)
        throw("The cumulated size of all SRs is smaller than the guess bits you gave, " + lexical_cast<string>(guess_bits) +" !");
    if (cpd.init_clock > 0 && guess_bits > cumulated_free_sr_positions)
        throw("The cumulated size of all SRs minus tweakable+one_out+zero_out positions is only " + lexical_cast<string>(cumulated_free_sr_positions) + ", but you gave " + lexical_cast<string>(guess_bits) + " guess bits! Remove some guess bits.");

    if (sr_best_bits_from_file) {
        if (guess_bits > 0)
            varstoguess = read_file(get_best_bit_filename(), guess_bits);
        if (varstoguess.size() < guess_bits)
            throw("The best bits file does not contain enough bit positions. It only contains " + lexical_cast<string>(varstoguess.size()) + " but you requested " + lexical_cast<string>(guess_bits) +" !");
    } else {
        for (uint i2 = 0; i2 < guess_bits;) {
            //TODO this is biased (impossible_position SRs get less). But at least it doesn't hang :O
            uint i = cpd.mtrand.randInt(cpd.sr_num-1);
            uint var = cpd.mtrand.randInt(cpd.sr_size[i] - 1) + cpd.sr_shift[i];
            DataElement toadd(sr_type, i, var);
            if (find(varstoguess.begin(), varstoguess.end(), toadd) != varstoguess.end())
                continue;
            if (cpd.init_clock > 0 && var_in_impossible_position(toadd))
                continue;

            varstoguess.push_back(toadd);
            i2++;
        }
    }

    if (cpd.init_clock > 0) {
        BOOST_FOREACH(const DataElement& v, varstoguess) {
            if (var_in_impossible_position(v))
                throw("Wrong best bits file - tweakable/one_out/zero_out bits are in best-bits!");
        }
    }

    BOOST_FOREACH(const DataElement& v, varstoguess) {
        if (v.index < cpd.sr_shift[v.which_of_type])
            throw("The best bits file contains bits for SR" + lexical_cast<string>(v.which_of_type) + " that are smaller than the shift(" + lexical_cast<string>(cpd.sr_shift[v.which_of_type]) + "). This is not supported, and in any case, it probably is a bad idea - the solving will be slower");
        if (v.index >= cpd.sr_shift[v.which_of_type] + cpd.sr_size[v.which_of_type])
            throw("The best bits file contains bits for SR" + lexical_cast<string>(v.which_of_type) + " that are larger than the shift(" + lexical_cast<string>(cpd.sr_shift[v.which_of_type]) + ") and the size(" + lexical_cast<string>(cpd.sr_size[v.which_of_type]) + ") combined. This is not supported, and in any case, it probably is a bad idea - the solving will be slower");
    }

    if (cpd.verbose) {
        cout << "guesses :";
        copy(varstoguess.begin(), varstoguess.end(),
             std::ostream_iterator<DataElement>(cout, ","));
        cout << endl;
    }

    /*if (cpd.init_clock > 0 && sr_varstoguess[1].size() < cpd.sr_size[1])
    cout << "Warning! cpd.init_clock > 0, but lfsr_help_bits is only " << sr_varstoguess[1].size() << " instead of "  << cpd.sr_size[1] << endl;*/

    assert(cpd.sr_num > 0);
    uint shift = cpd.sr_shift[0];
    if (cpd.mixed_shifts) for (uint i = 0; i < cpd.sr_num; i++)
            if (cpd.sr_shift[i] != shift) throw("Since there were mixed shifts, the shifts must all be the same. This is a bit of an over-reaction, as not all need to be the same, only the mixed ones. To check for only those is a TODO");
}

void BestBits::CalcBestBits::operator()()
{
    ExtendedMonomial::init_temps();
    Polynomial::init_temps();
    EquationsToSat::init_temps();

    MTRand mtrand(time(NULL) + cpd.mtrand.randInt());

    uint my_cumulated_solver_difficulty = 0;

    for (uint i = 0; i < test_per_thread; i++) {
        grain.randomize_tweak();
        SolverAttrib solverAttrib;
        EquationHolder eqHolder;
        eqHolder.add_eq(
            DataElement(help_type, v.which_of_type, v.index)
            , Monomial(cpd.vars.get_array_var(v.type, v.which_of_type, v.index), false)
            , new Polynomial(mtrand.randInt(1))
        );

        grain.random_test(solverAttrib, eqHolder, mtrand);
        my_cumulated_solver_difficulty += grain.difficulty;
        cpd.vars.clear_internal_vars();
    }
    cumulated_solver_difficulty += my_cumulated_solver_difficulty;

    EquationsToSat::delete_temps();
    Polynomial::delete_temps();
    ExtendedMonomial::delete_temps();
}

void BestBits::best_bit_test(const uint test_number)
{
    write_best_bits_file();
    assert(sr_best_bits_from_file);

    only_solver_difficulty = true;

    pair<DataElement, unsigned long int> lowest_difficulty(DataElement(sr_type, 0,0), ULONG_MAX);

    for (uint sr = 0; sr < cpd.sr_num; sr++)
        for (uint bit_test = cpd.sr_shift[sr]; bit_test < cpd.sr_shift[sr] + cpd.sr_size[sr]; bit_test++ ) {
            if (find(varstoguess.begin(), varstoguess.end(), DataElement(sr_type, sr, bit_test)) != varstoguess.end())
                continue;
            if (cpd.init_clock > 0 && var_in_impossible_position(DataElement(sr_type, sr, bit_test)))
                continue;

            cout << "--------NEXT BIT---------: sr" << sr << "-" << bit_test << endl;
            uint cumulated_solver_difficulty = 0;
            CalcBestBits c(*grain, DataElement(sr_type, sr, bit_test), cumulated_solver_difficulty, test_number);
            c.operator()();

            //assert(cumulated_solver_difficulty != 0);
            if ( lowest_difficulty.second > cumulated_solver_difficulty ) {
                lowest_difficulty.second = cumulated_solver_difficulty;
                lowest_difficulty.first = DataElement(sr_type, sr, bit_test);
            }
        }
    assert(lowest_difficulty.second != ULONG_MAX);

    cout << "Lowest difficulty for " << varstoguess.size() << "th best bit: sr" << lowest_difficulty.first.which_of_type << "-" << lowest_difficulty.first.index << "(difficulty: " << lowest_difficulty.second << ")" << endl;

    varstoguess.push_back(DataElement(sr_type, lowest_difficulty.first.which_of_type, lowest_difficulty.first.index));
    guess_bits++;

    write_best_bits_file();
}

vector<DataElement> BestBits::read_file(const bs::path& filename, const uint max_size) const
{
    vector<DataElement> ret;

    if (!bs::exists(filename))
        throw("Cannot find best bits file " + filename.file_string() + "!");

    ifstream file;
    file.open(filename.file_string().c_str());
    string lineread;
    while (getline(file, lineread)) { // read line by line
        uint sr, var;
        if (!parse(lineread.c_str(),(chseq<>("sr") >> uint_p[assign_a(sr)] >> "[" >> uint_p[assign_a(var)] >> "]"), space_p).full)
            throw("Cannot parse best bits file line:\n" + lineread);

        ret.push_back(DataElement(sr_type, sr, var));
        if (ret.size() == max_size) goto end;
    }
end:
    file.close();

    return ret;
}

bool BestBits::var_in_impossible_position(const DataElement& v) const
{
    assert(cpd.init_clock > 0);

    if (find(cpd.tweakable[v.which_of_type].begin(), cpd.tweakable[v.which_of_type].end(), v.index) !=
            cpd.tweakable[v.which_of_type].end())
        return true;
    if (find(cpd.one_out[v.which_of_type].begin(), cpd.one_out[v.which_of_type].end(), v.index) !=
            cpd.one_out[v.which_of_type].end())
        return true;
    if (find(cpd.zero_out[v.which_of_type].begin(), cpd.zero_out[v.which_of_type].end(), v.index) !=
            cpd.zero_out[v.which_of_type].end())
        return true;

    return false;
}

