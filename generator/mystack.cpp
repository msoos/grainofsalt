//
// C++ Implementation: mystack
//
// Description:
//
//
// Author: Mate SOOS <soos@inrialpes.fr>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "mystack.h"
#include "malloc.h"
#include "cipherdesc.h"

mpz_t& MonoStack::pop(const bool clear)
{
    static __thread uint stack_size;
    stack_size = real_stack.size();
    if (stack_size == 0) {
        const uint enlarge = 100000;

        real_stack.resize(enlarge);
        mpz_t* a = (mpz_t*)malloc(sizeof(mpz_t)*enlarge);
        mpz_array_init(*a, enlarge, cpd.vars.get_last_noninternal_var()+1);
        for (uint i = 0; i < enlarge; i++)
            real_stack[i] = a + i;

        stack_size = enlarge;
    }
    static __thread mpz_t* ret;
    ret = real_stack[stack_size-1];
    real_stack.pop_back();
    if (clear) mpz_set_ui(*ret, 0);
    return *ret;
}

