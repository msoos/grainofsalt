/***************************************************************************
 *   Copyright (C) 2007 by Mate Soos *
 *   soos@inrialpes.fr   *
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
#ifndef OUTPUT_MAKER_H
#define OUTPUT_MAKER_H

#include <string>
#include <vector>
#include <map>
#include "equationholder.h"
#include "data_holder.h"
#include "dataelement.h"

using std::list;

///Generates equations from the data describing the stream cipher
class OutputMaker
{
public:
    OutputMaker(const FuncDataHolder& dataHolder);

    /**
    Puts the equation for time "time" and function type "type" into the EquationHolder specified
    @param eqHolder Where the equation is added
    @param time The time for which the equation is generated
    @param type The type of the laft hand side of the equation. E.g. output, shift register or filter
    */
    void generate(EquationHolder& eqHolder, const uint time, const DataElement& type);

private:
    /**
    creates a polynomial from a function in the CipherDesc
    @param final_poly The finaly function is put into this variable
    @param time The time for which the equation must be generated - since with time the variable numbers change, this is needed
    @param function The function that must be instantiated into final_poly
    */
    void create_poly_from_function(const uint time, Polynomial& final_poly, const FunctionData& function) const;

    const FuncDataHolder& dataHolder; ///The holder of the functions needed to generate the equations
};

#endif
