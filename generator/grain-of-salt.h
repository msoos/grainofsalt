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
#ifndef GRAINOFSALT_H
#define GRAINOFSALT_H

#include <cstdlib>
#include <vector>
#include <map>
#include <gmp.h>

#include "debug.h"
#include "defines.h"
#include "MersenneTwister.h"
#include "output_maker.h"
#include "data_holder.h"
#include "SolverTypes.h"
#include "bestbits.h"
#include "solverattrib.h"

using std::vector;
using std::map;
using std::string;
using std::ostream;
namespace bs = boost::filesystem;

class EquationHolder;

/**
@brief Sets up and executes the solver

Basically, if the preferences are set, then speed_test() or debug() sets up everyting as it should and does a given number of tests.
*/

class GrainOfSalt
{
public:

    friend ostream& operator << (ostream& os, const DataElement& v); ///<Easily print DataElement-s

    GrainOfSalt(const uint guess_bits, const bool sr_best_bits_from_file);

    /**
    @brief Do a speed test: The guessed bits are randomly set
    The result is probably UNSAT. The verbosity is turned down.
    @param num Do the test this number of times
    */
    void speed_test(const uint num);
    
    /**
    @brief Do a debug run: the guessed bits are correctly set
    The result should be SAT - if not, there is an error. The verbosity is turned up
    @param num Do the test this number of times
    */
    double debug(const uint num);
    
    /**
    @brief Uses the BestBits class to calculate best bits.
    The best bits are for a given number of outputs and shifts.
    @param given_output If this parameter is omitted, the generated best bit will be good for all outputs. Otherwise, it will be specific to the given output
    @param test_number the number of times the test should run. The higher, the more accurate (but also more time-consuming)
    */
    void best_bit_test(const uint test_number);

    friend class BestBits;
    uint difficulty;
private:
    /**
    @brief Do one random test. Random, as the guessed bits will be randomly set.
    @param S The solver to use
    @param eqHolder the EquationHolder that will contain the equations
    @param mtrand The random source to use
    @param given_output If given, the output will not be randomized. Oherwise, it is randomized (by choosing a random state and generating the output using the Cipher class)
    @return The test run's ouptut 
    */
    bool random_test(SolverAttrib& solverAttrib, EquationHolder& eqHolder, MTRand& mtrand);
    
    /**
    @brief Do one test with an already set-up solver.
    The output and best bits are all set at this point.
    @param S The solver to use
    @param given_data the output and the best bits
    @param eqHolder the EquationHolder that will contain the equations
    @return The test run's ouptut
    */
    bool one_test(SolverAttrib& solverAttrib, EquationHolder& eqHolder, const map<DataElement, bool>& given_data);

    void add_tweakable_to_given_data(map<DataElement, bool>& given_data) const;
    void add_given_data(const map<DataElement, bool>& guess, EquationHolder& eqHolder) const;
    vector<vector<bool> > generate_random_state(const uint rand) const;
    vector<vector<bool> > state_from_string(string state) const;
    vector<bool> get_output(const uint which, const map<DataElement, bool>& given_data) const;
    void print_outputs_and_states(const map<DataElement, bool>& given_data, const vector<vector<bool> >& sr_state, const vector<vector<bool> >& filter_state) const;

    /**
    @brief Check that the important part of the state found by the solver is in fact correct. 
    
    The checked part of the state that is at shift... shift+state_size
    @param sr the shift register whose content we must check
    @param hex_state the state that is expected
    @param S the solver that contains the solution
    */
    void check_found_state(ofstream& file, const SolverAttrib& solverAttrib, const EquationHolder& eqHolder, const vector<bool>& hex_state, const uint sr) const;
    void check_found_state2(const vector<lbool>& output, const SolverAttrib& solverAttrib, const EquationHolder& eqHolder, const vector<bool>& hex_state, const uint sr) const;
    vector<lbool> readOutput(const string filename);
    void randomize_tweak();
    
    static bool var_in_impossible_position(const DataElement& v);
    uint cnfNum;
    
    string cnf_filename(const map<DataElement, bool>& given_data) const;

    vector<vector<bool> > tweak; ///<The tweakable bits are set here.

    FuncDataHolder dataHolder; ///<The functions of the stream cipher are stored here

    BestBits bestBits; ///<Store and calculate best bits
};

#endif //GRAINOFSALT
