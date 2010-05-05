/*
 * polynomial.cpp
 *
 *  Created on: Aug 13, 2008
 *      Author: Karsten Nohl, Mate Soos
 */

#include <algorithm>
#include <boost/foreach.hpp>
#include "assert.h"
#include "string.h"
#include "limits.h"

#include "polynomial.h"
#include "cipherdesc.h"

//#define DEBUG
//#define VERBOSE_DEBUG
//#define VERBOSE_DEBUG_REPLACE
//#define VERBOSE_DEBUG_XORWITH
//#define VERBOSE_DEBUG_CLEAN
//#define VERBOSE_DEBUG_MULTMONO
//#define INVERSION_HACKS

__thread Monomial* Polynomial::mixedMonos = NULL;
__thread list<Monomial>* Polynomial::monoList = NULL;
uint Polynomial::num_deleted_monomials = 0;
uint Polynomial::num_deleted_monomials_deg = 0;

//---------------------
//Static init in multi-thread
//---------------------

void Polynomial::init_temps()
{
    mixedMonos = cpd.get_free_mixedMonos();
    monoList = new list<Monomial>;
}

void Polynomial::delete_temps()
{
    cpd.return_mixedMonos(mixedMonos);
    delete monoList;
}

//---------------------
//Constructors
//---------------------

Polynomial::Polynomial(const bool _inverted) :
        inverted(_inverted)
{
}

Polynomial::Polynomial(const Monomial& m) :
        inverted(false)
{
    monos.push_back(m);
}

Polynomial& Polynomial::operator=(const Monomial& a)
{
    inverted = false;
    monos.clear();
    monos.push_back(a);

    return *this;
}

//-----------------------------
// Monomial->Polynomial replacement functions
//-----------------------------

void Polynomial::cleanFromDoubles(list<Monomial>& monos, const list<Monomial>::iterator smallest, const list<Monomial>::iterator largest)
{
#ifdef DEBUG
    assert(check_polynomial_order(monos));
#ifdef VERBOSE_DEBUG_CLEAN
    cout << "void Polynomial::cleanFromDoubles(list<Monomial>& monos, const list<Monomial>::iterator largest)" << endl;
    cout << "Original:" << monos << endl;
#endif
#endif
    list<Monomial>::iterator last = largest;
    for (list<Monomial>::iterator i = smallest; i != largest;) {
        if (last == largest ||  *last != *i) {
            last = i;
            i++;
        } else {
#ifdef VERBOSE_DEBUG_CLEAN
            cout << "Cleaning a + a: last: " << *last << " - it: " << *i << endl;
#endif

            monos.erase(last);
            num_deleted_monomials++;
            num_deleted_monomials_deg += i->deg();
            i = monos.erase(i);
            last = largest;
        }
    }

#ifdef DEBUG
    assert(check_polynomial(monos));
#ifdef VERBOSE_DEBUG_CLEAN
    cout << "Final:" << monos << endl << endl;
#endif
#endif
}

Polynomial& Polynomial::xorWithAndRemoveSecond(Polynomial& b)
{
#ifdef DEBUG
    assert(check_polynomial(monos));
    assert(check_polynomial(b.monos));
#ifdef VERBOSE_DEBUG_XORWITH
    cout << "Polynomial& Polynomial::xorWithAndRemoveSecond(Polynomial& b)" << endl;
    cout << "Original:" << *this << endl;
    cout << "second:" << b << endl;
#endif
#endif

    inverted ^= b.inverted;
    if (!b.empty()) {
        list<Monomial>::iterator smallest = b.monos.begin();
        list<Monomial>::iterator largest = b.monos.end();
        largest--;
#ifdef VERBOSE_DEBUG_XORWITH
        cout << "smallest in second:" << *smallest << endl;
        cout << "largest in second:" << *largest << endl;
#endif

        monos.merge(b.monos);
        largest++;
        if (smallest != monos.begin()) smallest--;
        cleanFromDoubles(monos, smallest, largest);
    }

#ifdef DEBUG
    assert(check_polynomial(monos));
#ifdef VERBOSE_DEBUG_XORWITH
    cout << "Final:" << *this << endl << endl;
#endif
#endif

    return *this;
}

