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
#ifndef EQUATIONSTOSAT_H
#define EQUATIONSTOSAT_H

#include "SolverTypes.h"
#include "equationholder.h"
#include "monomial.h"
#include "extendedmonomial.h"
#include "solverattrib.h"

#include <vector>
#include <bits/stl_pair.h>
#include <sys/types.h>
#include <fstream>
#include <map>
#include <list>

using std::ofstream;
using std::map;

/**
@brief Generates SAT formula from the equations

The CipherDesc will instruct it how to actually do it: XOR clauses or not? Does the satfile need to be generated? Etc.
*/
class EquationsToSat
{
public:

    static void init_temps(); ///<initalize the monoList temporary (needed for each thread)
    static void delete_temps(); ///<delete the monoList temporary (needed after each thread)

    EquationsToSat(const list<Equation>& equations, SolverAttrib& solverAttrib); ///<Reserves memory for the "input" and "output" variables, used in the Karnaugh-tables
    ~EquationsToSat(); ///<Deletes the karnaugh-table reserved memory

    uint convert(); ///<Converts the equations given in the constructor into the Solver and into the satfile. Returns difficulty

private:
    
    ///stores what is on the left-hand side of the equation
    class LeftHandSide
    {
        public:
            enum type {is_bool, is_var};
            
            LeftHandSide(const type _what_type, const uint _value) :
            what_type(_what_type)
            , value(_value) {}
            
            const type what_type; ///<boolean or a variable?
            const uint value; ///<value of the boolean/variable
    };
    
    //----------------------------------------
    // Polynomials -> XOR in SAT file
    //----------------------------------------
    const list<Equation>& equations; ///<The equations that need to be converted
    /**
    @brief Inserts a set of clauses that equate the extended monomial with the variable returned
    @param mono The monomial that needs to be inserted
    @param desc the description of the monomial (e.g. sr1[1]*sr2[3])
    @return The variable number that the extended monomial has been equated to
    */
    uint insert_equality_clauses(const ExtendedMonomial& mono, const string& desc);
    
    /**
    @brief Adds an equation as a XOR-clause
    @param lhs the left hand side of the equation
    @param p the polynomial on the right-hand side
    @param desc The description of the equation (e.g. f1[1])
    */
    uint add_as_xor(const LeftHandSide& lhs, const Polynomial& p, const string& desc); ///< Returns difficulty

    //----------------------------------------
    // Simplification of Polynomials
    //----------------------------------------
    /**
    @brief Makes a set of ExtendedMonomial-s from simple Monomial-s
    
    Extended monomials can have negated internal variables, which can reduce the size of the poly. Note that the return of this function is at most as large as the input - this function thus simplifes the polynomial
    @param[in] data The polynomial's monomials
    @param[out] ret The converted monomials
    */
    void simplify(list<ExtendedMonomial>& ret, const list<Monomial>& data);
    static list<ExtendedMonomial>::iterator find_one_larger_containing_this(const list<ExtendedMonomial>::iterator& what, list<ExtendedMonomial>& find_in); ///<A helper function to simpify()
    uint simplified_out_monos; ///<The number of monomials simplified out of the equations
    static __thread list<ExtendedMonomial>* monoList; ///per-thread data structure used by simplify() - used to avoid overhead

    //----------------------------------------
    //Polynomial-> Karnaugh-table -> SAT
    //----------------------------------------
    /**
    @brief Converts a polynomial to truth table
    
    This function only converts to a truth table. The "minimise_karnaugh"(external function) needs to be called to do the actual minimisation, and add_karnaugh_table to actually add the karnaugh table.
    @param p the polynomial to be put into the "input" variable as a truth table
    */
    void convert_to_karnaugh(const Polynomial& p);
    void print_karnaugh_table() const; ///<Prints the Karnaugh table that is stored in memory (useable after convert_to_karnaugh and minimise_karnaugh)
    /**
    @brief Adds the karnaugh table stored in "output" into the solver
    The convert_to_karnaugh() and print_karnaugh_table() functions need to be called before
    @param lhs the left-hand side of the equation
    @param desc the what the eqaution represents (e.g. f1[20])
    */
    uint add_karnaugh_table(const LeftHandSide& lhs, const string& desc); ///< Returns difficulty
    const uint karn_size; ///<The maximum size of the karnaugh table in rows
    int no_lines[3]; ///<Used for karnaugh table minimisation
    int** input; ///<The input to the karnaugh minimisation
    int** output; ///<The output of the karnaugh minimisation
    vector<uint> karnaugh_table_vars; ///<The variables that correspond to each column in the Karnaugh table

    //----------------------------------------
    //Solver-related internal vars
    //----------------------------------------
    uint clause_group; ///<The current maximal clause group number (each function has a new clause group number)
    SolverAttrib& solverAttrib;

    //----------------------------------------
    //Optimisation to encode the same ExtendedMonomial to the same variable
    //----------------------------------------
    //map<ExtendedMonomial, Var> used_monos; ///<maps an ExtendedMonomial to a variable number
    //map<Var, list<string> > var_used_in; ///<maps a variable number to the set of functions it was used in
    //uint no_reused_monos; ///<The number of reused monomials
};

#endif
