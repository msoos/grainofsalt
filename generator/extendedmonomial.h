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
#ifndef EXTENDEDMONOMIAL_H
#define EXTENDEDMONOMIAL_H

#include <bits/stl_pair.h>
#include "gmp.h"
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <string>

#include "monomial.h"

using std::string;

class MonoStack;

class ExtendedMonomial : private Monomial
{
public:
    //Initalisation of static variables in multi-thread environment
    static void init_temps();
    static void delete_temps();

    //Constructors
    ExtendedMonomial();
    ExtendedMonomial(const ExtendedMonomial& m);
    ExtendedMonomial(const Monomial& m);
    ~ExtendedMonomial();
    ExtendedMonomial(const uint var, const bool inverted);
    ExtendedMonomial& operator=(const ExtendedMonomial& b);

    //Setting opeartors
    void swap(ExtendedMonomial& m);
    void setmult(const ExtendedMonomial& m1, const ExtendedMonomial& m2);
    ExtendedMonomial& operator*=(const ExtendedMonomial& b);
    ExtendedMonomial operator*(const ExtendedMonomial& b) const;
    ExtendedMonomial& operator*=(const pair<uint, bool>& mypair);

    //Comparison operators
    bool operator==(const ExtendedMonomial& m) const;
    bool operator!=(const ExtendedMonomial& m) const;
    bool operator<(const ExtendedMonomial& m) const;
    bool operator<=(const ExtendedMonomial& m) const;

    //misc functions
    bool contains(const uint, const bool) const;
    inline bool contains(const uint var) const;
    bool contains(const ExtendedMonomial& m) const;
    bool clashesWith(const ExtendedMonomial& clashesWith) const;
    static uint getSingleVarDiff(const ExtendedMonomial& smaller, const ExtendedMonomial& larger);

    //small misc functions
    bool empty() const;
    pair<uint, bool> getSingleVar() const;
    uint deg() const;
    bool impossible() const;
    void pushVarsInto(vector<pair<uint, bool> >& topush) const;
    bool evaluate(mpz_t& set) const;
    const mpz_t& get_all_vars() const;
    string get_desc() const;

    //misc functions that touch the monomial
    void remove(const ExtendedMonomial& m);
    void remove(const uint var);
    void negate(const uint var);

    friend ostream& operator << (ostream& os, const ExtendedMonomial& s);
protected:
    static bool mycmp(const ExtendedMonomial& a, const ExtendedMonomial& b, const bool equal_or_less);

    mpz_t& positive_vars;
    mpz_t& inverted_vars;

    static __thread mpz_t tmp2;
};

extern ostream& operator << (ostream& os, const ExtendedMonomial& s);
extern ostream& operator << (ostream& os, const list<ExtendedMonomial>& monos);

#endif
