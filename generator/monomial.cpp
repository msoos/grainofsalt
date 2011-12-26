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

#include "monomial.h"
#include "cipherdesc.h"
#include "mystack.h"

using std::make_pair;
using std::stringstream;

//------------------------------
// Static initalizers
//------------------------------

__thread mpz_t Monomial::tmp;
__thread MonoStack* Monomial::monoStack = NULL;

void Monomial::init_temps()
{
    mpz_init2(tmp, cpd.vars.get_last_noninternal_var()+1);
    monoStack = cpd.get_free_monoStack();
}

void Monomial::delete_temps()
{
    mpz_clear(tmp);
    cpd.return_monoStack(monoStack);
}

//------------------------------
// Constructors
//------------------------------

Monomial::Monomial() :
        vars(monoStack->pop())
{
}

Monomial::Monomial(const Monomial& m) :
        vars(monoStack->pop(false))
{
    mpz_set(vars, m.vars);
}

Monomial::Monomial(const uint var, const bool inverted) :
        vars(monoStack->pop())
{
    mpz_setbit(vars, var);
}

Monomial::~Monomial()
{
    monoStack->push(vars);
}

bool Monomial::contains(const uint var, const bool inv) const
{
    if (inv) return false;
    return mpz_tstbit(vars, var);
}

uint Monomial::getSingleVar() const
{
    unsigned long int var, var2;

    var = mpz_scan1(vars, 0);
    assert(var != ULONG_MAX);
    var2 = mpz_scan1(vars, var+1);
    assert(var2 == ULONG_MAX);

    return var;
}

bool Monomial::contains(const Monomial& m) const
{
    mpz_and(tmp, vars, m.vars);
    return (mpz_cmp(tmp, m.vars) == 0 );
}

void Monomial::remove(const Monomial& m)
{
    mpz_com(tmp, m.vars);
    mpz_and(vars, tmp, vars);
}

bool Monomial::clashesWith(const Monomial& clashesWith) const
{
    mpz_and(tmp, vars, clashesWith.vars);
    if (mpz_cmp_ui(tmp, 0) != 0) return true;
    return false;
}

bool Monomial::empty() const
{
    return (mpz_cmp_ui(vars, 0) == 0);
}

uint Monomial::getSingleVarDiff(const Monomial& smaller, const Monomial& larger)
{
    mpz_com(tmp, smaller.vars);
    mpz_and(tmp, tmp, larger.vars);

    unsigned long int var, var2;

    var = mpz_scan1(tmp, 0);
    assert(var != ULONG_MAX);
    var2 = mpz_scan1(tmp, var+1);
    assert(var2 == ULONG_MAX);

    return var;
}

void Monomial::pushVarsInto(vector<pair<uint, bool> >& topush) const
{
    unsigned long int var = 0;

    while (true) {
        var = mpz_scan1(vars, var);
        if (var == ULONG_MAX) break;

        topush.push_back(make_pair(var, false));
        var++;
    }
}

Monomial& Monomial::operator*=(const pair<uint, bool>& mypair)
{
    assert(mypair.second == false);
    mpz_setbit(vars, mypair.first);

    return *this;
}

Monomial Monomial::operator*(const Monomial& m) const
{
    Monomial ret;
    mpz_ior(ret.vars, 		vars, 		m.vars);

    return ret;
}

bool Monomial::operator==(const Monomial& m) const
{
    return (mpz_cmp(vars, m.vars) == 0);
}

bool Monomial::mycmp(const Monomial& a, const Monomial& b, const bool equal_or_less)
{
    int mytmp = mpz_cmp(a.vars, b.vars);
    if ( mytmp < 0) return true;
    if ( mytmp > 0) return false;

    return equal_or_less;
}

bool Monomial::operator<=(const Monomial& m) const
{
    return mycmp(*this, m, true);
}

bool Monomial::operator<(const Monomial& m) const
{
    return mycmp(*this, m, false);
}

bool Monomial::operator!=(const Monomial& m) const
{
    return (mpz_cmp(vars, m.vars) != 0);
}

void Monomial::swap(Monomial& m)
{
    mpz_swap(vars, m.vars);
}

void Monomial::setmult(const Monomial& m1, const Monomial& m2)
{
    mpz_ior(vars, 		m1.vars, 		m2.vars);
}

Monomial& Monomial::operator*=(const Monomial& b)
{
    this->setmult(*this, b);

    return *this;
}

Monomial& Monomial::operator=(const Monomial& m)
{
    mpz_set(vars, 		m.vars);

    return *this;
}

uint Monomial::deg() const
{
    return mpz_popcount(vars);
}

ostream& operator << (ostream& os, const Monomial& m)
{
    uint deg = m.deg();
    uint num_printed = 0;

    unsigned long int var = 0;

    while (true) {
        var = mpz_scan1(m.vars, var);
        if (var == ULONG_MAX) break;

        num_printed++;
        os << var;
        if (num_printed < deg) os << "*";
        var++;
    }

    return os;
}

ostream& operator << (ostream& os, const list<Monomial>& monos)
{
    list<Monomial>::const_iterator ptr = monos.begin();
    const list<Monomial>::const_iterator end = monos.end();
    while (ptr != end) {
        if (ptr->deg() > 1) {
            os << "(" << *ptr << ")";
        } else os << *ptr;
        ptr++;
        if (ptr != monos.end()) os << " + ";
    }

    return os;
}

string Monomial::get_desc() const
{
    stringstream ss;
    ss << *this;
    return ss.str();
}

bool Monomial::impossible() const
{
    return false;
}

const mpz_t& Monomial::get_all_vars() const
{
    return vars;
}

