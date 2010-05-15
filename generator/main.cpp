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

#include <cstdlib>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include "grain-of-salt.h"
#include "extendedmonomial.h"
#include "string.h"
#include "defines.h"
#include <gmp.h>
#include "cipherdesc.h"
#include "equationstosat.h"
#include "time.h"
#include "solverattrib.h"

using boost::lexical_cast;
using std::cout;
using std::endl;
namespace po = boost::program_options;

/**
\author Mate Soos
\mainpage Grain-of-salt an automated stream-cipher problem generator
*/

int main(int argc, char *argv[])
{
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("crypto", po::value<string>(), "cryptographic function to simulate. Must have a directory that contains all functions where the binary is executed")
    ("outputs", po::value<int>(), "set number of output bits produced")
    ("karnaugh", po::value<int>(), "set number of monomials after which karnaugh-table is not used (for pure XOR-s it is never used). Default is 0, i.e. by default karnaugh is not used")
    ("xorcut", po::value<int>(), "set maximum length after which the XOR is cut into at least 2 pieces. Default is 7")
    ("noextmonomials","if set, extended monomials will not be used")
    ("init", po::value<string>(), "possible values: yes/no. Controls whether the initialisation phase of the ciphers are activated")
    ("base-shift", po::value<string>(), "Controls the base shifting. Can only be used to control the reference state variables")
    ("deterBits", po::value<int>(), "Set thie many of the variables that have been determined to be 'best bits' to use when generating the cipher")
    ("genDeterBits", po::value<int>(), "Generate the given number of deterministic bits through a greedy randomised algorithm. NOTE: if the file has already been generated, it will be removed!")
    ("probBits", po::value<int>(), "Set this many reference state variables randomly")
    ("xorclauses", "Use XOR clauses as per CryptoMiniSat (xors will not be cut)")
    ("seed", po::value<int>(), "Seed can be given to generate different CNFs with different runs of the executable. Default is 0")
    ("debug", "If set, all CNF-s will be Satisfiable, since the help bits given will all be correct")
    ("num", po::value<int>(), "The number of problem instances to generate. Default is 1")
    ("stats", "Print statistics to directory 'stats'")
    ("verbose", "Print verbose messages. E.g. functions used, bits set, etc.")
    ("cnfDir", po::value<string>(), string("Put generated CNF files into this directory. By default, it is '" + cpd.cnfDir + "'").c_str())
    ("linearize", "If set, linearizeable shift registers will be linearized, i.e. its feedback function will be calculated from the same set of reference state bits. Default is not to do this.")
    ("permutateVars", "If set, variables will be permutated in the generated CNF")
    ("permutateClauses", "If set, clauses will be permutated in teh generated CNF(s)")
    ;
    
    for (int i = 0; i < argc; i++) {
        cpd.commandlineParams += string(argv[i]);
    }
    
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> >& c) {
        cout << "Some option you gave was wrong. Please give '--help' to get help" << endl;
        return 1;
    }
    
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
   
    if (vm.count("outputs")) {
        cpd.outputs = vm["outputs"].as<int>();
        cout << "Number of outputs set to " << cpd.outputs << endl;
    } else {
        cout << "Number of outputs was not set. You must set it with --outputs NUM. Exiting\n";
        return 1;
    }
    
    if (vm.count("karnaugh")) {
        cpd.max_karnaugh_table = vm["karnaugh"].as<int>();
    } else {
        cpd.max_karnaugh_table = 0;
    }
    cout << "Cut-off for karnaugh-table optimisation: " << cpd.max_karnaugh_table << endl;
    
    if (vm.count("noextmonomials")) {
        cpd.noExtendedMonomial = true;
        cout << "Extended monomials not used" << endl;
    } else {
        cpd.noExtendedMonomial = false;
        cout << "Extended monomials used" << endl;
    }
    
    if (vm.count("xorcut")) {
        SolverAttrib::xor_cut_len= vm["xorcut"].as<int>();
    } else {
        SolverAttrib::xor_cut_len= 7;
    }
    cout << "xor-cut set at:" << SolverAttrib::xor_cut_len << endl;
    
    if (vm.count("crypto")) {
        cpd.set_name(vm["crypto"].as<string>());
        cout << "Loading crypto functions from directory '" << vm["crypto"].as<string>() << "'" << endl;
    } else {
        cout << "Option --crypto=NAME must be set!" << endl;
        return 1;
    }
    
    if (vm.count("init")) {
        if (vm["init"].as<string>() == "yes") {
            cout << "Initialisation clock set to " << cpd.init_clock  << ", the default given in the crypto function's description" << endl;
        } else if (vm["init"].as<string>() == "no") {
            cpd.init_clock = 0;
            cout << "Initialisation disabled. The full state is the unknown" << endl;
        } else {
            cout << "Wrong option at --init=yes/no" << endl;
            return 1;
        }
    }
    
    if (vm.count("base-shift")) {
        vector<uint> base_shifts;
        string line = vm["base-shift"].as<string>();
        //parse(str.c_str(),(chseq<>(what) >> '=' >> ((uint_p[push_back_a(vec)] >> *(',' >> uint_p[push_back_a(vec)])) | end_p)), space_p);
        if (!parse(line.c_str(),(((uint_p[push_back_a(base_shifts)] >> *(',' >> uint_p[push_back_a(base_shifts)])) | end_p)), space_p).full) {
            cout << "Cannot parse 'base-shift' option " << line << "'. It should be comma-delimited" << endl;
            exit(-1);
        }
        if (base_shifts.size() != cpd.sr_num) {
            cout << "option 'base-shift' must contain exactly as many values (comma-delimited) as there are shift registers" << endl;
            cout << "You gave " << base_shifts.size() << " values, but there are " << cpd.sr_num << " shift registers" << endl;
            exit(-1);
        }
        try {
            for (uint i = 0; i < cpd.sr_num; i++) {
                cout << "Setting base shift of sr" << i << " to " << base_shifts[i] << endl;
                cpd.set_shift(i, base_shifts[i]);
            }
        } catch(string s) {
            cout << "Error:" << s << endl;
            return 1;
        }
    } else {
        cout << "No base shifting is used -- all of them are 0 base-shifted" << endl;
    }
    
    if (vm.count("linearize")) {
        cpd.linearizeLinearizeable = true;
        std::cout << "Linearizing linearizeable feedback functions" << endl;
    } else {
        cpd.linearizeLinearizeable = false;
        std::cout << "Not linearizing linearizeable feedback functions" << endl;
    }
    
    if (vm.count("stats")) {
        cpd.print_stats = true;
        cout << "Statistics will be printed to the 'stats' directory." << endl 
        << "-> Only one problem's stats can be printed, so you should set 'num' to 1" << endl
        << "-> Otherwise, only the last problem's stats will be printed" << endl;
    } else {
        cout << "Statistics not printed." << endl;
    }
    
    if (vm.count("permutateVars")) {
        cpd.permutateVars = true;
        cout << "Variables will be permutated in the outputted CNF" << endl;
    }
    
    if (vm.count("permutateClauses")) {
        cpd.permutateClauses = true;
        cout << "Clauses will be permutated in the outputted CNF" << endl;
    }
    
    uint bits_of_help = 0;
    bool told_which_help = false;
    bool use_deterministic_help = false;
    bool genDeterBits = false;
    
    if (vm.count("genDeterBits")) {
        told_which_help = true;
        genDeterBits = true;
        bits_of_help = vm["genDeterBits"].as<int>();
        cout << "Generating " << bits_of_help << " deterministic bits through greedy randomised algorithm" << endl;
    }
    
    if (vm.count("probBits")) {
        if (told_which_help) {
            cout << "You can only have at most one of the following options: genDeterBits, probBits, deterBits" << endl;
            return -1;
        }
        told_which_help = true;
        use_deterministic_help = false;
        bits_of_help =  vm["probBits"].as<int>();
        cout << "Using probabilistic help bit calculation. Help bits given: " << bits_of_help << endl;
    }
    
    if (vm.count("deterBits")) {
        if (told_which_help) {
            cout << "You can only have at most one of the following options: genDeterBits, probBits, deterBits" << endl;
            return -1;
        }
        told_which_help = true;
        use_deterministic_help = true;
        bits_of_help =  vm["probBits"].as<int>();
        cout << "Using deterministic help bit calculation. Help bits given: " << bits_of_help << endl;
    }
    
    if (!told_which_help) {
        cout << "You did not specify which help bit type we should be using. Therefore, using 0 help bits, which will probably mean an impossibly difficult solving (but does not neccessitate help bits)" << endl;
    }
    
    if (vm.count("xorclauses")) {
        SolverAttrib::use_xor_clauses = true;
        cout << "Using XOR clauses in CNF. Beware, this CNF will not work with anything other than CryptoMiniSat." << endl;
    } else {
        cout << "Outputting normal CNF, without XOR-clause addition of CryptoMiniSat" << endl;
        SolverAttrib::use_xor_clauses = false;
    }
    
    if (vm.count("seed")) {
        cpd.mtrand.seed((long unsigned)vm["seed"].as<int>());
        cout << "Giving seed " << vm["seed"].as<int>() << endl;
    } else {
        cpd.mtrand.seed(0UL);
        cout << "Not giving explicit seed. Seed is default 0" << endl;
    }

    if (vm.count("cnfDir")) {
        cpd.cnfDir = vm["cnfDir"].as<string>();
        cout << "Putting generated files into " << vm["cnfDir"].as<string>() << endl;
    } else {
        cout << "Explicit directory name not given for CNF files. Using default " << cpd.cnfDir << endl;
    }
    
    bool speed_test = true;
    
    if (vm.count("debug")) {
        if (genDeterBits) {
            cout << "When generating the best deterministic help bit, debug mode cannot be turned on!" << endl;
            return -1;
        }
        cout << "Using debug mode: all problems will be Satisfiable, as the given help bits will all be correct" << endl;
        speed_test = false;
    } else {
        cout << "Giving randomly set help bits: unless the number of given help bits is very low, expect the generated problem(s) to be UNSAT" << endl;
    }

    if (vm.count("verbose")) {
        cpd.verbose = 1;
    }
    std::cout << "Verbosity set to " << std::boolalpha << cpd.verbose << endl;
    
    uint howmany = 1;
    
    if (vm.count("num")) {
        howmany =  vm["num"].as<int>();
    }
    cout << "Generating " << howmany << " problem instances" << endl;
    
    
    //cpd.set_fixed_state_init(556); // select distinct clause from learnts, run_set where run_set.ID = learnts.run_set_id and output = "e282674b82af4e" order by length(clause) limit 20;

    try {
        ExtendedMonomial::init_temps();
        Polynomial::init_temps();
        EquationsToSat::init_temps();
        
        if (genDeterBits)  {
            GrainOfSalt grain(0, true);
            for (uint i = 0; i < bits_of_help; i++) {
                grain.best_bit_test(10);
            }
        } else {
            GrainOfSalt grain(bits_of_help, use_deterministic_help);
            
            if (speed_test) {
                grain.speed_test(howmany);
            } else {
                grain.debug(howmany);
            }
            
            std::cout << "Outputted " << howmany << " CNF file(s) to " << cpd.cnfDir << endl;
        }
        
            
    } catch (string s) {
        cout << "Error:" << s << endl;
        return 1;
    } catch (const char* s) {
        cout << "Error:" << s << endl;
        return 1;
    }

    return 0;
}
