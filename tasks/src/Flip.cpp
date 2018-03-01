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
#include <cstring>
#ifdef __unix
#include <stdint.h>
#endif
#include "processlib/ProcessExceptions.h"
#include "processlib/Flip.h"
#include "processlib/Stat.h"
using namespace Tasks;

Flip::Flip() : _mode(FLIP_NONE) {}

Flip::Flip(const Flip &aFlip) : 
  LinkTask(aFlip),_mode(aFlip._mode) 
{}

static void _flip_y_inplace(Data &aSrcData)
{
  int lineSize = aSrcData.dimensions[0] * aSrcData.depth();
  char *aSrcPt = (char*)aSrcData.data();
  char *aDestPt = (char*)aSrcData.data();
  aDestPt += aSrcData.size() - lineSize;

  Buffer *aTmpLineBuffer = new Buffer(lineSize);
  for(int aNbLine = aSrcData.dimensions[1] / 2;aNbLine;
      --aNbLine,aDestPt -= lineSize,aSrcPt += lineSize)
    {
      memcpy(aTmpLineBuffer->data,aDestPt,lineSize);
      memcpy(aDestPt,aSrcPt,lineSize);
      memcpy(aSrcPt,aTmpLineBuffer->data,lineSize);
    }
  aTmpLineBuffer->unref();
}

static void _flip_y(Data &aSrcData,Data &aDestData)
{
  int lineSize = aSrcData.dimensions[0] * aSrcData.depth();
  char *aSrcPt = (char*)aSrcData.data();
  char *aDestPt = (char*)aDestData.data();
  aDestPt += aSrcData.size() - lineSize;
  
  for(int aNbLine = aSrcData.dimensions[1];aNbLine;
      --aNbLine,aDestPt -= lineSize,aSrcPt += lineSize)
    memcpy(aDestPt,aSrcPt,lineSize);
}

template<class INPUT>
inline static void _flip_x_inplace_template(INPUT *aSrcPt,int width, int height)
{
  INPUT *aDestPt = aSrcPt + width - 1;
  for(int aNbLine = height;aNbLine;
      --aNbLine,aDestPt += width + width / 2,aSrcPt += width - width / 2)
    for(int aNbColumn = width / 2 ;aNbColumn;--aNbColumn,--aDestPt,++aSrcPt)
      {
	INPUT aTmpValue = *aDestPt;
	*aDestPt = *aSrcPt;
	*aSrcPt = aTmpValue;
      }
}
#ifndef __unix
#if _MSC_VER < 1900
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t; 
typedef unsigned long long uint64_t; 
#endif
#endif
static void _flip_x_inplace(Data &aSrcData)
{
   switch(aSrcData.depth())
    {
    case 1:
      _flip_x_inplace_template((uint8_t*)aSrcData.data(),
			       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 2:
      _flip_x_inplace_template((uint16_t*)aSrcData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 4:
      _flip_x_inplace_template((uint32_t*)aSrcData.data(),
			       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 8:
      _flip_x_inplace_template((uint64_t*)aSrcData.data(),
			       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    }
}
template<class INPUT>
inline static void _flip_x_template(const INPUT *aSrcPt,INPUT *aDestPt,int width, int height)
{
  aDestPt += width - 1;
  for(int aNbLine = height;aNbLine;--aNbLine,aDestPt += (2 * width))
    for(int aNbColumn = width;aNbColumn;--aNbColumn,--aDestPt,++aSrcPt)
      *aDestPt = *aSrcPt;

}
static void _flip_x(Data &aSrcData,Data &aDestData)
{
  switch(aSrcData.depth())
    {
    case 1:
      _flip_x_template((const uint8_t*)aSrcData.data(),(uint8_t*)aDestData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 2:
      _flip_x_template((const uint16_t*)aSrcData.data(),(uint16_t*)aDestData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 4:
      _flip_x_template((const uint32_t*)aSrcData.data(),(uint32_t*)aDestData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 8:
      _flip_x_template((const uint64_t*)aSrcData.data(),(uint64_t*)aDestData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    }
}

template<class INPUT>
inline static void _flip_all_inplace_template(INPUT *aSrcPt,int width, int height)
{
  INPUT *aDestPt = aSrcPt + width * height - 1;
  for(int aNbPixel = (height * width) / 2;aNbPixel;
      --aNbPixel,--aDestPt,++aSrcPt)
    {
      INPUT aPixelBuff = *aDestPt;
      *aDestPt = *aSrcPt;
      *aSrcPt = aPixelBuff;
    }
}

static void _flip_all_inplace(Data &aSrcData)
{
   switch(aSrcData.depth())
    {
    case 1:
      _flip_all_inplace_template((uint8_t*)aSrcData.data(),
				 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 2:
      _flip_all_inplace_template((uint16_t*)aSrcData.data(),
				 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 4:
      _flip_all_inplace_template((uint32_t*)aSrcData.data(),
				 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 8:
      _flip_all_inplace_template((uint64_t*)aSrcData.data(),
				 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    }
}
template<class INPUT>
inline static void _flip_all_template(const INPUT *aSrcPt,INPUT *aDestPt,int width, int height)
{
  aDestPt += width * height - 1;
  for(int aNbPixel = height * width;aNbPixel;
      --aNbPixel,--aDestPt,++aSrcPt)
      *aDestPt = *aSrcPt;
}
static void _flip_all(Data &aSrcData,Data &aDestData)
{
  switch(aSrcData.depth())
    {
    case 1:
      _flip_all_template((const uint8_t*)aSrcData.data(),(uint8_t*)aDestData.data(),
			 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 2:
      _flip_all_template((const uint16_t*)aSrcData.data(),(uint16_t*)aDestData.data(),
			 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 4:
      _flip_all_template((const uint32_t*)aSrcData.data(),(uint32_t*)aDestData.data(),
			 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 8:
      _flip_all_template((const uint64_t*)aSrcData.data(),(uint64_t*)aDestData.data(),
			 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    }
}

Data Flip::process(Data &aData)
{
  Data aNewData;
  if(aData.dimensions.size() != 2)
    throw ProcessException("Flip : Only manage 2D data");
  else if(_processingInPlaceFlag)
    {
	switch(_mode)	{
		case FLIP_X:
		{
			Stat aStat(aData,"Flip X");
			_flip_x_inplace(aData);
		}
		break;
	
		case FLIP_Y:
		{
			Stat aStat(aData,"Flip Y");
			_flip_y_inplace(aData);
		}
		break;
	
		case FLIP_ALL:
	  {
	    Stat aStat(aData,"Flip X&Y");
	    _flip_all_inplace(aData);
	  }
	  break;
	
		default:
	  break;
	  } //sw
      aNewData = aData;
    } //if
  else
    {
      aNewData = aData.copyHeader(aData.type);
      switch(_mode)
	{
	case FLIP_X:
	  {
	    Stat aStat(aData,"Flip X");
	    _flip_x(aData,aNewData);
	  }
	  break;

	case FLIP_Y:
	  {
	    Stat aStat(aData,"Flip Y");
	    _flip_y(aData,aNewData);
	  }
	  break;
	
	case FLIP_ALL:
	  {
	    Stat aStat(aData,"Flip X&Y");
	    _flip_all(aData,aNewData);
	  }
	  break;
	
	default:
	  memcpy(aNewData.data(),aData.data(),aData.size());
	  break;
	  }
    }
  return aNewData;
}

void Flip::setFlip(FLIP_MODE aMode)
{
  _mode = aMode;
}
