//
// C++ Interface: bestbits
//
// Description:
//
//
// Author: Mate SOOS <mate.soos@inrialpes.fr> (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BESTBITS_H
#define BESTBITS_H

#include <vector>
#include <string>
#include <boost/filesystem/path.hpp>
//#include <boost/thread/mutex.hpp>

#include "dataelement.h"

using std::vector;
using std::string;
using std::map;

namespace bs = boost::filesystem;

class GrainOfSalt;

/**
@brief Handles everything that has to do with guessed bits

1) Calculates the best bits (=shift register variables) to give to the solver
2) If needed, randomly assigns them
*/
class BestBits
{
public:
    ///Reads the best bits, or generates them randomly if "_sr_best_bits_from_file" is true
    BestBits(GrainOfSalt* _grain, const bool _sr_best_bits_from_file, const uint _guess_bits);

    /**
    @brief Test bits to see which is the best.
    @param test_number The number of random tests to do. The higher, the better quality best bits are found.
    @param given_output Optimize for this given output. If not given, then it randomizes for a general output
    */
    void best_bit_test(const uint test_number);

    void reload_best_bits(); ///Reloads the best bits file (or, if not a file, then regenerates them randomly)

    ///used for multi-threading the best-bit calculation
    class CalcBestBits
    {
    public:
        CalcBestBits(GrainOfSalt& _grain, const DataElement& _v, uint& _cumulated_solver_difficulty, const uint _test_per_thread) :
                grain(_grain)
                , v(_v)
                , cumulated_solver_difficulty(_cumulated_solver_difficulty)
                , test_per_thread(_test_per_thread)
                {};
        void operator()();

    private:
        GrainOfSalt& grain;
        const DataElement& v;
        uint& cumulated_solver_difficulty; ///<The cumulated difficulty (as measured by the Solver's own difficulty metric function). It is just a reference, as all threads share this value
        const uint test_per_thread; ///<Number of tests needed per thread
    };

    vector<DataElement> varstoguess; ///<The best bits are stored here, in order from best to worst
    bool only_solver_difficulty; ///<Only calculate solver difficulty: do not actually execute the SAT-solver

private:
    static bs::path get_best_bit_dir(); ///<Returns the directory where the best bits files are located
    static bs::path get_best_bit_filename(); ///<Returns the name of the file that stores the best bits

    vector<DataElement> read_file(const bs::path& filename, const uint max_size) const; ///<Parse up a best-bits file
    void write_best_bits_file() const; ///<Write the best bits that have been calculated and are in the memory to the correct file
    bool var_in_impossible_position(const DataElement& v) const; ///<returns true if a variable in the best-bits is in a position that is impossible

    const bool sr_best_bits_from_file; ///<True if the best bits should be read from a file. If false, the best bits are randomly picked
    uint guess_bits; ///<The number of bits to guess from the best bits
    GrainOfSalt* grain;
};

#endif
