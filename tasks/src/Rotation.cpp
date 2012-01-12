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
#include "ProcessExceptions.h"
#include "Rotation.h"
#include "Stat.h"
using namespace Tasks;

Rotation::Rotation() :
  LinkTask(),
  m_type(R_90)
{}

Rotation::Rotation(const Rotation& other) :
  LinkTask(other),
  m_type(other.m_type)
{}

void Rotation::setType(Type aType)
{
  m_type = aType;
}

template<class INPUT>
void _rotate_180(Data &aSrc,Data &aDst)
{
  INPUT *aSrcPt = (INPUT*)aSrc.data();
  INPUT *aDstPt = (INPUT*)aDst.data();
  int aNbPixel = aDst.dimensions[0] * aDst.dimensions[1];
  aDstPt += aNbPixel - 1;	// Last Pixel
  for(int i = 0;i < aNbPixel;++i,++aSrcPt,--aDstPt)
    *aDstPt = *aSrcPt;
}

template<class INPUT>
void _rotate_270(Data &aSrc,Data &aDst)
{
  INPUT *aSrcPt = (INPUT*)aSrc.data();
  INPUT *aDstPt = (INPUT*)aDst.data();
  for(int lineId = 0;lineId < aSrc.dimensions[1];++lineId)
    {
      INPUT *aDstColumnPt = aDstPt + ((aSrc.dimensions[0] - 1) * aSrc.dimensions[1]) + lineId;
      for(int columnId = 0;columnId < aSrc.dimensions[0];
	  ++columnId,++aSrcPt,aDstColumnPt -= aSrc.dimensions[1])
	*aDstColumnPt = *aSrcPt;
    }
  aDst.dimensions[0] = aSrc.dimensions[1];
  aDst.dimensions[1] = aSrc.dimensions[0];
}

template<class INPUT>
void _rotate_90(Data &aSrc,Data &aDst)
{
  INPUT *aSrcPt = (INPUT*)aSrc.data();
  INPUT *aDstPt = (INPUT*)aDst.data();
  for(int lineId = 0;lineId < aSrc.dimensions[1];++lineId)
    {
      INPUT *aDstColumnPt = aDstPt + (aSrc.dimensions[1] - 1 - lineId);
      for(int columnId = 0;columnId < aSrc.dimensions[0];
	  ++columnId,++aSrcPt,aDstColumnPt += aSrc.dimensions[1])
	*aDstColumnPt = *aSrcPt;
    }
  aDst.dimensions[0] = aSrc.dimensions[1];
  aDst.dimensions[1] = aSrc.dimensions[0];
}

Data Rotation::process(Data &aData)
{
  Data aNewData;
  if(aData.dimensions.size() != 2)
    throw ProcessException("Rotation : Only manage 2D data");
  else
    {
      Type aType = m_type;
      std::stringstream info;
      info << "Rotation ";
      switch(aType)
	{
	case R_180:
	  info << "180 deg";
	  break;
	case R_270:
	  info << "270 deg";
	  break;
	default :		// Rotation 90
	  aType = R_90;
	  info << "90 deg";
	  break;
	};
      Stat aStat(aData,info.str());
      
      aNewData = aData.copyHeader(aData.type);

      switch(aType)
	{
	case R_180:
	  {
	    switch(aData.type)
	      {
	      case Data::UINT8:
		_rotate_180<unsigned char>(aData,aNewData);
		break;
	      case Data::UINT16:
		_rotate_180<unsigned short>(aData,aNewData);
		break;
	      case Data::UINT32:
		_rotate_180<unsigned int>(aData,aNewData);
		break;
	      case Data::INT32:
		_rotate_180<int>(aData,aNewData);
		break;
	      default:
		throw ProcessException("Rotation : type not managed yet");
	      }
	  }
	  break;
	case R_270:
	  {
	    switch(aData.type)
	      {
	      case Data::UINT8:
		_rotate_270<unsigned char>(aData,aNewData);
		break;
	      case Data::UINT16:
		_rotate_270<unsigned short>(aData,aNewData);
		break;
	      case Data::UINT32:
		_rotate_270<unsigned int>(aData,aNewData);
		break;
	      case Data::INT32:
		_rotate_270<int>(aData,aNewData);
		break;
	      default:
		throw ProcessException("Rotation : type not managed yet");
	      }
	  }
	  break;
	default:
	  {
	    switch(aData.type)
	      {
	      case Data::UINT8:
		_rotate_90<unsigned char>(aData,aNewData);
		break;
	      case Data::UINT16:
		_rotate_90<unsigned short>(aData,aNewData);
		break;
	      case Data::UINT32:
		_rotate_90<unsigned int>(aData,aNewData);
		break;
	      case Data::INT32:
		_rotate_90<int>(aData,aNewData);
		break;
	      default:
		throw ProcessException("Rotation : type not managed yet");
	      }
	  }
	  break;
	}

      if(_processingInPlaceFlag)
	{
	  memcpy(aData.data(),aNewData.data(),aNewData.size());
	  aData.dimensions = aNewData.dimensions;
	}
    }
  return aNewData;
}
