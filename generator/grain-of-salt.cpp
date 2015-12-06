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

#include "assert.h"
#include "string.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <iomanip>

#include "grain-of-salt.h"
#include "debug.h"
#include "output_maker.h"
#include "timer.h"
#include "defines.h"
#include "debug.h"
#include "equationholder.h"
#include "equationstosat.h"
#include "cipher.h"
#include "limits.h"
#include "cipherdesc.h"
#include "solverattrib.h"
#include "filereader.h"

using std::map;
using std::vector;
using std::pair;
using boost::lexical_cast;
using std::cout;
using std::endl;

GrainOfSalt::GrainOfSalt(const uint _guess_bits, const bool _sr_best_bits_from_file) :
        bestBits(this, _sr_best_bits_from_file, _guess_bits)
        , cnfNum(0)
{
    randomize_tweak();
}

void GrainOfSalt::randomize_tweak()
{
    tweak.resize(cpd.sr_num);
    for (uint i = 0; i < cpd.sr_num; i++) {
        tweak[i].resize(cpd.sr_size[i]);
        for (uint i2 = 0; i2 < cpd.sr_size[i]; i2++)
            tweak[i][i2] = cpd.mtrand.randInt(1);
    }
}

void GrainOfSalt::speed_test(const uint test_number)
{
    for (uint test_no = 0; test_no < test_number; test_no++) {
        cout << "Test number: " << test_no << endl;

        randomize_tweak();
        bestBits.reload_best_bits();
        SolverAttrib solverAttrib;
        EquationHolder eqHolder;

	random_test(solverAttrib, eqHolder, cpd.mtrand);
        
        cpd.vars.clear_internal_vars();
    }
}

double GrainOfSalt::debug(const uint num)
{
    double cumulated_solver_time = 0;
    for (uint test_no = 0; test_no < num; test_no++) {
        cout << "-----" << endl << "Test number: " << test_no << endl;

        randomize_tweak();
       
        vector<vector<bool> > sr_state = generate_random_state(cpd.random_state_init(cpd.mtrand));
        map<DataElement, bool> given_data;
        vector<vector<bool> > filter_state = Cipher::calculate_filters_and_states(sr_state, dataHolder);
        for (uint i = 0; i < cpd.no_ciphers; i++) {
            Cipher::output_for_input(i, sr_state, filter_state, dataHolder, given_data);
        }
        
        add_tweakable_to_given_data(given_data);
        
        if (cpd.verbose)
            print_outputs_and_states(given_data, sr_state, filter_state);

        EquationHolder eqHolder;
        BOOST_FOREACH(const DataElement& v, bestBits.varstoguess) switch (v.type) {
        case sr_type:
            given_data[v] = sr_state[v.which_of_type][v.index];
            break;
        default:
            assert(false);
        }

        add_given_data(given_data, eqHolder);

        SolverAttrib solverAttrib;
        one_test(solverAttrib, eqHolder, given_data);
        
        /*if (printed_file) {
            boost::filesystem::path filename = SolverAttrib::get_satfile_dir();
            filename /= cnf_filename(given_data) + ".output";
            ofstream file;
            file.open(filename.file_string().c_str());
            for (uint i = 0; i < cpd.sr_num; i++)
                check_found_state(file, solverAttrib, eqHolder, sr_state[i], i);
            file.close();
            
            boost::filesystem::path filename2 = SolverAttrib::get_satfile_dir();
            filename2 /= cnf_filename(given_data) + ".cnf";
            string toexec;
            toexec += "./cryptominisat \"";
            toexec += filename2.file_string();
            toexec += "\" > output.out";
            std::cout << "executing :" << toexec << std::endl;
            system(toexec.c_str());
            
            vector<lbool> output = readOutput("output.out");
            for (uint i = 0; i < cpd.sr_num; i++)
                check_found_state2(output, solverAttrib, eqHolder, sr_state[i], i);
            std::cout << "All states have been checked and are OK!" << std::endl;
        }*/
        
        cpd.vars.clear_internal_vars();
    }
    return cumulated_solver_time;
}

