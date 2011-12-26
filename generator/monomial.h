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
#ifndef MONOMIAL_H
#define MONOMIAL_H

#include <bits/stl_pair.h>
#include "gmp.h"
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <list>
#include <sstream>

using std::list;
using std::vector;
using std::pair;
using std::ostream;
using std::string;

class MonoStack;

class Monomial
{
public:
    //Initalisation of static variables in multi-thread environment
    static void init_temps();
    static void delete_temps();

    //Constructors
    Monomial();
    Monomial(const Monomial& m);
    ~Monomial();
    Monomial(const uint var, const bool inverted);
    Monomial& operator=(const Monomial& b);

    //Setting opeartors
    void swap(Monomial& m);
    void setmult(const Monomial& m1, const Monomial& m2);
    Monomial& operator*=(const Monomial& b);
    Monomial operator*(const Monomial& b) const;
    Monomial& operator*=(const pair<uint, bool>& mypair);

    //Comparison operators
    bool operator==(const Monomial& m) const;
    bool operator!=(const Monomial& m) const;
    bool operator<(const Monomial& m) const;
    bool operator<=(const Monomial& m) const;

    //misc functions
    bool contains(const uint, const bool) const;
    inline bool contains(const uint var) const;
    bool contains(const Monomial& m) const;
    bool clashesWith(const Monomial& clashesWith) const;
    static uint getSingleVarDiff(const Monomial& smaller, const Monomial& larger);

    //small misc functions
    bool empty() const;
    uint getSingleVar() const;
    uint deg() const;
    bool impossible() const;
    void pushVarsInto(vector<pair<uint, bool> >& topush) const;
    bool evaluate(mpz_t& set) const;
    const mpz_t& get_all_vars() const;
    string get_desc() const;

    //misc functions that touch the monomial
    void remove(const Monomial& m);
    inline void remove(const uint var);

    friend class ExtendedMonomial;
    friend ostream& operator << (ostream& os, const Monomial& s);
protected:
    static bool mycmp(const Monomial& a, const Monomial& b, const bool equal_or_less);

    mpz_t& vars;

    static __thread mpz_t tmp;
    static __thread MonoStack* monoStack;
};

extern ostream& operator << (ostream& os, const Monomial& s);
extern ostream& operator << (ostream& os, const list<Monomial>& monos);

inline bool Monomial::contains(const uint var) const
{
    return mpz_tstbit(vars, var);
}

inline void Monomial::remove(const uint var)
{
    mpz_clrbit(vars, var);
}

inline bool Monomial::evaluate(mpz_t& set) const
{
    mpz_and(tmp, set, vars);
    return !(mpz_cmp(tmp, vars));
}

#endif
