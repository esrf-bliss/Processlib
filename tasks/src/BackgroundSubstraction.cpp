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
#ifdef __SSE2__
#include <emmintrin.h>
#endif
#include "ProcessExceptions.h"
#include "BackgroundSubstraction.h"
#include "Stat.h"

using namespace Tasks;

template<class INPUT>
static void _substract_in_place(void *src,int aSize,void *background)
{
  INPUT *aSrcPt,*aBackPt;
  aSrcPt = (INPUT*)src;
  aBackPt = (INPUT*)background;
  for(int i = aSize / sizeof(INPUT);i;--i,++aSrcPt,++aBackPt)
    {
      if(*aSrcPt > *aBackPt)
	*aSrcPt -= *aBackPt;
      else
	*aSrcPt = INPUT(0);
    }
}
#ifdef __SSE2__
template<>
void _substract_in_place<unsigned short>(void *source,int aSize,void *background)
{
  unsigned short *srcShort,*backgroundShort;
  srcShort = (unsigned short*)source;
  backgroundShort = (unsigned short*)background;
  int n = aSize >> 1;		// div / 2
  if(!((long)srcShort & 15))	// aligned to 128 bits
    {
      for(;n >= 8;n -= 8)
	{
	  __m128i src,back;
	  src = _mm_loadu_si128((__m128i*)srcShort);
	  back = _mm_loadu_si128((__m128i*)backgroundShort);
	  src = _mm_subs_epu16(src,back);
	  _mm_store_si128((__m128i*)srcShort,src);
	  srcShort += 8,backgroundShort += 8;
	}
    }
  for(;n;--n,++srcShort,++backgroundShort)
    {
      if(*srcShort > *backgroundShort)
	*srcShort -= *backgroundShort;
      else
	*srcShort = 0;
    }
}
#endif

template<class INPUT>
static void _substract(void *src,void *dst,int aSize,void *background)
{
  INPUT *aSrcPt,*aBackPt,*aDestPt;
  aSrcPt = (INPUT*)src;
  aBackPt = (INPUT*)background;
  aDestPt = (INPUT*)dst;
  for(int i = aSize / sizeof(INPUT);i;--i,++aSrcPt,++aBackPt,++aDestPt)
    {
      if(*aSrcPt > *aBackPt)
	*aDestPt = *aSrcPt - *aBackPt;
      else
	*aDestPt = INPUT(0);
    }
}

#ifdef __SSE2__
template<>
void _substract<unsigned short>(void *source,void *dst,int aSize,void *background)
{
  unsigned short *srcShort,*backgroundShort,*destshort;
  srcShort = (unsigned short*)source;
  backgroundShort = (unsigned short*)background;
  destshort = (unsigned short*)dst;
  int n = aSize >> 1;		// div / 2
  if(!((long)srcShort & 15))	// aligned to 128 bits
    {
      for(;n >= 8;n -= 8)
	{
	  __m128i src,back;
	  src = _mm_loadu_si128((__m128i*)srcShort);
	  back = _mm_loadu_si128((__m128i*)backgroundShort);
	  src = _mm_subs_epu16(src,back);
	  _mm_store_si128((__m128i*)destshort,src);
	  srcShort += 8,backgroundShort += 8,destshort += 8;
	}
    }
  for(;n;--n,++srcShort,++backgroundShort,++destshort)
    {
      if(*srcShort > *backgroundShort)
	*destshort = *srcShort - *backgroundShort;
      else
	*destshort = 0;
    }
}
#endif

BackgroundSubstraction::BackgroundSubstraction() : LinkTask()
{
}

BackgroundSubstraction::BackgroundSubstraction(const BackgroundSubstraction &aBackgroundSubstraction) :
  LinkTask(aBackgroundSubstraction),_backgroundImageData(aBackgroundSubstraction._backgroundImageData)
{
}

void BackgroundSubstraction::setBackgroundImageData(Data &aData)
{
  if(aData.buffer && aData.buffer->owner == Buffer::MAPPED)
    {
      Data copiedData = aData.copy();
      _backgroundImageData.setData(copiedData);
    }
  else
    _backgroundImageData.setData(aData);
}

Data BackgroundSubstraction::process(Data &aData) 
{
  if(aData.type == _backgroundImageData.type &&
     aData.dimensions == _backgroundImageData.dimensions)
    {
      Stat aStat(aData,"Background substraction");
      if(_processingInPlaceFlag)
	{
	  switch(aData.type)
	    {
	    case Data::INT8:
	      _substract_in_place<char>(aData.data(),aData.size(),
					 _backgroundImageData.data());break;
	    case Data::UINT8:
	      _substract_in_place<unsigned char>(aData.data(),aData.size(),
						  _backgroundImageData.data());break;
	    case Data::INT16:
	      _substract_in_place<short>(aData.data(),aData.size(),
					  _backgroundImageData.data());break;
	    case Data::UINT16:
	      _substract_in_place<unsigned short>(aData.data(),aData.size(),
						   _backgroundImageData.data());break;
	    case Data::INT32:
	      _substract_in_place<int>(aData.data(),aData.size(),
					_backgroundImageData.data());break;
	    case Data::UINT32:
	      _substract_in_place<unsigned int>(aData.data(),aData.size(),
						 _backgroundImageData.data());break;
	    default: break;
	    }
	  return aData;
	}
      else
	{
	  Data aNewData = aData.copyHeader(aData.type);	// get a new data buffer
	  switch(aData.type)
	    {
	    case Data::INT8:
	      _substract<char>(aData.data(),aNewData.data(),aData.size(),
			       _backgroundImageData.data());break;
	    case Data::UINT8:
	      _substract<unsigned char>(aData.data(),aNewData.data(),aData.size(),
					_backgroundImageData.data());break;
	    case Data::INT16:
	      _substract<short>(aData.data(),aNewData.data(),aData.size(),
				_backgroundImageData.data());break;
	    case Data::UINT16:
	      _substract<unsigned short>(aData.data(),aNewData.data(),aData.size(),
					 _backgroundImageData.data());break;
	    case Data::INT32:
	      _substract<int>(aData.data(),aNewData.data(),aData.size(),
			      _backgroundImageData.data());break;
	    case Data::UINT32:
	      _substract<unsigned int>(aData.data(),aNewData.data(),aData.size(),
				       _backgroundImageData.data());break;
	    default: break;
	    }
	  return aNewData;
	}
    }
  else
    throw ProcessException("BackgroundSubstraction : Source image differ from background image");
  return Data();		// empty result
}