void GrainOfSalt::best_bit_test(const uint test_number)
{
    bestBits.best_bit_test(test_number);
}

bool GrainOfSalt::random_test(SolverAttrib& solverAttrib, EquationHolder& eqHolder, MTRand& mtrand)
{
    map<DataElement, bool> given_data;
    vector<vector<bool> > sr_state = generate_random_state(cpd.random_state_init(mtrand));
    vector<vector<bool> > filter_state = Cipher::calculate_filters_and_states(sr_state, dataHolder);
    for (uint i = 0; i < cpd.no_ciphers; i++)
        Cipher::output_for_input(i, sr_state, filter_state, dataHolder, given_data);

    add_tweakable_to_given_data(given_data);

    bool good_guess = true;
    BOOST_FOREACH(const DataElement& v, bestBits.varstoguess) {
        assert(v.type == sr_type);
        bool toguess = mtrand.randInt(1);
        if (sr_state[v.which_of_type][v.index] != toguess)
            good_guess = false;
        given_data[v] = toguess;
    }

    add_given_data(given_data, eqHolder);

    bool printed_file = one_test(solverAttrib, eqHolder, given_data);

    if (printed_file) {
        boost::filesystem::path filename = SolverAttrib::get_satfile_dir();
        filename /= cnf_filename(given_data) + ".output";
        ofstream file;
        file.open(filename.file_string().c_str());
        
        if (good_guess) {
            for (uint i = 0; i < cpd.sr_num; i++)
                check_found_state(file, solverAttrib, eqHolder, sr_state[i], i);
        } else {
            file << "UNSAT" << endl;
        }
        file.close();
    }
    
    return printed_file;
}

bool GrainOfSalt::one_test(SolverAttrib& solverAttrib, EquationHolder& eqHolder, const map<DataElement, bool>& given_data)
{
    if (cpd.verbose) {
        cout << "outputs        : " << cpd.outputs << endl;
        cout << "init_clock     : " << cpd.init_clock << endl;
        for (uint i = 0; i < cpd.sr_num; i++)
            cout << "SR" << i << " shift: " << cpd.sr_shift[i] << endl;
        for (uint i = 0; i < cpd.sr_num; i++) {
            uint guessed = 0;
            BOOST_FOREACH(const DataElement& v, bestBits.varstoguess)
            if (v.which_of_type == i) guessed++;
            cout << "SR" << i << " help bits: " << guessed << "/" << cpd.sr_size[i];
            if (( !cpd.one_out[i].empty()
                    || !cpd.zero_out[i].empty()
                    || !cpd.tweakable[i].empty()
                )
                    && cpd.init_clock > 0
               ) {
                cout << " (";
                if (!cpd.one_out[i].empty())
                    cout << " 1-ed out:" << cpd.one_out[i].size() << ",";
                if (!cpd.zero_out[i].empty())
                    cout << " 0-ed out:" << cpd.zero_out[i].size() << ",";
                if (!cpd.tweakable[i].empty())
                    cout << " tweaked:" << cpd.tweakable[i].size();
                cout << " )";
            }
            cout << endl;
        }
    }

    double eq_making_time = cpuTime();
    if (cpd.verbose) cout << "Generating eqs..." << std::flush;

    OutputMaker output_maker(dataHolder);
    for (uint time = 0; time < cpd.init_clock; time++) {
        for (uint i = 0; i < cpd.sr_num; i++)
            output_maker.generate(eqHolder, time, DataElement(sr_type, i, std::numeric_limits<unsigned int>::max()));
        for (uint i = 0; i < cpd.filter_num; i++)
            output_maker.generate(eqHolder, time, DataElement(filter_type, i, std::numeric_limits<unsigned int>::max()));
    }

    for (uint time = cpd.init_clock; time < cpd.init_clock + cpd.outputs; time++) {
        for (uint i = 0; i < cpd.sr_num; i++)
            output_maker.generate(eqHolder, time, DataElement(sr_type, i, std::numeric_limits<unsigned int>::max()));
        for (uint i = 0; i < cpd.filter_num; i++)
            output_maker.generate(eqHolder, time, DataElement(filter_type, i, std::numeric_limits<unsigned int>::max()));
        for (uint i = 0; i < cpd.no_ciphers; i++)
            output_maker.generate(eqHolder, time, DataElement(output_type, i, std::numeric_limits<unsigned int>::max()));
    }

    if (cpd.verbose)
        cout << "Done (time: " << cpuTime() - eq_making_time << "s)" << endl;

    //double cpu_time = cpuTime();
    lbool ret = l_Undef;
    if (cpd.propagateFacts) ret &= eqHolder.eliminate_trivial_and_mono_equations();
    if (ret != l_Undef && cpd.verbose) {
        cout << "Solved by EquationHolder's eliminate_trivial_equations" << endl;
        return false;
    }
    
    if (cpd.linearizeLinearizeable) eqHolder.linearize_linearizable();
    eqHolder.simplify_equations();
    eqHolder.remove_unneeded();
    if (cpd.print_stats) eqHolder.print_equations();

    EquationsToSat eqToSat(eqHolder.get_equations(), solverAttrib);
    difficulty = eqToSat.convert();
    
    cnfNum++;
    solverAttrib.print_satfile(cnf_filename(given_data) + ".cnf", eqHolder.get_same_vars());
    return true;
}

