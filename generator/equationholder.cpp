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

#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/operations.hpp>
#include "limits.h"

#include "equationholder.h"
#include "defines.h"
#include "MersenneTwister.h"
#include "timer.h"
#include "cipherdesc.h"

//#define DEBUG
//#define DIRTY_LINEARIZE_HACK

using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using boost::lexical_cast;

EquationHolder::EquationHolder()
{}

EquationHolder::~EquationHolder()
{
    BOOST_FOREACH(Equation& e, equations)
        delete &(e.right);
}

lbool EquationHolder::eliminate_trivial_and_mono_equations()
{
    bool changed_something = true;

    uint runs = 0;
    while (changed_something == true) {
        runs++;
        changed_something = false;
        if (!eliminate_trivial_equations(changed_something)) return l_False;
        eliminate_mono_equations(changed_something);
    }

#ifdef DEBUG
    if (runs > 1) cout << "multiple cycles of eliminate_trivial_equations() and eliminate_mono_equations() were needed" << endl;
#endif

    return l_Undef;
}

bool EquationHolder::eliminate_trivial_equations(bool& changed_something)
{
    uint num_replaced = 0;
    double cpu_time = cpuTime();
    if (cpd.verbose) cout << "Eliminating trivial equations..." << flush;
    for (list<Equation>::iterator it = equations.begin(); it != equations.end();) {
        if (!it->equation_is_trivial_and_has_been_replaced && it->trivial_equation()) {
            it->equation_is_trivial_and_has_been_replaced = true;
            it = eliminate(it, DataElementRange(any_type, UINT_MAX, UINT_MAX, UINT_MAX), num_replaced, false);
            changed_something = true;
        } else
            it ++;
    }

    //check for equations that are like "bool=bool". Remove if tautology, return false if impossible
    for (list<Equation>::iterator it = equations.begin(); it != equations.end(); ) {
        if (it->left.is_bool() && it->right.empty()) {
            if (it->left.get_bool() != it->right.is_inverted())
                return false;
            it = free_and_erase(it);
        } else it++;
    }

    if (cpd.verbose)
        cout << "Done (time: " << cpuTime() - cpu_time << "s)" << endl;

    return true;
}

void EquationHolder::eliminate_mono_equations(bool& changed_something)
{
    uint num_replaced = 0;
    double cpu_time = cpuTime();
    if (cpd.verbose) cout << "Eliminating mono equations..." << flush;
    for (list<Equation>::iterator it = equations.begin(); it != equations.end();) {
        if (it->left.is_mono() && it->right.size() == 1) {
            if (it->left.get_mono().deg() == 1 && it->right.get_monos().begin()->deg()==1) {
                //cout << it->left.get_mono() << " == " << *it->right.get_monos().begin() << endl;
                const uint left_var = it->left.get_mono().getSingleVar();
                const uint right_var = it->right.get_monos().begin()->getSingleVar();
                
                typedef pair<const uint, uint> mypair;
                BOOST_FOREACH(mypair& same, same_vars) {
                    if (same.second == left_var)
                        same.second = right_var;
                }
                
                same_vars[left_var] = right_var;
            }
            it = eliminate(it, DataElementRange(any_type, UINT_MAX, UINT_MAX, UINT_MAX), num_replaced);
            changed_something = true;
        } else
            it ++;
    }
    if (cpd.verbose)
        cout << "Done (replaced: " << num_replaced << " time: " << cpuTime() - cpu_time << "s)" << endl;
}

void EquationHolder::linearize_linearizable()
{
    double cpu_time = cpuTime();
    if (cpd.verbose) cout << "Linearizing linearizable SR(s): " << flush;
    bool old_verbose = cpd.verbose;
    cpd.verbose = false;
    BOOST_FOREACH(uint sr, cpd.linearizable_sr_during_init) {
        if (old_verbose) cout << "SR" << sr << "(init).." << flush;

        DataElementRange range(sr_type, sr, 0, cpd.init_clock);
        eliminateBetween(range, range, false);
    }

    BOOST_FOREACH(uint sr, cpd.linearizable_sr_during_norm) {
        if (old_verbose) cout << "SR" << sr << "(norm).." << flush;

        DataElementRange range(sr_type, sr, cpd.init_clock, cpd.init_clock + cpd.sr_size[sr] + cpd.outputs);
        eliminateBetween(range, range, false);
    }
    cpd.verbose = old_verbose;

    if (cpd.verbose)
        cout << "Done (time: " << cpuTime() - cpu_time << "s)" << endl;
}

