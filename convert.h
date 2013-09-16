/**
 * @file convert.h
 * @author The VLE Development Team
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment (http://vle.univ-littoral.fr)
 * Copyright (C) 2003 - 2008 The VLE Development Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include <iostream>
#include "mex.h"
#include <vle/oov/OutputMatrix.hpp>
//#include <vle/manager/OutputSimulationMatrix.hpp> TODO when OutputSimulationMatrix

#ifndef VLE_MATLABPACKAGE_CONVERT_H
#define VLE_MATLABPACKAGE_CONVERT_H


/*extern declaration is needed to compile C code */
#ifdef __cplusplus
extern "C" {
#endif




/*********************
 *  vle => mxArray
 *********************/

//
//for a cell matrix format required output
//
mxArray* outputMatrixToMxCellMatrix(const vle::oov::OutputMatrix& om);

//
//for a double matrix format required output
//
mxArray* outputMatrixToMxDoubleMatrix(const vle::oov::OutputMatrix& om);

//
//for a simple run
//
//mxArray* outputMatrixViewListToMxArray(const vle::oov::OutputMatrixViewList& lst, const char* return_type);
void outputMatrixViewListToMxCellArray(mxArray* mxOutputMatrixList,const vle::oov::OutputMatrixViewList& lst, const char* return_type);
//
//for replicates runs
// TODO when OutputSimulationMatrix
//mxArray* outputSimulationMatrixToMxArray(const vle::manager::OutputSimulationMatrix& lstMatrix, const char* return_type);



#ifdef __cplusplus
}
#endif

#endif