void GrainOfSalt::add_given_data(const map<DataElement, bool>& given_data, EquationHolder& eqHolder) const
{
    typedef pair<DataElement, bool> mypair;
    BOOST_FOREACH(const mypair& g, given_data) {
        eqHolder.add_eq(
            DataElement(help_type, g.first.which_of_type, std::numeric_limits<unsigned int>::max())
            , Monomial(cpd.vars.get_array_var(g.first), false)
            , new Polynomial(g.second)
        );
    }
}

vector<vector<bool> > GrainOfSalt::generate_random_state(const uint state_init) const
{
    MTRand mtrand(state_init);
    
    vector<vector<bool> > sr_state(cpd.sr_num);
    for (uint i = 0; i < cpd.sr_num; i++)
        for (uint i2 = 0; i2 < cpd.sr_size[i]; i2++)
            sr_state[i].push_back(mtrand.randInt(1));

    if (cpd.init_clock > 0) {
        for (uint i = 0; i < cpd.sr_num; i++) {
            BOOST_FOREACH(uint var, cpd.zero_out[i])
                sr_state[i][var] = 0;
            BOOST_FOREACH(uint var, cpd.one_out[i])
                sr_state[i][var] = 1;
            BOOST_FOREACH(uint var, cpd.tweakable[i])
                sr_state[i][var] = tweak[i][var];
        }
    }

    for (uint i = 0; i < cpd.sr_num; i++)
        assert(sr_state[i].size() == cpd.sr_size[i]);

    return sr_state;
}

void GrainOfSalt::check_found_state2(const vector<lbool>& output, const SolverAttrib& solverAttrib, const EquationHolder& eqHolder, const vector<bool>& hex_state, const uint sr) const
{
    //lbool good = l_True;
    for (uint i = cpd.sr_shift[sr]; i < cpd.sr_size[sr] + cpd.sr_shift[sr]; i++) {
        uint var = cpd.vars.get_array_var(sr_type, sr, i);
        if (eqHolder.get_same_vars().find(var) != eqHolder.get_same_vars().end())
            var = eqHolder.get_same_vars().find(var)->second;
        assert(output.size() > solverAttrib.get_mixed_var(var));
        lbool should_be = hex_state[i] ? l_True : l_False;
        if (output[solverAttrib.get_mixed_var(var)] != should_be) {
            std::cout << "The content of sr[" << sr <<"][" << i << "] is incorrect!!!" << std::endl;
            std::cout << "expected: " << should_be.getBool() << std::endl;
            std::cout << "got: ";
            if (output[solverAttrib.get_mixed_var(var)].isUndef())
                std::cout << "UNDEF" << std::endl;
            else {
                if (output[solverAttrib.get_mixed_var(var)].getBool())
                    std::cout << "TRUE" << std::endl;
                else
                    std::cout << "FALSE" << std::endl;
            }
            exit(-1);
        }
    }
}

