#ifndef SOLVERATTRIB_H
#define SOLVERATTRIB_H

#include "variables.h"
#include <fstream>
#include <boost/filesystem.hpp>

#include "xor_computer.h"
#include "SolverTypes.h"

using std::ofstream;
namespace bs = boost::filesystem;

class Clause
{
public:
    Clause(vector<Lit> _lits, const uint _group, const string _desc) :
        lits(_lits)
        , desc(_desc)
        , group(_group)
        {}
    
    vector<Lit> lits;
    uint group;
    string desc;
    
    void add(ofstream& satfile) const;
};

class XorClause
{
public:
    XorClause(vector<Lit> _lits, const uint _group, const string _desc) :
        lits(_lits)
        , desc(_desc)
        , group(_group)
    {}
    
    vector<Lit> lits;
    uint group;
    string desc;
    uint matrix_no;
    
    void add(ofstream& satfile) const;
};

class SolverAttrib
{
public:
    
    SolverAttrib();
    
    void addClause(vector<Lit>& lits, const uint clause_group, const string& desc);
    void addXorClause(vector<Lit>& lits, const uint clause_group, const string desc);
    
    void setAttribs();
    void printAttribs();
    uint get_mixed_var(const uint var) const;
    static bs::path get_satfile_dir();
    
    void print_satfile(const string name, const map<uint, uint>& same_vars);
    
    static bool use_xor_clauses; ///<Should we use xor-clauses? If not, then cutting will be done
    static uint xor_cut_len; ///<If not using xor-clauses what should be the cutting lenght? (default to 7)
    
    double solving_time;
    
private:
    //----------------------------------------
    //Addig info to the SAT solver (names)
    //----------------------------------------
    void add_sr_varnames(ofstream& satfile); ///<Add the names of the shift register's variables to the solver
    void add_mono_varnames(ofstream& satfile);///<Add the names of the monomials to the solver
    void add_filter_varnames(ofstream& satfile); ///<Add the names of the filter functions's variables to the solver
    
    void setupPermutateVars(); ///<Sets up permutateVars. Must be called, even if permutation is not performed
    void permutateVars(); ///<Permutate variables
    void permutateClauses(); ///<Permutate the clauses and the xorClauses
    
    ///A helper function to add_as_xor_CNF
    XOR_permutator xor_permutator; ///<A helper class for add_as_xor_CNF
    
    vector<Clause> clauses;
    vector<XorClause> xorClauses;
    vector<uint> variableMix;
};


#endif