void EquationHolder::simplify_equations()
{
    double   cpu_time = cpuTime();
    if (cpd.verbose) cout << "Simplifying... " << flush;

    /*for (uint filter = 0; filter < cpd.filter_num; filter++) {
        DataElementRange range(filter_type, filter, 0, cpd.init_clock + cpd.outputs);
        eliminateBetween(range, DataElementRange(any_type, UINT_MAX, 0, UINT_MAX), true);
    }*/
    
    
    //eliminateBetween(filter_type, 1, any_type, UINT_MAX, 0, cpd.init_clock + cpd.outputs);
    //eliminateBetween(filter_type, 2, any_type, UINT_MAX, 0, cpd.init_clock + cpd.outputs);

    //eliminateBetween(init_sr_type, 0, , any_type, UINT_MAX, 0, cpd.init_clock);
    //eliminateBetween(init_sr_type, 1, any_type, UINT_MAX, 0, cpd.init_clock);
    //eliminateBetween(norm_sr_type, 0, any_type, UINT_MAX, cpd.init_clock, cpd.init_clock +cpd.sr_size[0] + cpd.outputs);
    //eliminateBetween(norm_sr_type, 1, any_type, UINT_MAX, cpd.init_clock, cpd.init_clock +cpd.sr_size[1] + cpd.outputs);

    if (cpd.verbose) cout << "Done (time:" << cpuTime() - cpu_time  << "s)" << endl;
}

void EquationHolder::eliminateBetween(const DataElementRange& type, const DataElementRange& eliminate_type, const bool delete_eq)
{
    assert(type.type != any_type);

    print_stats(0);

    if (cpd.verbose)
        cout << "Eliminating ALL between states " << type.var_from << " and " <<  type.var_to << endl;
    for (uint i = type.var_from ; i < type.var_to; i++) {
        if (cpd.verbose)
            cout << i-type.var_from <<"/" << type.var_to-type.var_from << " - elim " << i << ":";

        list<Equation>::iterator it = findEquationByState(DataElement(type.type, type.which_of_type, i));
        if (it == equations.end()) {
            if (cpd.verbose) cout << "None found" << endl;
        } else {
            uint num_replaced = 0;
            eliminate(it, eliminate_type, num_replaced, delete_eq);
            print_stats(num_replaced);
        }
    }
}

void EquationHolder::eliminateBetweenSmallestX(const DataElementRange& type, const DataElementRange& eliminate_type, const uint howmany, const bool delete_eq)
{
    print_stats(0);

    if (cpd.verbose)
        cout << "Eliminating " << howmany << " between " << type.var_from << " and " << type.var_to << endl;
    for (uint i = 0 ; i < howmany; i++) {
        pair<list<Equation>::iterator, unsigned long int> smallest(equations.begin(), ULONG_MAX);

        for (list<Equation>::iterator it = equations.begin(); it != equations.end(); it++) {
            if (!type.contains(it->type)) continue;
            if (it->trivial_equation()) continue;
            if (it->right.size() < smallest.second) {
                smallest.first = it;
                smallest.second = it->right.size();
            }
        }
        if (smallest.second == ULONG_MAX) {
            if (cpd.verbose)
                cout << "Could only remove " << i << ", there is simply no more!" << endl;
            return;
        }

        if (cpd.verbose)
            cout << i <<"/"  << howmany << " - elim " << smallest.first->left.get_mono().getSingleVar() << ":";
        uint num_replaced = 0;
        eliminate(smallest.first, eliminate_type, num_replaced, delete_eq);
        print_stats(num_replaced);
    }
}