Polynomial& Polynomial::operator*=(const Monomial& b)
{
    if (b.empty()) return *this;

#ifdef INVERSION_HACKS
    if (inverted && size() == 1) {
        *monos.begin() *= b;
        if (monos.begin()->impossible()) monos.clear();

        return *this;
    }
#endif

    list<Monomial> orderedList;

#ifdef DEBUG
    assert(check_polynomial(monos));
    uint original_size = monos.size();
    uint num_ordered = 0;
#ifdef VERBOSE_DEBUG_MULTMONO
    cout << "Polynomial& Polynomial::operator*=(const Monomial& b)" << endl;
    cout << "Original:" << *this << endl;
    cout << "multiplying by:" << b << endl;
#endif
#endif
    uint num_mixed = 0;
    list<Monomial>::iterator splice_from, splice_to;
    splice_from  = splice_to = monos.begin();
    const list<Monomial>::const_iterator end = monos.end();
    for (list<Monomial>::iterator ptra = monos.begin(); ptra != end; ) {
        if (ptra->clashesWith(b)) {
            if (splice_from != splice_to)
                orderedList.splice(orderedList.end(), monos, splice_from, splice_to);
            mixedMonos[num_mixed++].setmult(*ptra,  b);
            assert(num_mixed < maxMonos);
            ptra++;
            splice_from = splice_to = ptra;
        } else {
            ptra->setmult(*ptra, b);
            ptra++;
            splice_to = ptra;
#ifdef DEBUG
            num_ordered++;
#endif
        }
    }
    if (splice_from != splice_to)
        orderedList.splice(orderedList.end(), monos, splice_from, splice_to);

#ifdef DEBUG
    assert(num_ordered + num_mixed == original_size);
    assert(orderedList.size() == num_ordered);
    assert(monos.size() == num_mixed);
    assert(check_polynomial(orderedList));
#ifdef VERBOSE_DEBUG_MULTMONO
    cout << "orderedList: ";
    for (list<Monomial>::iterator ptra = orderedList.begin(); ptra != orderedList.end(); ptra++) cout << *ptra << " + ";
    cout << endl;

    cout << "mixed: ";
    for (uint v = 0;  v < num_mixed; v++) cout << mixedMonos[v] << " + ";
    cout << endl;
#endif
#endif
    if (inverted) {
        mixedMonos[num_mixed++] = b;
        monos.push_back(Monomial()); //just to make space
        inverted ^= true;
    }
    std::sort(mixedMonos, mixedMonos + num_mixed);

#ifdef DEBUG
    assert(monos.size() == num_mixed);
#endif
    list<Monomial>::iterator it = monos.begin();
    for (uint i = 0; i < num_mixed; i++, it++)
        it->swap(mixedMonos[i]);
#ifdef DEBUG
    assert(it == monos.end());
    assert(check_polynomial_order(monos));
#ifdef VERBOSE_DEBUG_MULTMONO
    cout << "Mixed sorted:" << *this << endl;
#endif
#endif

    monos.merge(orderedList);
    cleanFromDoubles(monos, monos.begin(), monos.end());

#ifdef DEBUG
    assert(check_polynomial(monos));
#ifdef VERBOSE_DEBUG_MULTMONO
    cout << "Final:" << *this << endl << endl;
#endif
#endif

    return *this;
}

Polynomial& Polynomial::operator^=(const Monomial& mono)
{
    assert(!mono.empty());
#ifdef DEBUG
    assert(check_polynomial(monos));
#ifdef VERBOSE_DEBUG
    cout << "Polynomial& Polynomial::operator^=(const Monomial& mono)" << endl;
    cout << "Original:" << *this << endl;
    cout << "inserting:" << mono << endl;
#endif
#endif

    list<Monomial>::iterator i = monos.begin();
    const list<Monomial>::iterator end = monos.end();
    for (; i != end; i++)
        if (!(*i < mono)) break;

    if (i == end || mono != *i)
        monos.insert(i, mono);
    else
        i = monos.erase(i);

#ifdef DEBUG
    assert(check_polynomial(monos));
#ifdef VERBOSE_DEBUG
    cout << "Final:" << *this << endl << endl;
#endif
#endif
    return *this;
}

list<Monomial>::iterator Polynomial::find_first_larger_or_equal_to(const Monomial& m)
{
    const list<Monomial>::const_iterator end = monos.end();
    for (list<Monomial>::iterator it = monos.begin(); it != end; it++) {
        if (m <= *it) return it;
    }

    return monos.end();
}

uint Polynomial::replace(const Monomial& to_replace, const Polynomial& replace_with)
{
#ifdef DEBUG
    assert(check_polynomial(replace_with.monos));
    assert(check_polynomial(monos));
#ifdef VERBOSE_DEBUG_REPLACE
    cout << "uint Polynomial::replace(const Monomial& to_replace, const Polynomial& replace_with)" << endl;
    cout << "replacing:" << to_replace << " with: " << replace_with << endl;
#endif
#endif
    uint num_replaced = 0;

    //this is only to speed it up. It actually works with complete monomials, not only with vars -- however, beware about inverted monomials...
    uint to_replace_var = to_replace.getSingleVar();

    //cerr << ".";
    const list<Monomial>::iterator end = monos.end();
    for (list<Monomial>::iterator i = monos.begin(); i != end; ) {
        if (i->contains(to_replace_var)) {
            bool var_was_inverted = i->contains(to_replace_var, true);
#ifdef VERBOSE_DEBUG_REPLACE
            cout << "-----------" << endl;
            cout << "original poly:" << *this << endl;
            cout << "original mono:" << *i << endl;
#endif
            Monomial m(*i);
            monos.erase(i);
            m.remove(to_replace_var);

            Polynomial p(replace_with);
            if (var_was_inverted) p.invert();
            p *= m;

            list<Monomial>::iterator first_larger_or_equal = find_first_larger_or_equal_to(m);
            monoList->splice(monoList->begin(), monos, monos.begin(), first_larger_or_equal);
#ifdef VERBOSE_DEBUG_REPLACE
            cout << "cleaned mono   :" << m << endl;
            cout << "sliced         :" << *monoList << endl;
            cout << "remaining monos:" << monos << endl;
#endif
            this->xorWithAndRemoveSecond(p);
            i = monos.begin();
            monos.splice(monos.begin(), *monoList, monoList->begin(), monoList->end());

#ifdef VERBOSE_DEBUG_REPLACE
            cout << "final          :" << *this << endl;
#endif
            num_replaced++;
        } else
            i++;
    }

#ifdef DEBUG
    assert(check_polynomial(monos));
#endif

    return num_replaced;
}

