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
#include "equation.h"

#include "string.h"
#include <boost/foreach.hpp>

//#define DEBUG

using std::cout;
using std::endl;

Equation::Equation(const DataElement& _type, const Monomial& a, Polynomial& b):
        type(_type)
        , left(a)
        , right(b)
        , equation_is_trivial_and_has_been_replaced(false)
{
}

uint Equation::replace(const Equation& replace_with)
{
    assert(replace_with.left.is_mono());

    uint num_replaced = right.replace(replace_with.left.get_mono(), replace_with.right);

    if ((left.is_mono() && left.get_mono().contains(replace_with.left.get_mono()))) {
        if (replace_with.right.size() == 1 && replace_with.left.get_mono() == left.get_mono()) {
            left =  *replace_with.right.get_monos().begin();
        } else if (replace_with.right.empty() && replace_with.left.get_mono() == left.get_mono()) {
            left =  replace_with.right.is_inverted();
        } else {
            cout << "Replacing with:" << endl << replace_with  << endl << "..but the equation is:" << endl << *this << endl;
            throw("Replacing varible with an equation, but that variable is present in another equation's left-hand side");
        }
    }

    return num_replaced;
}

bool Equation::trivial_equation() const
{
    uint right_size = right.size();
    if (left.is_mono()
            && (right_size == 0 ||
                (right.size() == 1 && right.get_monos().begin()->deg() == 1)
               )
       ) return true;
    return false;
}

bool Equation::tautology() const
{
    if (left.is_mono() && right.size() == 1 && left.get_mono() ==  *right.get_monos().begin() ) return true;
    return false;
}

ostream& operator << (ostream& os, const Equation& e)
{
    return os << e.type << endl << e.left << "=" << e.right;
}

ostream& operator << (ostream& os, const Mono_or_Bool& mb)
{
    if (mb.is_mono()) os << mb.get_mono();
    else os << mb.get_bool();
    return os;
}


