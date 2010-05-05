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
#ifndef XOR_COMPUTER_H
#define XOR_COMPUTER_H

#include "SolverTypes.h"

#include <vector>
#include <list>
#include <ext/hash_set>

using std::vector;
using std::list;

/**
@brief Generates all the combinations of an n-long xor so that it can be added as a normal clause

It pre-computes a permutated sequence and then when something needs to be permutated, it simply inserts it into a pointer-list so that the permutation does not need to be done again
*/
class XOR_permutator
{
public:
    XOR_permutator(const uint max_size);
    /**
    @brief Return all possible combinations of a vector of Lit-s.
    @param pair Invert the xor or not?
    @param original the original xor-clause
    @return the 2^(n-1) clauses, in a list
    */
    list<vector<Lit> > all_combinations(const vector<Lit>& original, bool pair) const;

private:
    void doit(const uint depth, const uint num_to_negate); ///Recursive function to do the initial permutation

    vector<bool> pre_xor;
    list<vector<bool> > pre_combinations;
    vector<list<vector<bool> > > pre_compute_pair;
    vector<list<vector<bool> > > pre_compute_impair;
    vector<bool> tmp;
};

#endif
