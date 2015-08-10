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

#include <boost/foreach.hpp>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <set>

#include "equationstosat.h"
#include "defines.h"
#include "timer.h"
#include "cipherdesc.h"

using boost::lexical_cast;
using std::cout;
using std::endl;
using std::make_pair;
using std::stringstream;
using std::set;

//#define DEBUG_SIMPLIFY
//#define DEBUG

__thread list<ExtendedMonomial>* EquationsToSat::monoList;

extern "C"
{
    extern void minimise_karnaugh(int no_inputs, int no_outputs, int** input,
                                      int** output, int* no_lines, bool onlymerge);
}

/*std::ostream& operator << (std::ostream& os, const vec<Lit>& v)
{
    for (int i = 0; i < v.size(); i++) {
        if (v[i].sign()) os << "-";
            os << v[i].var()+1 << " ";
        }
        
        return os;
}*/

std::ostream& operator << (std::ostream& os, const vector<Lit>& v)
{
    for (uint i = 0; i < v.size(); i++) {
        if (v[i].sign()) os << "-";
            os << v[i].var()+1 << " ";
    }
    
    return os;
}

//---------------------
//Static init in multi-thread
//---------------------

void EquationsToSat::init_temps()
{
    monoList = new list<ExtendedMonomial>;
}

void EquationsToSat::delete_temps()
{
    delete monoList;
}

//---------------------
//Constructors
//---------------------

EquationsToSat::EquationsToSat(const list<Equation>& _equations, SolverAttrib& _solverAttrib) :
        simplified_out_monos(0)
        , equations(_equations)
        , solverAttrib(_solverAttrib)
        , clause_group(1)
        , karn_size((0x1UL) << cpd.max_karnaugh_table)
{
    input = new int*[karn_size];
    for (uint i = 0; i < karn_size; i++)
        input[i] = new int[cpd.max_karnaugh_table];

    output = new int*[karn_size];
    for (uint i = 0; i < karn_size; i++)
        output[i] = new int[3];
}

EquationsToSat::~EquationsToSat()
{
    for (uint i = 0; i < karn_size; i++)
        delete[] input[i];
    delete[] input;

    for (uint i = 0; i < karn_size; i++)
        delete[] output[i];
    delete[] output;
}

//---------------------
//Main functions
//---------------------

uint EquationsToSat::insert_equality_clauses(const ExtendedMonomial& m, const string& desc)
{
    string clause_group_name;
    clause_group_name = m.get_desc();

    vector<pair<uint, bool> > vars;
    m.pushVarsInto(vars);

    
    typedef pair<uint, bool> mypair;
    uint extmono_var = cpd.vars.add_extmono(m);
    BOOST_FOREACH(mypair var, vars) {
        vector<Lit> lits;
        lits.push_back(Lit(extmono_var, true));
        lits.push_back(Lit(var.first, var.second));
        solverAttrib.addClause(lits, clause_group, clause_group_name);
    }

    vector<Lit> lits;
    lits.push_back(Lit(extmono_var, false));
    BOOST_FOREACH(mypair var, vars)
        lits.push_back(Lit(var.first, !var.second));
    solverAttrib.addClause(lits, clause_group, clause_group_name);

    clause_group++;
    return extmono_var;
}

uint EquationsToSat::add_as_xor(const LeftHandSide& lhs, const Polynomial& p, const string& desc)
{
    uint difficulty = 0;
    monoList->clear();
    simplify(*monoList, p.get_monos());

    difficulty += p.size()*7;
    
    vector<Lit> lits;
    BOOST_FOREACH(const ExtendedMonomial& m, *monoList) {
        Lit newlit;
        if (m.deg() == 1) {
            pair<uint, bool> var = m.getSingleVar();
            newlit = Lit(var.first, var.second);
        } else {
            if (!cpd.vars.extmono_exists(m)) {
                newlit = Lit(insert_equality_clauses(m, desc), false);
            } else
                newlit = Lit(cpd.vars.get_extmono_var(m), false);
        }

        lits.push_back(newlit);
        difficulty += 2*m.deg() + m.deg();
    }

    bool invert = !p.is_inverted();
    if (lhs.what_type == LeftHandSide::is_bool) invert ^= lhs.value;
    else
        lits.push_back(Lit(lhs.value, false));

    if (lits.size() == 0 && invert)
        return difficulty;
    
    if (invert) lits[0] = ~lits[0];

    solverAttrib.addXorClause(lits, clause_group, desc);
    
    clause_group++;
    
    return difficulty;
}

void EquationsToSat::convert_to_karnaugh(const Polynomial& p)
{
    p.get_all_vars(karnaugh_table_vars);
    mpz_t test;
    mpz_init2(test, karnaugh_table_vars.size());

    no_lines[0] = 0;
    no_lines[1] = 0;
    no_lines[2] = 0;
    if (cpd.max_karnaugh_table < karnaugh_table_vars.size()) {
        stringstream ss;
        ss << "poly: " << p << " (vars in poly:";
        copy(karnaugh_table_vars.begin(), karnaugh_table_vars.end(),
             std::ostream_iterator<uint>(ss, ","));
        ss << ")" << endl;
        throw("max_var in equationstosat.cpp is too small! In:" + ss.str());
    }
    for (uint setting = 0; setting < ((uint)0x1 << karnaugh_table_vars.size()); setting++) {
        mpz_set_ui(test, 0);
        for (uint i = 0; i < karnaugh_table_vars.size(); i++) {
            uint val = (setting >> i) & 0x1;
            input[no_lines[1]][i] = val;
            if (val) mpz_setbit(test, karnaugh_table_vars[i]);
        }
        bool out = p.evaluate(test);
        if (out) output[no_lines[1]][0] = 1, no_lines[1]++;
    }
    mpz_clear(test);
#ifdef DEBUG
    print_karnaugh_table();
#endif
}

