
#include "solverattrib.h"
#include "cipherdesc.h"
#include "time_mem.h"
#include "equationholder.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <algorithm>

using boost::lexical_cast;

bool SolverAttrib::use_xor_clauses;
uint SolverAttrib::xor_cut_len;

std::ostream& operator << (std::ostream& os, const vector<Lit>& v);

SolverAttrib::SolverAttrib() :
    xor_permutator(xor_cut_len)
{
}

void SolverAttrib::permutateVars()
{
    variableMix.resize(cpd.vars.get_last_var());
    
    for (uint i = 0; i < variableMix.size(); i++)
        variableMix[i] = i;
    
    for (int i = 0; i < variableMix.size(); i++) {
        int j = cpd.mtrand.randInt(i);
        uint tmp = variableMix[i];
        variableMix[i] = variableMix[j];
        variableMix[j] = tmp;
    }
    
    BOOST_FOREACH(Clause& c, clauses) {
        BOOST_FOREACH(Lit& l, c.lits) {
            assert(variableMix.size() > l.var());
            l = Lit(variableMix[l.var()], l.sign());
        }
    }
    
    BOOST_FOREACH(XorClause& c, xorClauses) {
        BOOST_FOREACH(Lit& l, c.lits) {
            assert(variableMix.size() > l.var());
            l = Lit(variableMix[l.var()], l.sign());
        }
    }
}

void SolverAttrib::permutateClauses()
{
    for (int i = 0; i < clauses.size(); i++) {
        int j = cpd.mtrand.randInt(i);
        Clause tmp = clauses[i];
        clauses[i] = clauses[j];
        clauses[j] = tmp;
    }
    
    for (int i = 0; i < xorClauses.size(); i++) {
        int j = cpd.mtrand.randInt(i);
        XorClause tmp = xorClauses[i];
        xorClauses[i] = xorClauses[j];
        xorClauses[j] = tmp;
    }
}

void SolverAttrib::print_satfile(const string name, const map<uint, uint>& same_vars)
{
    if (!bs::exists(get_satfile_dir())) {
        std::cout << "Directory '" << get_satfile_dir() << "' does not exist. Trying to create it." << std::endl;
        if (!bs::create_directory(get_satfile_dir())) {
            std::cout << "Could not create directory " << get_satfile_dir() << ". Exiting." << std::endl;
            exit(-1);
        }
    }
    bs::path output_satfile_filename = get_satfile_dir();
    output_satfile_filename /= name;
    ofstream satfile;
    satfile.open(output_satfile_filename.native_file_string().c_str());
    if (!satfile)
        throw("Cannot open " + output_satfile_filename.native_file_string() + " for writing. Cannot write output file");
    
    permutateVars();
    permutateClauses();
    
    BOOST_FOREACH(const Clause& c, clauses)
        c.add(satfile);
    
    BOOST_FOREACH(const XorClause& c, xorClauses)
        c.add(satfile);
    
    satfile << "c -----------  equal variables ----------------" << std::endl;
    typedef pair<uint, uint> mypair ;
    BOOST_FOREACH(const mypair& p, same_vars) {
        satfile << "c var " << p.first+1 << " eq " << p.second+1 << std::endl;
    }
    
    add_sr_varnames(satfile);
    add_filter_varnames(satfile);
    add_mono_varnames(satfile);
    satfile.close();
}

uint SolverAttrib::get_mixed_var(const uint var) const
{
    return variableMix[var];
}

void SolverAttrib::add_sr_varnames(ofstream& satfile)
{
    satfile << "c --------------- Variables in shift registers (sr-s) ------------" << std::endl;
    for (uint i = 0; i < cpd.sr_num; i++) {
        for (uint i2 = 0; i2 < cpd.sr_size[i] + cpd.init_clock + cpd.outputs; i2++) {
            string basename = "sr[" + lexical_cast<string>(i) + "]";
            string extraname = "[" + lexical_cast<string>(i2) + "]";
            if (i2 >= cpd.sr_shift[i] && i2 < (cpd.sr_shift[i] + cpd.sr_size[i]) )
                extraname += "(real unknown)";
            
            uint var = cpd.vars.get_array_var(sr_type, i, i2);
            var = variableMix[var];
            
            satfile << "c v " <<  var + 1 << " " << (basename+extraname) << std::endl;
        }
    }
    satfile << "c --------------- End variables in shift registers (sr-s) ------------" << std::endl;
}

void SolverAttrib::add_filter_varnames(ofstream& satfile)
{
    satfile << "c --------------- Variables of filters (f-s) ------------" << std::endl;
    for (uint i = 0; i < cpd.filter_num; i++) {
        for (uint i2 = 0; i2 < cpd.init_clock + cpd.outputs; i2++) {
            string basename = "f[" + lexical_cast<string>(i) + "]";
            string extraname = "[" + lexical_cast<string>(i2) + "]";
            
            uint var = cpd.vars.get_array_var(filter_type, i, i2);
            var = variableMix[var];
            
            satfile << "c v " << var + 1 << " " << (basename+extraname) << std::endl;
        }
    }
    satfile << "c --------------- End variables in filters (f-s) ------------" << std::endl;
}

void SolverAttrib::add_mono_varnames(ofstream& satfile)
{
    satfile << "c --------------- Variables assigned to monomials (monos-s) ------------" << std::endl;
    
    typedef const pair<ExtendedMonomial, Var> mypair;
    
    BOOST_FOREACH(const mypair extmono_pair, cpd.vars.get_extmonos()) {
        string name = cpd.vars.get_varname_from_varnum(extmono_pair.second);
        
        uint var = extmono_pair.second;
        var = variableMix[var];
        
        satfile << "c v " << var + 1 << " " << name << std::endl;
    }
    satfile << "c --------------- End variables assigned to monomials (monos-s) ------------" << std::endl;
}

void SolverAttrib::addXorClause(vector<Lit>& lits, const uint clause_group, const string desc)
{
    if (use_xor_clauses) 
        xorClauses.push_back(XorClause(lits, clause_group, desc));
    else {
        uint last_var = UINT_MAX;
        for (uint i = 0; i < lits.size();) {
            vector<Lit> newlits;
            
            //chain it
            if (i > 0) newlits.push_back(Lit(last_var, false));
            
            while ((newlits.size() < xor_cut_len-1) && (i < lits.size()))
                newlits.push_back(lits[i++]);
            
            if (i < lits.size()) {
                last_var = cpd.vars.add_cutxor(newlits);
                newlits.push_back(Lit(last_var, true));
            }
            
            list<vector<Lit> > all_comb(xor_permutator.all_combinations(newlits, true));
            
            typedef vector<Lit> lit_vec;
            BOOST_FOREACH(lit_vec& lits, all_comb)
                addClause(lits, clause_group, desc);
        }
    }
}

void SolverAttrib::addClause(vector<Lit>& lits, const uint clause_group, const string& desc)
{
    clauses.push_back(Clause(lits, clause_group, desc));
}

bs::path SolverAttrib::get_satfile_dir()
{
    bs::path dirname( bs::initial_path<bs::path>() );
    dirname /= cpd.cnfDir;
    
    return dirname;
}

void Clause::add(ofstream& satfile) const
{
    satfile << lits << "0" << std::endl;
    satfile  << "c group " << group << " "  << desc << std::endl;
}

void XorClause::add(ofstream& satfile) const
{
    satfile << "x" << lits << "0" << std::endl;
    satfile << "c group " << group << " "  << desc << std::endl;
}