//-----------------------------
// Debug functions
//-----------------------------

bool Polynomial::check_polynomial(const list<Monomial>& poly)
{
    if (poly.size() == 1) return true;

    list<Monomial>::const_iterator it = poly.begin();
    list<Monomial>::const_iterator it2 = poly.begin();
    it2++;
    const list<Monomial>::const_iterator end = poly.end();
    for (; it2 != end; it++, it2++) {
        //check integrity of integrity checker
        assert(it != it2);
        //check order of polynomial
        if (!(*it < *it2)) return false;
    }

    return true;
}

bool Polynomial::check_polynomial_order(const list<Monomial>& poly)
{
    if (poly.size() == 1) return true;

    list<Monomial>::const_iterator it = poly.begin();
    list<Monomial>::const_iterator it2 = poly.begin();
    it2++;
    const list<Monomial>::const_iterator end = poly.end();
    for (; it2 != end; it++, it2++) {
        //check integrity of integrity checker
        assert(it != it2);
        //check integrity of polynomial
        if (!(*it <= *it2)) return false;
    }

    return true;
}

//------------------------------
// Printing functions
//------------------------------

ostream& operator << (ostream& os, const Polynomial& p)
{
    if (p.inverted) os << "!(";
    os << p.monos;
    if (p.inverted) os << ")";

    return os;
}

//------------------------------
// Miscellaneous functions
//------------------------------

bool Polynomial::evaluate(mpz_t& set) const
{
    bool final = inverted;

    BOOST_FOREACH(const Monomial& m, monos) {
        final ^= m.evaluate(set);
#ifdef VERBOSE_DEBUG
        cout << "mono eval:" << m.evaluate(set) << endl;
#endif
    }

#ifdef VERBOSE_DEBUG
    cout << "final:" << final << endl;
#endif
    return final;
}

void Polynomial::invert()
{
    inverted ^= true;
}

void Polynomial::get_all_vars(vector<uint>& ret) const
{
    ret.clear();
    mpz_t all_vars;
    mpz_init2(all_vars, cpd.vars.get_last_noninternal_var()+1);

    get_all_vars(all_vars);

    unsigned long int var = 0;
    while (true) {
        var = mpz_scan1(all_vars, var);
        if (var == ULONG_MAX) break;
        else ret.push_back((uint)var);
        var++;
    }

    mpz_clear(all_vars);
}

void Polynomial::get_all_vars(mpz_t& all_vars) const
{
    mpz_set_ui(all_vars, 0);
    BOOST_FOREACH(const Monomial& m, monos)
        mpz_ior(all_vars, all_vars, m.get_all_vars());
}

uint Polynomial::get_num_vars() const
{
    mpz_t all_vars;
    mpz_init2(all_vars, cpd.vars.get_last_noninternal_var()+1);

    get_all_vars(all_vars);
    uint ret = mpz_popcount(all_vars);

    mpz_clear(all_vars);

    return ret;
}

bool Polynomial::simpleXOR() const
{
    BOOST_FOREACH(const Monomial& m, monos)
        if (m.deg() > 1) return false;

    return true;
}

uint Polynomial::size() const
{
    return monos.size();
}

bool Polynomial::empty() const
{
    return monos.empty();
}

uint Polynomial::sum_mono_deg() const
{
    uint ret = 0;
    BOOST_FOREACH(const Monomial& m, monos)
        ret += m.deg();

    return ret;
}

const bool Polynomial::is_inverted() const
{
    return inverted;
}

const list<Monomial>& Polynomial::get_monos() const
{
    return monos;
}

void Polynomial::add_monomial_distribution(vector<uint>& distrib) const
{
    BOOST_FOREACH(const Monomial& m, monos) {
        uint deg = m.deg();
        if (deg >= distrib.size()) distrib.resize(deg+1, 0);
        distrib[deg]++;
    }
}

bool Polynomial::operator==(const Monomial& a) const
{
    if (monos.size() != 1) return false;
    if (!inverted && a == *monos.begin()) return true;

    return false;
}

uint Polynomial::get_num_deleted_monomials_deg()
{
    return num_deleted_monomials_deg;
}

uint Polynomial::get_num_deleted_monomials()
{
    return num_deleted_monomials;
}