uint EquationsToSat::add_karnaugh_table(const LeftHandSide& lhs, const string& desc)
{
    //if (output_satfile)
    //    satfile << "c Karnaugh representation\n";

    uint difficulty = 0;
    
    Lit leftvar;
    if (lhs.what_type == LeftHandSide::is_var)
        leftvar = Lit(lhs.value, false);

    if (lhs.what_type == LeftHandSide::is_var || (lhs.what_type == LeftHandSide::is_bool  && !lhs.value))
        for (int i = 0; i < no_lines[1]; i++) {
            assert(output[i][0] == 1);
            vector<Lit> lits;
            if (lhs.what_type == LeftHandSide::is_var) lits.push_back(leftvar);

            for (uint i2 = 0; i2 < karnaugh_table_vars.size(); i2++)
                if (input[i][i2] != 2) lits.push_back(Lit(karnaugh_table_vars[i2], input[i][i2]));
            
            solverAttrib.addClause(lits, clause_group, desc);
            difficulty += lits.size();
        }

    if (lhs.what_type == LeftHandSide::is_var || (lhs.what_type == LeftHandSide::is_bool && lhs.value))
        for (int i = no_lines[1] ; i < no_lines[1]+no_lines[0]; i++) {
            assert(output[i][0] == 0);
            vector<Lit> lits;
            if (lhs.what_type == LeftHandSide::is_var) lits.push_back(~leftvar);

            for (uint i2 = 0; i2 < karnaugh_table_vars.size(); i2++)
                if (input[i][i2] != 2) lits.push_back(Lit(karnaugh_table_vars[i2], input[i][i2]));
            
            solverAttrib.addClause(lits, clause_group, desc.c_str());
            difficulty += lits.size();
        }

    clause_group++;
    
    return difficulty;
}

uint EquationsToSat::convert()
{
    double   cpu_time = cpuTime();
    if (cpd.verbose) cout << "Converting to SAT..." << std::flush;
    
    uint difficulty = 0;

    for (list<Equation>::const_iterator it = equations.begin(); it != equations.end(); it++) if (!it->tautology()) {
#ifdef DEBUG
        cout << "Polynomial:" << *it << endl;
#endif

        LeftHandSide::type type = (it->left.is_bool()) ? LeftHandSide::is_bool : LeftHandSide::is_var;
        uint vb;
        if (type == LeftHandSide::is_var)
            vb = it->left.get_mono().getSingleVar();
        else
            vb = it->left.get_bool();
        
        LeftHandSide lhs(type, vb);

        uint num_vars = it->right.get_num_vars();
        if (num_vars <= cpd.max_karnaugh_table && !it->right.simpleXOR()) {
            convert_to_karnaugh(it->right);
            minimise_karnaugh(karnaugh_table_vars.size(), 1, input, output, no_lines, false);
#ifdef DEBUG
            print_karnaugh_table();
#endif
            difficulty += add_karnaugh_table(lhs, it->type.get_desc());
        } else  {
            difficulty += add_as_xor(lhs, it->right, it->type.get_desc());
        }
    }
    
    if (cpd.verbose) {
        cout << "Done (simplified monos: " << simplified_out_monos << ", time:" <<  cpuTime() - cpu_time << "s)" << endl;
    }
    
    return difficulty;
}

//---------------------
//Misc functions
//---------------------

void EquationsToSat::print_karnaugh_table() const
{
    for (int i = 0; i < no_lines[0] + no_lines[1]; i++) {
        for (uint i2 = 0; i2 < karnaugh_table_vars.size(); i2++) {
            cout << input[i][i2] << endl;
        }
        cout << " " << output[i][0]  << endl;
    }
    printf("--------------\n");
}

void EquationsToSat::simplify(list<ExtendedMonomial>& ret, const list<Monomial>& data)
{
    assert(ret.empty());
    if (data.empty()) return;

#ifdef DEBUG_SIMPLIFY
    cout << "EquationsToSat::simplify(list<Monomial>& data)" << endl;
    cout << "Original poly:\n" << data << endl;
#endif
    BOOST_FOREACH(const Monomial& m, data)
    ret.push_back(m);
    
    if (cpd.noExtendedMonomial) return;

    for (list<ExtendedMonomial>::iterator it = ret.begin(); it != ret.end();) {
        list<ExtendedMonomial>::iterator tmp = find_one_larger_containing_this(it, ret);
        if (tmp != ret.end()) {
            uint to_negate = ExtendedMonomial::getSingleVarDiff(*it, *tmp);
#ifdef DEBUG_SIMPLIFY
            cout << "-->removing " << *it << " using " << *tmp << ". Negating var " << to_negate << endl;
#endif
            simplified_out_monos++;
            tmp->negate(to_negate);
            it = ret.erase(it);
        } else {
            it++;
        }
    }

#ifdef DEBUG_SIMPLIFY
    cout << "Final poly:\n" << ret << endl << "---------" << endl;
#endif
}

list<ExtendedMonomial>::iterator EquationsToSat::find_one_larger_containing_this(const list<ExtendedMonomial>::iterator& what, list<ExtendedMonomial>& find_in)
{
    ExtendedMonomial& tofind = *what;
    uint tofind_deg = tofind.deg() + 1;
    assert(tofind_deg > 1);

    const list<ExtendedMonomial>::iterator end = find_in.end();
    for (list<ExtendedMonomial>::iterator it = what; it != end; it++) {
        if (it->contains(tofind) && it->deg() == tofind_deg)
            return it;
    }

    return end;
}
