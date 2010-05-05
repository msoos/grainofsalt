//
// C++ Interface: mystack
//
// Description:
//
//
// Author: Mate SOOS <soos@inrialpes.fr>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MYSTACK_H
#define MYSTACK_H

/**
	@author Mate SOOS <soos@inrialpes.fr>
*/

#include <vector>
#include <stack>
#include "gmp.h"
#include <sys/types.h>
#include <boost/thread/thread.hpp>

using std::vector;

class MonoStack
{
public:
    mpz_t& pop(const bool clear = true);
    inline void push(mpz_t& f) {
        real_stack.push_back(&f);
    }

private:
    vector<mpz_t*> real_stack;
};

#endif
