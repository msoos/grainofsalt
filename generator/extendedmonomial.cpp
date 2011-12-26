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

#include "assert.h"
#include "limits.h"
#include <sstream>

#include "extendedmonomial.h"
#include "cipherdesc.h"
#include "mystack.h"

using std::make_pair;
using std::stringstream;

//------------------------------
// Static initalizers
//------------------------------

__thread mpz_t ExtendedMonomial::tmp2;

void ExtendedMonomial::init_temps()
{
    Monomial::init_temps();
    mpz_init2(tmp2, cpd.vars.get_last_noninternal_var()+1);
}

void ExtendedMonomial::delete_temps()
{
    Monomial::delete_temps();
    mpz_clear(tmp2);
}

//------------------------------
// Constructors
//------------------------------

ExtendedMonomial::ExtendedMonomial() :
        Monomial()
        ,positive_vars(monoStack->pop())
        ,inverted_vars(monoStack->pop())
{
}

ExtendedMonomial::ExtendedMonomial(const ExtendedMonomial& m) :
        Monomial(m)
        ,positive_vars(monoStack->pop(false))
        ,inverted_vars(monoStack->pop(false))
{
    mpz_set(positive_vars, m.positive_vars);
    mpz_set(inverted_vars, m.inverted_vars);
}

ExtendedMonomial::ExtendedMonomial(const uint var, const bool inverted) :
        Monomial(var, false)
        ,positive_vars(monoStack->pop())
        ,inverted_vars(monoStack->pop())
{
    if (!inverted) mpz_setbit(positive_vars, var);
    else mpz_setbit(inverted_vars, var);
}

ExtendedMonomial::ExtendedMonomial(const Monomial& m) :
        Monomial(m)
        ,positive_vars(monoStack->pop(false))
        ,inverted_vars(monoStack->pop())
{
    mpz_set(positive_vars, m.vars);
}

ExtendedMonomial::~ExtendedMonomial()
{
    monoStack->push(positive_vars);
    monoStack->push(inverted_vars);
}

bool ExtendedMonomial::contains(const uint var, const bool inv) const
{
    if (!mpz_tstbit(vars, var)) return false;
    if (inv) return mpz_tstbit(inverted_vars, var);
    else return mpz_tstbit(positive_vars, var);
}

bool ExtendedMonomial::contains(const uint var) const
{
    return mpz_tstbit(vars, var);
}

pair<uint, bool> ExtendedMonomial::getSingleVar() const
{
    unsigned long int var, var2;

    var = mpz_scan1(vars, 0);
    assert(var != ULONG_MAX);
    var2 = mpz_scan1(vars, var+1);
    assert(var2 == ULONG_MAX);
    bool inverted = mpz_tstbit(inverted_vars, var);
    bool positive = mpz_tstbit(positive_vars, var);
    assert(!( inverted && positive));

    return make_pair(var, inverted);
}

bool ExtendedMonomial::contains(const ExtendedMonomial& m) const
{
    mpz_and(tmp, positive_vars, m.positive_vars);
    if (mpz_cmp(tmp, m.positive_vars) != 0) return false;
    mpz_and(tmp2, inverted_vars, m.inverted_vars);
    return (mpz_cmp(tmp2, m.inverted_vars) == 0);
}

uint ExtendedMonomial::getSingleVarDiff(const ExtendedMonomial& smaller, const ExtendedMonomial& larger)
{
    assert(smaller.deg() + 1 == larger.deg());
    assert(!smaller.impossible());
    assert(!larger.impossible());

    return Monomial::getSingleVarDiff(smaller, larger);
}

void ExtendedMonomial::remove(const ExtendedMonomial& m)
{
    mpz_com(tmp, m.positive_vars);
    mpz_and(positive_vars, tmp, positive_vars);

    mpz_com(tmp, m.inverted_vars);
    mpz_and(inverted_vars, tmp, inverted_vars);

    mpz_ior(vars, positive_vars, inverted_vars);
}

void ExtendedMonomial::remove(const uint var)
{
    mpz_clrbit(positive_vars, var);
    mpz_clrbit(inverted_vars, var);
    mpz_clrbit(vars, var);
}

void ExtendedMonomial::negate(const uint var)
{
    assert(!impossible());

    if (mpz_tstbit(positive_vars, var)) {
        mpz_clrbit(positive_vars, var);
        mpz_setbit(inverted_vars, var);
    } else {
        mpz_clrbit(inverted_vars, var);
        mpz_setbit(positive_vars, var);
    }
}

bool ExtendedMonomial::clashesWith(const ExtendedMonomial& clashesWith) const
{
    mpz_and(tmp, positive_vars, clashesWith.positive_vars);
    if (mpz_cmp_ui(tmp, 0) != 0) return true;
    mpz_and(tmp, inverted_vars, clashesWith.inverted_vars);
    if (mpz_cmp_ui(tmp, 0) != 0) return true;

    return false;
}

bool ExtendedMonomial::empty() const
{
    return (mpz_cmp_ui((mpz_t&)vars, 0) == 0);
}

void ExtendedMonomial::pushVarsInto(vector<pair<uint, bool> >& topush) const
{
    unsigned long int var = 0;

    while (true) {
        var = mpz_scan1(vars, var);
        if (var == ULONG_MAX) break;

        bool found = false;
        if (mpz_tstbit(inverted_vars, var)) topush.push_back(make_pair(var, true)), found = true;
        if (mpz_tstbit(positive_vars, var)) topush.push_back(make_pair(var, false)), found = true;
        assert(found);
        var++;
    }
}

