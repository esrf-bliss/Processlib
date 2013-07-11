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
#include <math.h>
#include "ProcessExceptions.h"
#include "FlatfieldCorrection.h"
using namespace Tasks;

template<class INPUT>
void _div(void *src,int aSize,void *flatfield)
{
  INPUT *aSrcPt;
  float *aFFieldPt;
  aSrcPt = (INPUT*)src;
  aFFieldPt = (float*)flatfield;
  for(int i = aSize / sizeof(INPUT);i;--i,++aSrcPt,++aFFieldPt)
    {
      if(*aFFieldPt > 1e-6)
	*aSrcPt = INPUT((float(*aSrcPt) / *aFFieldPt));
      else
	*aSrcPt = INPUT(0);	// @todo check what we should do
    }
}

template<class INPUT>
Data _norm(Data &src)
{
  double sum = 0.;
  int nbItems = 1;
  for(std::vector<int>::const_iterator i = src.dimensions.begin();
      i != src.dimensions.end();++i)
    nbItems *= *i;

  int i = nbItems;
  for(INPUT *aPt = (INPUT*)src.data();i;--i,++aPt)
    sum += *aPt;
  
  double mean = sum / nbItems;

  Data aReturnFF;
  if(src.type == Data::FLOAT && fabs(mean - 1.) < 1e-6)	// FF already normed
    aReturnFF = src;
  else if(mean > 0.)
    {
      aReturnFF = src.copyHeader(Data::FLOAT);
      i = nbItems;
      float *aFFPt = (float*)aReturnFF.data();
      for(INPUT *aPt = (INPUT*)src.data();i;--i,++aPt,++aFFPt)
	*aFFPt = float(float(*aPt) / mean) ;
    }
  else
    throw ProcessException("FlatfieldCorrection : Flatfield data mean is 0. !!!");
  return aReturnFF;
}

static Data _normalize(Data &src)
{
  Data aReturnData;
  switch(src.type)
    {
    case Data::UINT8: 	aReturnData = _norm<unsigned char>(src);break;
    case Data::INT8:	aReturnData = _norm<char>(src);break;
    case Data::UINT16: 	aReturnData = _norm<unsigned short>(src);break;
    case Data::INT16: 	aReturnData = _norm<short>(src);break;
    case Data::UINT32: 	aReturnData = _norm<unsigned int>(src);break;
    case Data::INT32: 	aReturnData = _norm<int>(src);break;
    case Data::UINT64: 	aReturnData = _norm<unsigned long long>(src);break;
    case Data::INT64: 	aReturnData = _norm<long long>(src);break;
    case Data::FLOAT: 	aReturnData = _norm<float>(src);break;
    default:
      throw ProcessException("FlatfieldCorrection : flatfield array type not managed");
      break;
    }
  return aReturnData;
}

FlatfieldCorrection::FlatfieldCorrection() : 
LinkTask()
{}

FlatfieldCorrection::FlatfieldCorrection(const FlatfieldCorrection &aFlatField) :
  LinkTask(aFlatField),
  _flatFieldImage(aFlatField._flatFieldImage)
{
}
/** @brief set en external flafield correction array.
 *  @param aData the flatfield array
 *  @param normalize if true normalize the flatfield if it's not
 */
void FlatfieldCorrection::setFlatFieldImageData(Data &aData,bool normalize)
{
  Data FF;
  if(!aData.empty())
    {
      if(normalize)
	FF = _normalize(aData);
      else if(aData.buffer->owner == Buffer::MAPPED)
	FF = aData.copy();
      else
	FF = aData;
    }

  _flatFieldImage.setData(FF);
}

Data FlatfieldCorrection::process(Data &aData)
{
  if(aData.dimensions == _flatFieldImage.dimensions)
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
	case Data::INT32:
	  _div<int>(aData.data(),aData.size(),
		    _flatFieldImage.data());break;
	default:
	  throw ProcessException("FlatfieldCorrection : data type not yet managed");
	  break;
	}
    }
  else
    throw ProcessException("FlatfieldCorrection : Source image differ from flatfield array");
  return aData;
}
