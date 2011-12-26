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
#ifndef EQUATION_H
#define EQUATION_H

#include <string>

#include "polynomial.h"
#include "dataelement.h"

using std::string;

class Mono_or_Bool
{
private:
    enum {mono_type, bool_type} mono_or_bool;
    bool b;
    Monomial mono;
public:
    Mono_or_Bool(const bool _b):
            mono_or_bool(bool_type)
            , b(_b) {}

    Mono_or_Bool(const Monomial& _mono):
            mono_or_bool(mono_type)
            , mono(_mono) {}

    const Monomial& get_mono() const {
        assert(mono_or_bool == mono_type);
        return mono;
    }

    Monomial& get_mono() {
        assert(mono_or_bool == mono_type);
        return mono;
    }

    bool& get_bool() {
        assert(mono_or_bool == bool_type);
        return b;
    }

    const bool& get_bool() const {
        assert(mono_or_bool == bool_type);
        return b;
    }

    const bool is_mono() const {
        return mono_or_bool == mono_type;
    }

    const bool is_bool() const {
        return mono_or_bool == bool_type;
    }
};

extern ostream& operator << (ostream& os, const Mono_or_Bool& mb);

class Equation
{
public:
    Equation(const DataElement& _type, const Monomial& a, Polynomial& b);

    uint replace(const Equation& replace_with);
    bool trivial_equation() const;
    bool tautology() const;

    DataElement type;
    Mono_or_Bool left;
    Polynomial& right;
    bool equation_is_trivial_and_has_been_replaced;

    friend ostream& operator << (ostream& os, const Equation& p);
};

extern ostream& operator << (ostream& os, const Equation& p);

#endif
