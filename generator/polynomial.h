/*
 * polynomial.h
 *
 *  Created on: Aug 13, 2008
 *      Author: Karsten Nohl, Mate Soos
 */

#ifndef POLYNOMIAL_H_
#define POLYNOMIAL_H_

#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <gmp.h>
#include "assert.h"
#include "monomial.h"
#include <boost/pool/pool_alloc.hpp>
#include <boost/pool/object_pool.hpp>

using std::vector;
using std::list;

class Polynomial
{
public:
    //Initalisation of static variables in multi-thread environment
    static uint get_num_deleted_monomials();
    static uint get_num_deleted_monomials_deg();
    static void init_temps();
    static void delete_temps();

    //Constructors
    Polynomial(const bool inverted);
    Polynomial(const Monomial& m);

    //Monomial->Polynomial replacement functions
    Polynomial& operator^=(const Monomial& mono);
    Polynomial operator*(const Polynomial& b) const;
    Polynomial& operator=(const Monomial& a);
    Polynomial& operator*=(const Monomial& b);
    uint replace(const Monomial& to_replace, const Polynomial& replace_with);

    //Miscellaneous functions
    bool operator==(const Monomial& a) const;
    void invert();
    void get_all_vars(vector<uint>& all_vars) const;
    void get_all_vars(mpz_t& all_vars) const;
    uint get_num_vars() const;
    const bool is_inverted() const;
    const list<Monomial>& get_monos() const;
    bool evaluate(mpz_t& set) const;
    void add_monomial_distribution(vector<uint>& distrib) const;
    uint size() const;
    bool empty() const;
    uint sum_mono_deg() const;
    bool simpleXOR() const;

    //the fixed size of mixedMonos
    static const uint maxMonos = 100;

    friend ostream& operator << (ostream& os, const Polynomial& p);

private:
    //Monomial->Polynomial replacement functions
    list<Monomial>::iterator find_first_larger_or_equal_to(const Monomial& m);
    Polynomial& xorWithAndRemoveSecond(Polynomial& b);
    void cleanFromDoubles(list<Monomial>& monos, const list<Monomial>::iterator smallest, const list<Monomial>::iterator largest);

    //Debug functions
    static bool check_polynomial(const list<Monomial>& poly);
    static bool check_polynomial_order(const list<Monomial>& poly);

    //Private variables
    bool inverted;
    list<Monomial> monos;

    //Class-wide, thread-specific variables
    static uint num_deleted_monomials;
    static uint num_deleted_monomials_deg;
    static __thread Monomial* mixedMonos;
    static __thread list<Monomial>* monoList;
};

//Printing functions
extern ostream& operator << (ostream& os, const Polynomial& p);

#endif /* POLYNOMIAL_H_ */