void GrainOfSalt::check_found_state(ofstream& file, const SolverAttrib& solverAttrib, const EquationHolder& eqHolder, const vector<bool>& hex_state, const uint sr) const
{
    //lbool good = l_True;
    for (uint i = cpd.sr_shift[sr]; i < cpd.sr_size[sr] + cpd.sr_shift[sr]; i++) {
        uint var = cpd.vars.get_array_var(sr_type, sr, i);
        if (eqHolder.get_same_vars().find(var) != eqHolder.get_same_vars().end())
            var = eqHolder.get_same_vars().find(var)->second;
        file << solverAttrib.get_mixed_var(var)+1 << "\t" << std::boolalpha << hex_state[i] << endl;
    }
}

vector<bool> GrainOfSalt::get_output(const uint which, const map<DataElement, bool>& given_data) const
{
    vector<bool> output;
    uint i = 0;
    while (given_data.find(DataElement(output_type, which, i)) != given_data.end())
        output.push_back(given_data.find(DataElement(output_type, which, i++))->second);
    return output;
}

void GrainOfSalt::add_tweakable_to_given_data(map<DataElement, bool>& given_data) const
{
    if (cpd.init_clock > 0) for (uint i = 0; i < cpd.sr_num; i++) {
        BOOST_FOREACH(const uint state, cpd.one_out[i])
            given_data[DataElement(sr_type, i, state)] = true;

        BOOST_FOREACH(const uint state, cpd.zero_out[i])
            given_data[DataElement(sr_type, i, state)] = false;

        BOOST_FOREACH(const uint state, cpd.tweakable[i])
            given_data[DataElement(sr_type, i, state)] = tweak[i][state];
    }
}

vector<lbool> GrainOfSalt::readOutput(const string filename)
{
    vector<lbool> solved;
    
    FILE * in = fopen(filename.c_str(), "rb");
    if (in == NULL) {
        cout << "ERROR! Could not open file: " << filename << std::endl;
        exit(1);
    }
    
    parse_DIMACS(in, solved);
    
    fclose(in);
    return solved;
}

void GrainOfSalt::print_outputs_and_states(const map<DataElement, bool>& given_data, const vector<vector<bool> >& sr_state, const vector<vector<bool> >& filter_state) const {
    cout << std::endl;
    for (uint i = 0; i < cpd.no_ciphers; i++) {
        cout << "Output" << i << " in hex: 0x" << Debug::hexify(get_output(i, given_data)) << endl;
        cout << "Output" << i  << " in bits: " << Debug::printbits(get_output(i, given_data)) << endl;
    }
    
    cout << std::endl;
    for (uint i = 0; i < cpd.sr_num; i++) {
        cout << "Full SR" << i  << " in hex: 0x" <<  Debug::hexify(sr_state[i]) << endl;
        cout << "Full SR" << i  << " in bit: " << Debug::printbits(sr_state[i]) << endl;
    }
    
    cout << std::endl;
    for (uint i = 0; i < cpd.filter_num; i++) {
        cout << "Full filter" << i  << " in hex: 0x" <<  Debug::hexify(filter_state[i]) << endl;
        cout << "Full filter" << i  << " in bit: " << Debug::printbits(filter_state[i]) << endl;
    }
    cout << std::endl;
}

string GrainOfSalt::cnf_filename(const map<DataElement, bool>& given_data) const
{
    stringstream name;
    name << cpd.ciphername << "-" << bestBits.varstoguess.size() << "-" << cpd.outputs << "-";
    for (uint i = 0; i < cpd.sr_num; i++) {
        name << cpd.sr_shift[i];
        if (i != cpd.sr_num-1) name << "s";
    }
    name << "-";
    
    for (uint i = 0; i < cpd.no_ciphers; ) {
        name << "0x" << Debug::hexify(get_output(i, given_data));
        i++;
        if (i < cpd.no_ciphers)
            name << "-";
    }
    name << "-" <<  cnfNum; 
    
    return name.str();
}
