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
#ifndef CIPHERDESC_H
#define CIPHERDESC_H

#include <sys/types.h>
#include <vector>
#include <stack>
#include <string>
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/thread/thread.hpp>

#include "MersenneTwister.h"
#include "variables.h"

using namespace boost::spirit;
namespace bs = boost::filesystem;
using std::stack;
using std::vector;
using std::string;

class Monomial;
class MonoStack;

/**
@brief Parses up and stores the description and options of the cipher

It stores a lot of configuration data. An instance of this class, "cpd" is known by virtually everyone, This way, preferences can be set in advance, and then used throughout.
*/
class CipherDesc
{
public:
    CipherDesc();

    void set_name(const string& _ciphername); ///<Sets the name of the cipher description file, and reads the file
    void set_shift(uint which_sr, uint to_what); ///<Set the shifting of the cipher's shift registers
    bs::path get_cipher_path() const; ///<Returns the path of the cipher's description file

    //----------------------------------------
    //Cipher-related variables
    //----------------------------------------
    string ciphername; ///<The name of the cipher - do NOT change
    uint sr_num; ///<The number of shift registers - do NOT change
    uint filter_num; ///<The number of filter functions - do NOT change
    uint init_clock; ///<The initialisation clock number - can be changed
    uint outputs; ///<The number of outputs needed  - can be changed
    uint no_ciphers; ///<The number of ciphers (i.e. stuff that produces outputs)

    vector<uint> sr_size; ///<Size of earch shift register - do NOT change
    vector<vector<uint> > zero_out; ///<Which shift registers' which variables to zero out (only active if init_clock > 0) - do NOT change
    vector<vector<uint> > one_out; ///<Which shift registers' which variables to "1" out (only active if init_clock > 0) - do NOT change
    vector<vector<uint> > tweakable; ///<Which shift registers' which variables are tweakable (i.e. IV) (only active if init_clock > 0) - do NOT change
    vector<uint> sr_shift; ///<The shifting of the shift registers  - do NOT change
    vector<uint> linearizable_sr_during_init; ///<Which shift registers are linearizeable during initialisation? - do NOT change
    vector<uint> linearizable_sr_during_norm; ///<Which shift registers are linearizeable during normal running of the cipher? - do NOT change
    bool mixed_shifts; ///<Have the shifting been "mixed"? - do NOT change
    bool noExtendedMonomial; ///<Extended monomials used?
    bool linearizeLinearizeable; ///<Should we linearize linearizeable shift registers?

    //----------------------------------------
    //User-controllable variables
    //----------------------------------------
    bool verbose; ///<Should the program be verbose?
    uint max_karnaugh_table; ///<The maximum size (i.e. max num of variables) in the Karnaugh table
    bool use_xor_clauses; ///<Should we use xor-clauses? If not, then cutting will be done
    uint xor_cut_len; ///<If not using xor-clauses what should be the cutting lenght? (default to 7)
    bool print_stats; ///<Should statistics be printed?
    string cnfDir; ///<The directory where the generated CNFs are put into
    bool permutateClauses; ///<If set, clauses will be permutated in the outputted CNF
    bool permutateVars;///<If set, variables will be permutated in the outputted CNF
    string commandlineParams; ///<Command line parameters given

    //----------------------------------------
    //Global variables
    //----------------------------------------
    MTRand mtrand; ///<Random should be drawn from here
    Variables vars; ///<The variable ranges allocated are stored here

    //----------------------------------------
    //Functions for getting/returning thread-static vars
    //----------------------------------------
    Monomial* get_free_mixedMonos();
    void return_mixedMonos(Monomial* mixedMonos);
    MonoStack* get_free_monoStack();
    void return_monoStack(MonoStack* monoStack);
    
    //----------------------------------------
    //satate-fixing functions
    //----------------------------------------
    void set_fixed_state_init(const uint state); ///<Set a fixed "random" state. This ensures that the same problem will be generated the next time, if the same "random" state is set
    const uint get_fixed_state() const; ///<The fixed random state stored is accessible with this function. If a state has not been fixed, this will return an error.
    const uint random_state_init(MTRand& mtrand2);  ///<Called to get a random state. The state will not be random if it has been fixed before using the set_fixed_state_init

private:
    ///Parse up a vector in the config file
    static parse_info<const char*> parse_vec(const string& str, const char* what, vector<uint>& vec);
    ///Parse up a list in the config file (list = 1,3,4...)
    static parse_info<const char*> parse_list(const string& str, const char* what, vector<vector<uint> >& vec);
    ///Parse up a value in the config file
    static parse_info<const char*> parse_whatever(const string& str, char const* what, uint& equal);

    void check_within_limits(const vector<vector<uint> >& to_check, const string name) const;
    static void check_dont_clash(const vector<vector<uint> >& check1, const vector<vector<uint> >& check2);

    //----------------------------------------
    //Thread-static vars
    //----------------------------------------
    stack<MonoStack*> monoStackStack;
    stack<Monomial*> mixedMonosStack;
    boost::mutex monoStackStack_mutex, mixedMonosStack_mutex;

    //----------------------------------------
    //Fixed "random" state. This way, the output and state can be fixed -- needed for learning clauses about the same problem
    //----------------------------------------
    bool fixed_state_init; ///<Has the state been fixed?
    uint fixed_state_init_state; ///<If the state has been fixed, then to what value?
};

extern CipherDesc cpd; ///<an instance of the CipherDesc should be available from every class

#endif
