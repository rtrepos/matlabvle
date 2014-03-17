/**
 * @file convert.cpp
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



#include "convert.h"
#include <iostream>
#if !(defined _WIN32 || defined _WIN64) // needed to compile on linux.
#include <string.h>
#endif

/*********************
 *  vle => mxArray
 *********************/

//
//for a cell matrix format required output
//
mxArray* outputMatrixToMxCellMatrix(const vle::oov::OutputMatrix& om)
{
	vle::value::ConstMatrixView view(om.values());
	//Initialisation of return (list of OutputMatrix : line*col matrix)

	int nbLines = view.shape()[0];
	int nbCols = view.shape()[1]+1;

	mxArray* mxOutputMatrix = mxCreateCellMatrix(nbCols,nbLines);//WARNING note inverse lines and columns (!!)

	mxArray* cellValue;
	
	
    /*mexPrintf("Converting results to Cell array\n");
    std::cout << "Nb Cols " << nbCols << std::endl;
    std::cout << "Nb lines " << nbLines << std::endl;*/
    
	//Names in first line
	mxSetCell(mxOutputMatrix,0,mxCreateString("time"));
	const vle::oov::OutputMatrix::MapPairIndex& index(om.index());
	int n = 1;
	for ( vle::oov::OutputMatrix::MapPairIndex::const_iterator it = index.begin();
	it != index.end(); ++it,n++ ) {
		
		cellValue = mxCreateString(boost::str(boost::format("%1%.%2%") %
				it->first.first %
				it->first.second).c_str());
		mxSetCell(mxOutputMatrix, (n)*nbCols,cellValue);
		
		/*std::cout << "Port " << it->first.second << std::endl;
		std::cout << "Column " << n  << ", name: " << mxArrayToString(cellValue) << std::endl;
		std::cout << "Cell index " << (n)*nbCols  << ", name: " << mxArrayToString(cellValue) << std::endl;*/
	}
	//Cell data
	for(int i = 0; i<nbLines ; i++){
		for(int j = 0; j<nbCols-1 ; j++){
			if(om.getValue(i)[j] == NULL){
				cellValue = mxCreateString(std::string("NA").c_str());//TODO find a null cell value
			}else{
				switch(om.getValue(i)[j]->getType()) {
				case vle::value::Value::BOOLEAN:
					cellValue = mxCreateDoubleScalar(vle::value::toBoolean(om.getValue(i)[j]));
					break;
				case vle::value::Value::DOUBLE:
					cellValue = mxCreateDoubleScalar(vle::value::toDouble(om.getValue(i)[j]));
					break;
				case vle::value::Value::INTEGER:
					cellValue = mxCreateDoubleScalar(vle::value::toInteger(om.getValue(i)[j]));
					break;
				case vle::value::Value::STRING:
					cellValue = mxCreateString(vle::value::toString(om.getValue(i)[j]).c_str());
					break;
				default:
					//mexErrMsgTxt("\n mvle : outputMatrixToMxArray : error while converting output simulations (Value null)");//Libraries linkage issue, not managed mexErrMsgTxt
					mexPrintf("\n matlabvle : ERROR  while converting output simulations (unknown Value type)");
					return NULL;
				}
			}
			mxSetCell(mxOutputMatrix, (i)*nbCols+j+1,cellValue);
		}
	}
	return mxOutputMatrix;
}

