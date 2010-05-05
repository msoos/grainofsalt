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

#include "xor_computer.h"

#include <boost/foreach.hpp>
#include "assert.h"

using std::vector;

XOR_permutator::XOR_permutator(const uint max_size)
{
    pre_compute_pair.resize(max_size + 1);
    pre_compute_impair.resize(max_size + 1);

    for (uint size = max_size; size > 0; size--) {
        pre_xor.resize(size);
        fill(pre_xor.begin(), pre_xor.end(), false);
        tmp.resize(size);
        for (uint i2 = 0; i2 <= size; i2++) {
            pre_combinations.clear();
            doit(0, i2);
            if (i2 % 2 == 0) pre_compute_pair[size].splice(pre_compute_pair[size].begin(), pre_combinations);
            else pre_compute_impair[size].splice(pre_compute_impair[size].begin(), pre_combinations);
        }
    }
}

void XOR_permutator::doit(const uint depth, const uint num_to_negate)
{
    //if at the very end, add it to the combinations
    assert(pre_xor.size() == tmp.size());
    if (depth == pre_xor.size()) {
        if (num_to_negate == 0) pre_combinations.push_back(tmp);
        return;
    }
    //else if it is at all possible to negate that many, try both
    else if (pre_xor.size() - depth >= num_to_negate) {
        //try both leafs of the tree if there is something to negate
        tmp[depth] = pre_xor[depth];
        doit(depth + 1, num_to_negate);
        if (num_to_negate > 0) {
            tmp[depth] = !tmp[depth];
            doit(depth + 1, num_to_negate - 1);
        }
    }
}

list<vector<Lit> > XOR_permutator::all_combinations(const vector<Lit>& original, bool pair) const
{
    assert(original.size() <= pre_compute_pair.size() - 1);

    list<vector<Lit> > ret_real;

    const list<vector<bool> >* which;
    if (pair) which =  &pre_compute_pair[original.size()];
    else which = &(pre_compute_impair[original.size()]);

    typedef vector<bool> bool_vec;

    BOOST_FOREACH(const bool_vec& v, *which) {
        assert(v.size() == original.size());
        ret_real.push_back(original);

        vector<bool>::const_iterator bool_it;
        vector<Lit>::iterator lit_it;
        for (bool_it = v.begin(), lit_it = ret_real.rbegin()->begin(); bool_it != v.end(); bool_it++, lit_it++)
            *lit_it ^= *bool_it;
    }

    return ret_real;
}