list<Equation>::iterator EquationHolder::findEquationByVar(const uint var)
{
    list<Equation>::iterator it;
    for ( it = equations.begin(); it != equations.end(); it++) {
        if (it->left.is_mono() && it->left.get_mono().deg() == 1 && it->left.get_mono().getSingleVar() == var) break;
    }

    return it;
}

list<Equation>::iterator EquationHolder::findEquationByState(const DataElement& type)
{
    list<Equation>::iterator it;
    for ( it = equations.begin(); it != equations.end(); it++)
        if (it->type.matches(type)) break;

    return it;
}

list<Equation>::iterator EquationHolder::eliminate(list<Equation>::iterator it, const DataElementRange& eliminate_type, uint& num_replaced, const bool delete_eq)
{
    if (it->tautology()) return free_and_erase(it);

    for (list<Equation>::iterator it2 = equations.begin(); it2 != equations.end(); it2++) {
        if (it == it2) continue;
        if (!eliminate_type.contains(it2->type)) continue;

        uint this_num_replaced = it2->replace(*it);
        //if (this_num_replaced > 0) cout << "replacing with " << endl << *it << " in " << endl << *it2 << endl;
        num_replaced += this_num_replaced;
    }

    if (!delete_eq) return ++it;
    else return free_and_erase(it);
}

void EquationHolder::remove_unneeded()
{
    if (cpd.verbose) cout << "Removing unneccessary equations.." << flush;
    mpz_t depend;
    mpz_init(depend);
    mpz_t tmp;
    mpz_init(tmp);
    BOOST_FOREACH(const Equation& eq, equations) {
        if (eq.type.type == output_type) {
            eq.right.get_all_vars(tmp);
            mpz_ior(depend, depend, tmp);
        }
    }

    mpz_t checked_depend;
    mpz_init(checked_depend);
    mpz_t inv_checked_depend;
    mpz_init(inv_checked_depend);
    bool something_modified = true;
    while (something_modified) {
        something_modified = false;
        const list<Equation>::iterator end = equations.end();
        for (list<Equation>::iterator it = equations.begin(); it != end; it++) if (it->left.is_mono()) {
                uint var = it->left.get_mono().getSingleVar();
                if (mpz_tstbit(depend, var)) {
                    it->right.get_all_vars(tmp);

                    mpz_com(inv_checked_depend, checked_depend);
                    mpz_and(tmp, tmp, inv_checked_depend);
                    mpz_ior(depend, depend, tmp);

                    mpz_setbit(checked_depend, var);
                    mpz_clrbit(depend, var);
                    something_modified = true;
                }
            }
    }
    mpz_clear(inv_checked_depend);
    mpz_clear(tmp);
    mpz_clear(depend);

    uint unneccesary = 0;
    uint removed = 0;
    const list<Equation>::const_iterator end = equations.end();
    for (list<Equation>::iterator it = equations.begin(); it != end; )
        if (it->type.type != output_type
                && it->left.is_mono()
                && !mpz_tstbit(checked_depend, it->left.get_mono().getSingleVar())
           ) {
            unneccesary++;
#ifdef DEBUG
            cout << "unneccessary: "  << *it << endl;
#endif
            if (!it->trivial_equation()) it = free_and_erase(it), removed++;
            else {
                it++;
#ifdef DEBUG
                cout << "not removing" << endl;
#endif
            }
        } else it++;
    mpz_clear(checked_depend);

    if (cpd.verbose)
        cout << "found " << unneccesary << ", only removed " << removed << "(rest was trivial eq)"<< endl;
}

//------------------------------
// Printing functions
//------------------------------

void EquationHolder::print_equations_set(const DataElement& type, const string& name) const
{
    ofstream polyfile, distribfile;

    bs::path filename = get_stats_dir() / name;
    polyfile.open(filename.native_file_string().c_str());
    if (polyfile.fail())
        throw "cannot open file " + filename.native_file_string() + " for writing!";

    bs::path filename_dist = get_stats_dir() / (name + "-dist");
    distribfile.open(filename_dist.native_file_string().c_str());

    BOOST_FOREACH(const Equation& eq, equations) {
        if (type.type != any_type && eq.type.type != type.type) continue;
        if (type.which_of_type != UINT_MAX && eq.type.which_of_type != (uint)type.which_of_type) continue;

        polyfile << eq << endl << endl;

        distribfile << eq.type.index << "\t" << eq.right.size() << endl;
    }

    polyfile.close();
    distribfile.close();
}