//
//for a double matrix format required output
//
mxArray* outputMatrixToMxDoubleMatrix(const vle::oov::OutputMatrix& om)
{
	vle::value::ConstMatrixView view(om.values());
	//Initialisation of return (list of OutputMatrix : line*col matrix)
	int nbLines = view.shape()[0];
	int nbCols = view.shape()[1];
	mxArray* mxOutputMatrix = mxCreateDoubleMatrix(nbCols,nbLines,mxREAL);//WARNING note inverse lines and columns (!!)
	double* mxOutputMatrixTab = (double *)mxGetPr(mxOutputMatrix);
	//create  data
	for(int i = 0; i<nbLines ; i++){
		for(int j = 0; j<nbCols ; j++){
			if (view[i][j]) {
				switch(om.getValue(i)[j]->getType()) {
				case vle::value::Value::BOOLEAN:
					mxOutputMatrixTab[i*nbCols + j] =(double)vle::value::toBoolean(om.getValue(i)[j]);
					break;
				case vle::value::Value::DOUBLE:
					mxOutputMatrixTab[i*nbCols + j] =vle::value::toDouble(om.getValue(i)[j]);
					break;
				case vle::value::Value::INTEGER:
					mxOutputMatrixTab[i*nbCols + j] =(double)vle::value::toInteger(om.getValue(i)[j]);
					break;
				case vle::value::Value::STRING:
					mxOutputMatrixTab[i*nbCols + j] =mxGetNaN();
					break;
				default:
					mxOutputMatrixTab[i*nbCols + j] =mxGetNaN();

				}
			} else {
				mxOutputMatrixTab[i*nbCols + j] =mxGetNaN();
			}
		}
	}
	return mxOutputMatrix;
}



//
//for a simple run
//
//mxArray* outputMatrixViewListToMxArray(const vle::oov::OutputMatrixViewList& lst, const char* return_type)
void outputMatrixViewListToMxCellArray(mxArray* mxOutputMatrixList,const vle::oov::OutputMatrixViewList& lst, const char* return_type)
{
	if(lst.size() < 1){
		mexPrintf("\n matlabvle : WARNING while converting output simulations... \n   ...perhaps storage plugin is not set in vpz output specification \n");
	}
        
        //Cast of inputs
	std::string return_type_str(return_type);
	//iterates on OutputMatrixViewList
	vle::oov::OutputMatrixViewList::const_iterator it;
	int n;
	for (it=lst.begin() , n = 0; it != lst.end(); ++it, ++n) {
		if(return_type_str == "CELL_MATRIX"){
		      mxSetCell(mxOutputMatrixList, n,outputMatrixToMxCellMatrix(it->second));
		}else if(return_type_str == "DOUBLE_MATRIX"){
		      mxSetCell(mxOutputMatrixList, n,outputMatrixToMxDoubleMatrix(it->second));
		}else{
                      // to be added: liste of valid return_type: "CELL_MATRIX", "DOUBLE_MATRIX"
		      mexPrintf("\n matlabvle : ERROR unknown output type %s \n",return_type);
		}
	}
	//return mxOutputMatrixList;
}

// TODO: ??? void outputMatrixViewListToMxStructArray : with column names (fieldnames: names=cellstr,data=results double matrix) !



//
//for replicates runs
//
//TODO when vle 1.1 outputs ok
//mxArray* outputSimulationMatrixToMxArray(const vle::manager::OutputSimulationMatrix& lstMatrix, const char* return_type)
//{
//	//Initialization of return
//	mxArray* mxOutputSimulationMatrix = mxCreateCellMatrix(lstMatrix.shape()[0],lstMatrix.shape()[1]);
//	//vle::manager::OutputSimulationMatrix::index i, j; TODO are they necessary ?
//	//nb replicats : lstMatrix.shape()[0]);
//	//nb combs : lstMatrix.shape()[1]);
//
//	//Iterate on replicates
//	for (unsigned int i = 0; i < lstMatrix.shape()[0]; ++i) {
//		//Iterate on combinations
//		for (unsigned int j = 0; j < lstMatrix.shape()[1]; ++j) {
//			const vle::oov::OutputMatrixViewList& lst((lstMatrix)[i][j]);
//			//Note : SetCell indexes first columns and then lines
//			mxSetCell(mxOutputSimulationMatrix,(j*lstMatrix.shape()[0]+i),outputMatrixViewListToMxArray(lst,return_type));
//		}
//	}
//	return mxOutputSimulationMatrix;
//}
