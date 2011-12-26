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
#ifndef EQUATIONHOLDER_H
#define EQUATIONHOLDER_H

#include <vector>
#include <sys/types.h>
#include <map>
#include <boost/filesystem/path.hpp>

#include "equation.h"
#include "dataelementrange.h"
#include "SolverTypes.h"

class Polynomial;
class MTRand;

namespace bs = boost::filesystem;
using std::map;

class EquationHolder
{
public:
    //Constructors
    EquationHolder();
    ~EquationHolder();

    void add_eq(const DataElement& type, const Monomial& a, Polynomial* b);

    //Equation-managing functions
    void simplify_equations();
    lbool eliminate_trivial_and_mono_equations();
    void linearize_linearizable();
    void remove_unneeded();

    //Misc functions
    void print_equations() const;
    const list<Equation>& get_equations() const;
    bs::path get_stats_dir() const;
    const map<uint, uint>& get_same_vars() const;
    

private:
    bool eliminate_trivial_equations(bool& changed_something);
    void eliminate_mono_equations(bool& changed_something);

    //functions for use during elimination
    void eliminateBetween(const DataElementRange& type, const DataElementRange& eliminate_type, const bool delete_eq = true);

    void eliminateBetweenSmallestX(const DataElementRange& type, const DataElementRange& eliminate_type, const uint howmany, const bool delete_eq = true);

    list<Equation>::iterator eliminate(list<Equation>::iterator it, const DataElementRange& eliminate_type, uint& num_replaced, const bool delete_eq = true);
    list<Equation>::iterator findEquationByState(const DataElement& type);
    list<Equation>::iterator findEquationByVar(const uint var);

    //When erasing an equation, this must be called
    list<Equation>::iterator free_and_erase(list<Equation>::iterator& it);

    //printing functions
    void print_equations_set(const DataElement& type, const string& name) const;
    void print_stats(const uint num_replaced) const;

    map<uint, uint> same_vars;
    list<Equation> equations;
};

#endif