ExtendedMonomial& ExtendedMonomial::operator*=(const pair<uint, bool>& mypair)
{
    mpz_setbit(vars, mypair.first);
    if (mypair.second)  mpz_setbit(inverted_vars, mypair.first);
    else mpz_setbit(positive_vars, mypair.first);

    return *this;
}

ExtendedMonomial ExtendedMonomial::operator*(const ExtendedMonomial& m) const
{
    ExtendedMonomial ret;
    mpz_ior(ret.vars, 		vars, 		m.vars);
    mpz_ior(ret.inverted_vars, 	inverted_vars, 	m.inverted_vars);
    mpz_ior(ret.positive_vars, 	positive_vars, 	m.positive_vars);

    return ret;
}

bool ExtendedMonomial::operator==(const ExtendedMonomial& m) const
{
    //TODO if we implement impossible() monomials correctly, this can be reduced
    return (mpz_cmp(vars, m.vars) == 0 && mpz_cmp(positive_vars, m.positive_vars) == 0 && mpz_cmp(inverted_vars, m.inverted_vars) == 0);
}

bool ExtendedMonomial::mycmp(const ExtendedMonomial& a, const ExtendedMonomial& b, const bool equal_or_less)
{
    int mytmp = mpz_cmp(a.vars, b.vars);
    if ( mytmp < 0) return true;
    if ( mytmp > 0) return false;

    mytmp = mpz_cmp(a.inverted_vars, b.inverted_vars);
    if ( mytmp < 0) return true;
    if ( mytmp > 0) return false;

    //TODO if we implement impossible() monomials correctly, these 3 lines are not needed
    mytmp = mpz_cmp(a.positive_vars, b.positive_vars);
    if ( mytmp < 0) return true;
    if ( mytmp > 0) return false;

    return equal_or_less;
}

bool ExtendedMonomial::operator<=(const ExtendedMonomial& m) const
{
    return mycmp(*this, m, true);
}

bool ExtendedMonomial::operator<(const ExtendedMonomial& m) const
{
    return mycmp(*this, m, false);
}

bool ExtendedMonomial::operator!=(const ExtendedMonomial& m) const
{
    //TODO if we implement impossible() monomials correctly, this can be reduced
    return (mpz_cmp(vars, m.vars) != 0 || mpz_cmp(positive_vars, m.positive_vars) != 0 || mpz_cmp(inverted_vars, m.inverted_vars) != 0);
}

void ExtendedMonomial::swap(ExtendedMonomial& m)
{
    mpz_swap(vars, m.vars);
    mpz_swap(inverted_vars, m.inverted_vars);
    mpz_swap(positive_vars, m.positive_vars);
}

void ExtendedMonomial::setmult(const ExtendedMonomial& m1, const ExtendedMonomial& m2)
{
    mpz_ior(vars, 		m1.vars, 		m2.vars);
    mpz_ior(inverted_vars, 	m1.inverted_vars, 	m2.inverted_vars);
    mpz_ior(positive_vars, 	m1.positive_vars,	m2.positive_vars);
}

ExtendedMonomial& ExtendedMonomial::operator*=(const ExtendedMonomial& b)
{
    this->setmult(*this, b);

    return *this;
}

ExtendedMonomial& ExtendedMonomial::operator=(const ExtendedMonomial& m)
{
    mpz_set(vars, 		m.vars);
    mpz_set(inverted_vars, 	m.inverted_vars);
    mpz_set(positive_vars, 	m.positive_vars);

    return *this;
}

uint ExtendedMonomial::deg() const
{
    //TODO if we implement impossible() monomials correctly, this can be reduced to mpz_popcount(vars)
    return mpz_popcount(positive_vars) + mpz_popcount(inverted_vars);
}

ostream& operator << (ostream& os, const ExtendedMonomial& m)
{
    uint deg = m.deg();
    uint num_printed = 0;

    unsigned long int var = 0;

    while (true) {
        var = mpz_scan1(m.vars, var);
        if (var == ULONG_MAX) break;

        if (mpz_tstbit(m.positive_vars, var)) {
            num_printed++;
            os << cpd.vars.get_varname_from_varnum(var);
            if (num_printed < deg) os << "*";
        }
        if (mpz_tstbit(m.inverted_vars, var)) {
            num_printed++;
            os << "!" << cpd.vars.get_varname_from_varnum(var);
            if (num_printed < deg) os << "*";
        }
        var++;
    }

    return os;
}

ostream& operator << (ostream& os, const list<ExtendedMonomial>& monos)
{
    list<ExtendedMonomial>::const_iterator ptr = monos.begin();
    const list<ExtendedMonomial>::const_iterator end = monos.end();
    while (ptr != end) {
        if (ptr->deg() > 1) {
            os << "(" << *ptr << ")";
        } else os << *ptr;
        ptr++;
        if (ptr != monos.end()) os << " + ";
    }

    return os;
}

string ExtendedMonomial::get_desc() const
{
    stringstream ss;
    ss << *this;
    return ss.str();
}

bool ExtendedMonomial::impossible() const
{
    mpz_and(tmp, positive_vars, inverted_vars);
    return (mpz_cmp_ui(tmp, 0) != 0);
}

bool ExtendedMonomial::evaluate(mpz_t& set) const
{
    //this function is aware of impossible() (it returns false, since set cannot contain a 1 and a 0 at the same bit position). However, re-writing this function should be done carefully
    mpz_and(tmp, set, positive_vars);
    if (mpz_cmp(tmp, positive_vars) != 0) return false;

    mpz_com(tmp, set);
    mpz_and(tmp, tmp, inverted_vars);
    if (mpz_cmp(tmp, inverted_vars) != 0) return false;

    return true;
}

const mpz_t& ExtendedMonomial::get_all_vars() const
{
    return vars;
}
