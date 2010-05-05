//
// C++ Interface: variables
//
// Author: Mate SOOS <soos@inrialpes.fr> (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef VARIABLES_H
#define VARIABLES_H

#include <vector>
#include <map>
#include <string>
#include <sys/types.h>

#include "dataelement.h"
#include "extendedmonomial.h"
#include "SolverTypes.h"

using std::vector;
using std::map;
using std::string;

/**
@brief Holds the type and the which_of_type. It's an extended simple element.

Used to encompass a bit more info. It can also sort
*/
class ExtendedElement
{
public:
    ExtendedElement(const element_type _type, const uint _which_of_type) :
            type(_type)
            , which_of_type(_which_of_type) {}

    const element_type type; ///<The type of the element
    const uint which_of_type; ///<Which of the type (i.e. sr1, sr2, etc.)

    ///To be able to sort them
    const bool operator < (const ExtendedElement& b) const {
        if (type < b.type) return true;
        if (type > b.type) return false;
        //element_type are equal

        if (which_of_type < b.which_of_type) return true;
        if (which_of_type > b.which_of_type) return false;
        //they are the same
        return false;
    }
};


/**
@brief Responsible for keeping track of variables.

It is possible to request variable ranges, and it is possible to query what a variable means. This class takes care of re-use of the same monomials and internal cutted xor-s
*/
class Variables
{
public:
    Variables(); ///<Initialises "next_avialable_var" and "last_noninternal_var"

    /** 
    @brief Reserves a variable range
    @param type the type of variable (shift register, function..)
    @param which_of_type The number of the type (sr1 or sr2, etc.)
    @param size the number of varibles needed
    */
    void reserve_array(const element_type type, const uint which_of_type, const uint size);
    
    ///Return the variable number for a given element's given index
    uint get_array_var(const element_type type, const uint which_of_type, const uint index) const;
    
    ///The extended monomial is already in the database and has been defined
    bool extmono_exists(const ExtendedMonomial& extmono) const;
    
    /**
    @brief Add a new, non-existent ExtendedMonomial to the database
    @param[in] extmono The ExtendedMonomial we want to define
    @return The variable the extmono has been assigned to
    */
    uint add_extmono(const ExtendedMonomial& extmono);
    
    /**
    @brief Returns the extended monomial's variable
    \pre The extended monomial has been defined previously
    */
    uint get_extmono_var(const ExtendedMonomial& extmono) const;
    
    /**
    @brief Returns the cutted xor's variable
    \pre The cutted xor has been defined previously
    */
    uint add_cutxor(const vector<Lit>& lits);
    
    
    uint get_cutxor_var(const vector<Lit>& lits);
    
    ///Return the variable number for a given element. The element is specified as a DataElement
    uint get_array_var(const DataElement& d) const;
    vector<uint> get_array(const element_type type, const uint which_of_type) const;
    /**
    @brief Get the description of the variable from the variable number
    @param var the variable number
    @return the name of the variable
    */
    string get_varname_from_varnum(const uint var) const;
    
    /**
    @brief Returns the type of the variable from the variable number
    @param var the variable number
    @return the type of the variable
    */
    DataElement get_type_from_var(const uint var) const;
    
    const map<ExtendedMonomial, Var>& get_extmonos() const;
    const map<vector<Lit>, Var>& get_cutxors() const;
    
    /**
    @brief Returns whether the variable is internal or not
    internal variable means that it's either a variable representing an ExtendedMono or one representing a cutted xor
    @param[in] the variable queried
    @return whether it is internal or not
    */
    bool var_is_internal(const Var var) const;
    
    uint get_last_noninternal_var() const;
    uint get_last_var();
    
    void clear_internal_vars();
    void full_reset();
private:
    
    ///Stores a variable range (start, end, size)
    class VarArrayData
    {
    public:
        VarArrayData(const uint _var_start, const uint _var_end, const uint _size) :
                var_start(_var_start)
                ,var_end(_var_end)
        {
        }
        
        const uint size() const { return var_end - var_start; }

        const uint var_start; ///<start of the range (inclusive)
        const uint var_end; ///<end of the range (non-inclusive)
    };
    
    map<ExtendedElement, VarArrayData> arrays; ///<Variable arrays are stored here
    map<ExtendedMonomial, Var> extmonos; ///<Extended monomials are stored here
    map<vector<Lit>, Var> cutxors; ///<Cutted xors are stored here
    uint next_available_var; ///<Stores the next available (i.e. non-allocated) variable number. This variable will be allocated next.
    uint last_noninternal_var; ///<Stores the last variable number that was non-internal. This variable has been allocated.
    
    bool frozen; ///<Freeze the database, no more get_new_ is allowed
};

#endif
