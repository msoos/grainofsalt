//
// C++ Implementation: variables
//
// Author: Mate SOOS <soos@inrialpes.fr> (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "variables.h"
#include "assert.h"
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/concept_check.hpp>

using boost::lexical_cast;
using std::pair;
using std::make_pair;
using std::stringstream;

Variables::Variables() :
        next_available_var(0)
        , last_noninternal_var(0)
        , frozen(false)
{
}

void Variables::full_reset()
{
    frozen = false;
    arrays.clear();
    extmonos.clear();
    cutxors.clear();
    next_available_var = 0;
    last_noninternal_var = 0;
}

void Variables::reserve_array(const element_type type, const uint which_of_type, const uint size)
{
    assert(!frozen);
    
    VarArrayData ret(next_available_var, next_available_var + size, size);
    arrays.insert(std::make_pair(ExtendedElement(type, which_of_type), ret));

    next_available_var += size;
    last_noninternal_var = next_available_var-1;
}

uint Variables::get_array_var(const element_type type, const uint which_of_type, const uint index) const
{
    const map<ExtendedElement, VarArrayData>::const_iterator it(arrays.find(ExtendedElement(type, which_of_type)));
    assert(it != arrays.end());
    assert(it->second.size() > index);

    return it->second.var_start + index;
}

uint Variables::get_array_var(const DataElement& d) const
{
    return get_array_var(d.type, d.which_of_type, d.index);
}

vector<uint> Variables::get_array(const element_type type, const uint which_of_type) const
{
    vector<uint> ret;

    const map<ExtendedElement, VarArrayData>::const_iterator it(arrays.find(ExtendedElement(type, which_of_type)));
    assert(it != arrays.end());

    for (uint i = 0; i < it->second.size(); i++)
        ret.push_back(it->second.var_start + i);

    return ret;
}

string Variables::get_varname_from_varnum(const uint var) const
{
    typedef pair<ExtendedElement, VarArrayData> mypair;
    BOOST_FOREACH(const mypair& p, arrays) {
        if (var >= p.second.var_start && var < p.second.var_end) {
            stringstream ss;
            ss << DataElement(p.first.type, p.first.which_of_type, var - p.second.var_start);
            return ss.str();
        }
    }
    
    typedef pair<ExtendedMonomial, Var> mypair2;
    BOOST_FOREACH(const mypair2& p, extmonos) if (p.second == var) {
        stringstream ss;
        ss << p.first;
        return ss.str();
    }

    typedef pair<vector<Lit>, Var> mypair3;
    BOOST_FOREACH(const mypair3& p, cutxors) if (p.second == var) {
        stringstream ss;
        ss << "cut-xor-var";
        return ss.str();
    }

    assert(false && "Var out of range");
}

bool Variables::var_is_internal(const uint var) const
{
    return (var > last_noninternal_var);
}

DataElement Variables::get_type_from_var(const uint var) const
{
    typedef pair<ExtendedElement, VarArrayData> mypair;
    BOOST_FOREACH(const mypair& p, arrays) {
        if (var >= p.second.var_start && var < p.second.var_end)
            return DataElement(p.first.type, p.first.which_of_type, var - p.second.var_start);
    }
    
    assert(false && "Var out of range");
}

uint Variables::get_extmono_var(const ExtendedMonomial& extmono) const
{
    map<ExtendedMonomial, Var>::const_iterator it = extmonos.find(extmono);
    if (it == extmonos.end()) throw("ooops, this extmono does not exist!");
    else return it->second;
}

uint Variables::add_cutxor(const vector<Lit>& lits)
{
    assert(!frozen);
    
    cutxors[lits] = next_available_var++;
    return next_available_var-1;
}

bool Variables::extmono_exists(const ExtendedMonomial& extmono) const
{
    map<ExtendedMonomial, Var>::const_iterator it = extmonos.find(extmono);
    if (it != extmonos.end()) return true;
    return false;
}

uint Variables::add_extmono(const ExtendedMonomial& extmono)
{
    assert(!frozen);
    
    extmonos[extmono] = next_available_var++;
    return next_available_var-1;
}

uint Variables::get_last_noninternal_var() const
{
    return last_noninternal_var;
}

const map<ExtendedMonomial, Var>& Variables::get_extmonos() const
{
    return extmonos;
}

const map<vector<Lit>, Var>& Variables::get_cutxors() const
{
    return cutxors;
}

uint Variables::get_last_var()
{
    frozen = true;
    return next_available_var;
}

void Variables::clear_internal_vars()
{
    frozen = false;
    extmonos.clear();
    cutxors.clear();
    next_available_var = last_noninternal_var+1;
}