void EquationHolder::print_equations() const
{
    if ( !bs::exists(get_stats_dir()) ) {
        cout << "The statistics directory '" << get_stats_dir().native_file_string() << "' does not exist." << endl
        << "Please create it so that I can write statistics there!" << endl;
        exit(-1);
    }
    
    for (uint i = 0; i < cpd.sr_num; i++) {
        print_equations_set(DataElement(sr_type, i, UINT_MAX)
                            , "init_sr" + lexical_cast<string>(i));
    }
    for (uint i = 0; i < cpd.sr_num; i++) {
        print_equations_set(DataElement(sr_type, i, UINT_MAX)
                            , "sr" + lexical_cast<string>(i));
    }

    print_equations_set(DataElement(output_type, UINT_MAX, UINT_MAX), "out");

    for (uint i = 0; i < cpd.filter_num; i++) {
        print_equations_set(DataElement(filter_type, i, UINT_MAX)
                            , "f" + lexical_cast<string>(i));
    }

    print_equations_set(DataElement(any_type, UINT_MAX, UINT_MAX), "all");

    ofstream monomial_size_distrib_file;
    bs::path filename = get_stats_dir() / "monos-dist";
    monomial_size_distrib_file.open(filename.native_file_string().c_str());
    if (monomial_size_distrib_file.fail())
        throw "cannot open file " + filename.native_file_string() + " for writing!";

    vector<uint> monomial_size_distrib;
    BOOST_FOREACH(const Equation& eq, equations)
    eq.right.add_monomial_distribution(monomial_size_distrib);

    for (uint i = 0; i < monomial_size_distrib.size(); i++) {
        if (monomial_size_distrib[i] > 0)
            monomial_size_distrib_file << i << "\t" << monomial_size_distrib[i] << endl;
    }

    monomial_size_distrib_file.close();
}

void EquationHolder::print_stats(const uint num_replaced) const
{
    if (!cpd.verbose) return;

    uint sum_monos = 0;
    uint sum_mono_deg = 0;
    BOOST_FOREACH(const Equation& eq, equations) {
        sum_monos += eq.right.size();
        sum_mono_deg += eq.right.sum_mono_deg();
    }
    cout << "replaced: " << num_replaced << ", num equations left: " << equations.size() << ", sum monos:" << sum_monos << ", sum monos deg:" << sum_mono_deg << ", avg num monos: " << (double)sum_monos/(double)equations.size() << ", avg mono size: " << (double)sum_mono_deg/(double)sum_monos << endl;

    cout << "Double monomials cleaned:" << Polynomial::get_num_deleted_monomials() << ",";
    cout << "their sum deg:"<< Polynomial::get_num_deleted_monomials_deg() <<", ";
    cout << "their avg deg:" << (double)Polynomial::get_num_deleted_monomials_deg()/
         (double) Polynomial::get_num_deleted_monomials() << endl;
}

//------------------------------
// Miscellaneous functions
//------------------------------

const list<Equation>& EquationHolder::get_equations() const
{
    return equations;
}

void EquationHolder::add_eq(const DataElement& type, const Monomial& a, Polynomial* b)
{
    assert(type.type != any_type);
    equations.push_back(Equation(type, a, *b));
}

list<Equation>::iterator EquationHolder::free_and_erase(list<Equation>::iterator& it)
{
    delete &(it->right);
    return equations.erase(it);
}

bs::path EquationHolder::get_stats_dir() const
{
    bs::path dirname( bs::initial_path<bs::path>() );
    dirname /= "stats";

    return dirname;
}

const map<uint, uint>& EquationHolder::get_same_vars() const
{
    return same_vars;
}
