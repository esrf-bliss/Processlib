//###########################################################################
// This file is part of ProcessLib, a submodule of LImA project the
// Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
#include <iostream>
#include <math.h>
#include "FlatfieldCorrection.h"
using namespace Tasks;

template<class INPUT>
void _div(void *src,int aSize,void *flatfield)
{
  INPUT *aSrcPt,*aFFieldPt;
  aSrcPt = (INPUT*)src;
  aFFieldPt = (INPUT*)flatfield;
  for(int i = aSize / sizeof(INPUT);i;--i,++aSrcPt,++aFFieldPt)
    *aSrcPt /= *aFFieldPt;
}

FlatfieldCorrection::FlatfieldCorrection() : 
LinkTask(),
_xcenter(NAN),_ycenter(NAN),
_lambda(NAN),_distance(NAN),
_flatFieldCorrectionDirty(false) {}

FlatfieldCorrection::FlatfieldCorrection(const FlatfieldCorrection &aFlatField) :
  LinkTask(aFlatField),
  _flatFieldImage(aFlatField._flatFieldImage),
  _xcenter(aFlatField._xcenter),_ycenter(aFlatField._ycenter),
  _lambda(aFlatField._lambda),_distance(aFlatField._distance),
  _flatFieldCorrectionDirty(aFlatField._flatFieldCorrectionDirty)
{
}
/** @brief set en external flafield correction array.
 *  You can use this function to set all the parameters or
 *  use setXCenter,setYCenter,setLambda,setDetectorDistance
 *  @param aData the flatfield array
 */
void FlatfieldCorrection::setFlatFieldImageData(Data &aData)
{
  _flatFieldImage.setData(aData);
  _flatFieldCorrectionDirty = false;
  _xcenter = NAN,_ycenter = NAN;
  _lambda = NAN,_distance= NAN;
}

/** @brief set the x center of the beam
 */
void FlatfieldCorrection::setXCenter(double aXCenter)
{
  _xcenter = aXCenter;
  _flatFieldCorrectionDirty = true;
}
/** @brief set the y center of the beam
 */
void FlatfieldCorrection::setYCenter(double aYcenter)
{
  _ycenter = aYcenter;
  _flatFieldCorrectionDirty = true;
}
/** @brief set beam's lambda
 */
void FlatfieldCorrection::setLambda(double aLambda)
{
  _lambda = aLambda;
  _flatFieldCorrectionDirty = true;
}
/** @brief set detector distance
 */
void FlatfieldCorrection::setDetectorDistance(double aDistance)
{
  _distance = aDistance;
  _flatFieldCorrectionDirty = true;
}
/** @brief calc the flatfield correction array
 */
void FlatfieldCorrection::_calcFlatFieldImage(int,int,Data::TYPE)
{
  _flatFieldCorrectionDirty = false;
				// TODO calc correction array
#ifdef __unix
#warning "_calcFlatFieldImage is not yet done -> can't use setDetectorDistance,setLambda,setYCenter,setXCenter"
#else
#pragma message ( "_calcFlatFieldImage is not yet done -> can't use setDetectorDistance,setLambda,setYCenter,setXCenter" )
#endif
}
Data FlatfieldCorrection::process(Data &aData)
{
  if(aData.type == _flatFieldImage.type &&
     aData.height == _flatFieldImage.height &&
     aData.width == _flatFieldImage.width)
    {
      switch(aData.type)
	{
	case Data::UINT8:
	  _div<unsigned char>(aData.data(),aData.size(),
			      _flatFieldImage.data());break;
	case Data::UINT16:
	  _div<unsigned short>(aData.data(),aData.size(),
			       _flatFieldImage.data());break;
	case Data::UINT32:
	  _div<unsigned int>(aData.data(),aData.size(),
			     _flatFieldImage.data());break;
	default: break;
	}
    }
  else
    {
      if(_flatFieldCorrectionDirty)
	{
	  _calcFlatFieldImage(aData.width,aData.height,aData.type);
	  return process(aData);
	}
      else
	std::cerr << "FlatfieldCorrection : Source image differ from flatfield array" << std::endl;
    }
  return aData;
}
