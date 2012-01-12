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
#include <stdio.h>
#include "ProcessExceptions.h"
#include "Mask.h"
using namespace Tasks;

Mask::Mask() : LinkTask() {}

Mask::Mask(const Mask &aTask) :
  LinkTask(aTask),
  _MaskImage(aTask._MaskImage)
{}

void Mask::setMaskImageData(Data &aMaskImage)
{
  _MaskImage.setData(aMaskImage);
}

template<class INPUT,class MASK>
static void _mask_inplace(INPUT *src,int aSize,MASK *mask)
{
  for(int i = aSize / sizeof(INPUT);i;--i,++src,++mask)
    if(*mask) *src = INPUT(*mask);
}

template<class INPUT,class MASK>
static void _mask(INPUT *src,INPUT *dst,int aSize,MASK *mask)
{
  for(int i = aSize / sizeof(INPUT);i;--i,++src,++dst,++mask)
    {
      if(*mask) 
	*dst = INPUT(*mask);
      else
	*dst = *src;
    }
}

Data Mask::process(Data &aData)
{
  const char *errorMsgPt = NULL;
  if(aData.dimensions == _MaskImage.dimensions)
    {
      if(_processingInPlaceFlag)
	{
	  switch(aData.type)
	    {
	    case Data::UINT8:
	      if(aData.type == _MaskImage.type)
		_mask_inplace((unsigned char*)aData.data(),aData.size(),
			      (unsigned char*)_MaskImage.data());
	      else
		{
		  errorMsgPt = "mask image must be an unsigned char";
		  goto error;
		}
	      break;
	    case Data::UINT16:
	      switch(_MaskImage.type)
		{
		case Data::UINT8:
		  _mask_inplace((unsigned short*)aData.data(),aData.size(),
				(unsigned char*)_MaskImage.data());
		  break;
		case Data::UINT16:
		  _mask_inplace((unsigned short*)aData.data(),aData.size(),
				(unsigned short*)_MaskImage.data());
		  break;
		default:
		  errorMsgPt = "mask image must be an unsigned char or unsigned short";
		  goto error;
		}
	      break;
	    case Data::UINT32:
	      switch(_MaskImage.type)
		{
		case Data::UINT8:
		  _mask_inplace((unsigned int*)aData.data(),aData.size(),
				(unsigned char*)_MaskImage.data());
		  break;
		case Data::UINT16:
		  _mask_inplace((unsigned int*)aData.data(),aData.size(),
				(unsigned short*)_MaskImage.data());
		  break;
		case Data::UINT32:
		  _mask_inplace((unsigned int*)aData.data(),aData.size(),
				(unsigned int*)_MaskImage.data());
		  break;
		default:
		  errorMsgPt = "mask image must be an unsigned char,unsigned short or unsigned int";
		  goto error;
		}
	      break;
	    default: 
	      errorMsgPt = "Data type not managed";
	      goto error;
	    }
	  return aData;
	}
      else
	{
	  Data aNewData = aData.copyHeader(aData.type);	// get a new data buffer
	  switch(aData.type)
	    {
	    case Data::UINT8:
	      if(aData.type == _MaskImage.type)
		_mask((unsigned char*)aData.data(),(unsigned char*)aNewData.data(),
		      aData.size(),(unsigned char*)_MaskImage.data());
	      else
		{
		  errorMsgPt = "mask image must be an unsigned char";
		  goto error;
		}
	      break;
	    case Data::UINT16:
	      switch(_MaskImage.type)
		{
		case Data::UINT8:
		  _mask((unsigned short*)aData.data(),(unsigned short*)aNewData.data(),
			aData.size(),(unsigned char*)_MaskImage.data());
		  break;
		case Data::UINT16:
		  _mask((unsigned short*)aData.data(),(unsigned short*)aNewData.data(),
			aData.size(),
			(unsigned short*)_MaskImage.data());
		  break;
		default:
		  errorMsgPt = "mask image must be an unsigned char or unsigned short";
		  goto error;
		}
	      break;
	    case Data::UINT32:
	      switch(_MaskImage.type)
		{
		case Data::UINT8:
		  _mask((unsigned int*)aData.data(),(unsigned int*)aNewData.data(),
			aData.size(),
			(unsigned char*)_MaskImage.data());
		  break;
		case Data::UINT16:
		  _mask((unsigned int*)aData.data(),(unsigned int*)aNewData.data(),
			aData.size(),
			(unsigned short*)_MaskImage.data());
		  break;
		case Data::UINT32:
		  _mask((unsigned int*)aData.data(),(unsigned int*)aNewData.data(),
			aData.size(),
			(unsigned int*)_MaskImage.data());
		  break;
		default:
		  errorMsgPt = "mask image must be an unsigned char,unsigned short or unsigned int";
		  goto error;
		}
	      break;
	    default: 
	      errorMsgPt = "Data type not managed";
	      goto error;
	    }
	  return aNewData;
	}
    }
  else
    errorMsgPt = "Source image differ from mask image";

 error:
  if(errorMsgPt)
    {
      char aBuffer[256];
      snprintf(aBuffer,sizeof(aBuffer),"Mask : %s",errorMsgPt);
      throw ProcessException(aBuffer);
    }
  return Data();
}
