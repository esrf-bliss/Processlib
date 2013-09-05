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
#include "RoiCounter.h"
using namespace Tasks;
#include <math.h>

RoiCounterTask::RoiCounterTask(RoiCounterManager &aMgr) :
  SinkTask<RoiCounterResult>(aMgr),
  _type(UNDEF),
  _x(0),_y(0),
  _width(0),_height(0)
{
}

RoiCounterTask::RoiCounterTask(const RoiCounterTask &aTask) :
  SinkTask<RoiCounterResult>(aTask),
  _type(aTask._type),
  _x(aTask._x),_y(aTask._y),
  _width(aTask._width),_height(aTask._height),
  _mask(aTask._mask),
  _lut(aTask._lut)
{
}

void RoiCounterTask::setRoi(int x,int y,int width,int height)
{
  _type = SQUARE;
  _x = x,_y = y;
  _width = width,_height = height;
}
void RoiCounterTask::getRoi(int &x,int &y,int &width,int &height)
{
  if(_type != SQUARE)
    throw ProcessException("RoiCounterTask: This is not a SQUARE Roi");
  x = _x,y = _y,width = _width,height = _height;
}

void RoiCounterTask::setLut(int x,int y,Data &lut)
{
  if(lut.dimensions.size() < 1 || lut.dimensions.size() > 2)
    throw ProcessException("RoiCounterTask : Only manage 1 or 2D data");

  _type = LUT;
  _lut = lut.cast(Data::DOUBLE);
  _x = x,_y = y;
}
void RoiCounterTask::getLut(int &x,int &y,Data &lut)
{
  if(_type != LUT)
    throw ProcessException("RoiCounterTask: This is not a LUT Roi");

  x = _x,y = _y,lut = _lut;
}

template<class INPUT> static void _get_average_std(const Data& aData,
						   int x,int y,
						   int width,int height,
						   RoiCounterResult &aResult)
{
  const INPUT *aSrcPt = (const INPUT*)aData.data();
  int widthStep = aData.dimensions[0];
  INPUT aMin,aMax;
  aMin = aMax = *(aSrcPt + y * widthStep + x); // Init
  double aSum = 0.;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      const INPUT *aLinePt = aSrcPt + lineId * widthStep + x;
      for(int i = 0;i < width;++i,++aLinePt)
	{
	  aSum += double(*aLinePt);
	  if(*aLinePt > aMax)
	    aMax = *aLinePt;
	  else if(*aLinePt < aMin)
	    aMin = *aLinePt;
	}
    }

  aResult.sum = aSum;
  aResult.average = aSum / (width * height);
  aResult.minValue = double(aMin);
  aResult.maxValue = double(aMax);

  //STD
  aSum = 0.;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      const INPUT *aLinePt = aSrcPt + lineId * widthStep + x;
      for(int i = 0;i < width;++i,++aLinePt)
	{
	  double diff = *aLinePt - aResult.average;
	  diff *= diff;
	  aSum += diff;
	}
    }
  aSum /= width * height;
  aResult.std = sqrt(aSum);
}

template<class INPUT> static void _get_average_std_with_mask(const Data& aData,
							     const char *aMaskPt,
							     int x,int y,
							     int width,int height,
							     RoiCounterResult &aResult)
{
  const INPUT *aSrcPt = (INPUT*)aData.data();
  int widthStep = aData.dimensions[0];
  INPUT aMin = INPUT(0.),aMax = INPUT(0.);
  //min max init;
  bool continueFlag = true;
  for(int lineId = y;continueFlag && lineId < y + height;++lineId)
    {
      int offset = lineId * widthStep + x;
      const INPUT *aLinePt = aSrcPt + offset;
      const char *aMaskLinePt = aMaskPt + offset;
      for(int i = 0;continueFlag && i < width;++i,++aLinePt,++aMaskLinePt)
	{
	  if(*aMaskLinePt)
	    aMin = aMax = *aLinePt,continueFlag = false;
	}
    }

  double aSum = 0.;
  int usedSize = width * height;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      int offset = lineId * widthStep + x;
      const INPUT *aLinePt = aSrcPt + offset;
      const char *aMaskLinePt = aMaskPt + offset;
      for(int i = 0;i < width;++i,++aLinePt,++aMaskLinePt)
	{
	  if(*aMaskLinePt)
	    aSum += double(*aLinePt);
	  else
	    --usedSize;
	}
    }

  aResult.sum = aSum;
  if(usedSize > 0)
    aResult.average = aSum / usedSize;
  else
    aResult.average = 0.;
  
  //STD
  aSum = 0.;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      int offset = lineId * widthStep + x;
      const INPUT *aLinePt = aSrcPt + offset;
      const char *aMaskLinePt = aMaskPt + offset;
      for(int i = 0;i < width;++i,++aLinePt,++aMaskLinePt)
	{
	  if(*aMaskLinePt) continue;

	  double diff = *aLinePt - aResult.average;
	  diff *= diff;
	  aSum += diff;
	}
    }

  if(usedSize > 0)
    {
      aSum /= usedSize;
      aResult.std = sqrt(aSum);
    }
  else
    aResult.std = 0.;
}

template<class INPUT> static void _lut_get_average_std(const Data& data,
						       int x,int y,Data& lut,
						       RoiCounterResult &aResult)
{
  double *lutPt = (double*)lut.data();
  const INPUT* aSrcPt = (const INPUT*)data.data();
  //basic check
  if(lut.dimensions.size() != data.dimensions.size())
    throw ProcessException("RoiLutCounterResult lut and data must have the same dimension");
  if(x < 0 || y < 0)
    throw ProcessException("RoiLutCounterResult lut origin must be positive");
  if(lut.dimensions[0] + x > data.dimensions[0])
    throw ProcessException("RoiLutCounterResult lut width + origin go outside of the data bounding box");
  if(lut.dimensions.size() > 1 && 
     lut.dimensions[1] + y > data.dimensions[1])
    throw ProcessException("RoiLutCounterResult lut height + origin got outside of the data bounding box");

  
  int widthStep = data.dimensions[0];
  double aMin,aMax;
  //jump to start
  aSrcPt += y * widthStep + x;

  int nbItems = 1;
  for(std::vector<int>::iterator i = lut.dimensions.begin();
      i != lut.dimensions.end();++i)
    nbItems *= *i;
  
  int totalItems = nbItems;
  //min max initialization
  aMin = aMax = 0.;
  double aSum = 0.;
  double aWeight = 0.;
  int cId;
  bool continueFlag = true;
  while(continueFlag && nbItems)
    {
      for(cId = 0;continueFlag && cId < lut.dimensions[0];++cId,--nbItems,++lutPt)
	{
	  if(*lutPt != 0.)
	    {
	      double value = aSrcPt[cId] * *lutPt; 
	      aMax = aMin = value;
	      aSum = value;
	      aWeight = *lutPt;
	      continueFlag = false;
	    }
	}
      if(continueFlag)
	aSrcPt += widthStep;
    }
  
  while(nbItems)
    {
      for(;cId < lut.dimensions[0];++cId,--nbItems,++lutPt)
	{
	  if(*lutPt != 0.)
	    {
	      double value = aSrcPt[cId] * *lutPt;
	      if(value > aMax) aMax = value;
	      else if(value < aMin) aMin = value;
	      aSum += value;
	      aWeight += *lutPt;
	    }
	}
      aSrcPt += widthStep,cId = 0;
    }

  aResult.minValue = aMin;
  aResult.maxValue = aMax;
  aResult.sum = aSum;
  if(aWeight > 0.)
    aResult.average = aSum / aWeight;
  else
    aResult.average = 0.;
  
  lutPt = (double*)lut.data();
  aSrcPt = (const INPUT*)data.data();
  aSrcPt += y * widthStep + x;
  
  nbItems = totalItems;
  aSum = 0.;
  while(nbItems)
    {
      for(int cId = 0;cId < lut.dimensions[0];++cId,--nbItems,++lutPt)
	{
	  if(*lutPt != 0.)
	    {
	      double value = aSrcPt[cId] * *lutPt;
	      double diff = value - aResult.average;
	      diff *= diff;
	      aSum += diff;
	    }
	}
      aSrcPt += widthStep;
    }
  
  if(aWeight != 0.)
    aResult.std = sqrt(aSum / aWeight);
  else
    aResult.std = 0.;
}

#define GET_AVERAGE_STD(TYPE)					\
  {								\
  if(_type == SQUARE)						\
    _get_average_std<TYPE>(aData,_x,_y,_width,_height,aResult);	\
  else if(_type == LUT)						\
    _lut_get_average_std<TYPE>(aData,_x,_y,_lut,aResult);	\
  }

void RoiCounterTask::process(Data &aData)
{
  //_check_roi(_x,_y,_width,_height,aData);
  if(aData.dimensions.size() != 2)
    throw ProcessException("RoiCounterTask : Only manage 2D data");

  RoiCounterResult aResult;
  aResult.frameNumber = aData.frameNumber;
  if(_mask.empty())
    {
      switch(aData.type)
	{
	case Data::UINT8: 
	  GET_AVERAGE_STD(unsigned char);
	  break;
	case Data::INT8:
	  GET_AVERAGE_STD(char);
	  break;
	case Data::UINT16:
	  GET_AVERAGE_STD(unsigned short);
	  break;
	case Data::INT16:
	  GET_AVERAGE_STD(short);
	  break;
	case Data::UINT32:
	  GET_AVERAGE_STD(unsigned int);
	  break;
	case Data::INT32:
	  GET_AVERAGE_STD(int);
	  break;
	case Data::UINT64:
	  GET_AVERAGE_STD(unsigned long long);
	  break;
	case Data::INT64:
	  GET_AVERAGE_STD(long long);
	  break;

	case Data::FLOAT:
	  GET_AVERAGE_STD(float);
	  break;
	case Data::DOUBLE:
	  GET_AVERAGE_STD(double);
	  break;
	default: 
	  break;				// error
	}
    }
  else if(_mask.dimensions == aData.dimensions)
    {
      switch(aData.type)
	{
	case Data::UINT8: 
	  _get_average_std_with_mask<unsigned char>(aData,
						    (char*)_mask.data(),
						    _x,_y,_width,_height,aResult);
	  break;
	case Data::INT8:
	  _get_average_std_with_mask<char>(aData,
					   (char*)_mask.data(),
					   _x,_y,_width,_height,aResult);
	  break;

	case Data::UINT16:
	  _get_average_std_with_mask<unsigned short>(aData,
						     (char*)_mask.data(),
						     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT16:
	  _get_average_std_with_mask<short>(aData,
					    (char*)_mask.data(),
					    _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT32:
	  _get_average_std_with_mask<unsigned int>(aData,
						   (char*)_mask.data(),
						   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT32:
	  _get_average_std_with_mask<int>(aData,
					  (char*)_mask.data(),
					  _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT64:
	  _get_average_std_with_mask<unsigned long long>(aData,
							 (char*)_mask.data(),
							 _x,_y,_width,_height,aResult);
	  break;
	case Data::INT64:
	  _get_average_std_with_mask<long long>(aData,
						(char*)_mask.data(),
						_x,_y,_width,_height,aResult);
	  break;

	case Data::FLOAT:
	  _get_average_std_with_mask<float>(aData,
					    (char*)_mask.data(),
					    _x,_y,_width,_height,aResult);
	  break;
	case Data::DOUBLE:
	  _get_average_std_with_mask<double>(aData,
					     (char*)_mask.data(),
					     _x,_y,_width,_height,aResult);
	  break;
	default: 
	  break;				// error
	}
    }
  else
    throw ProcessException("RoiCounter : Source image size differ from mask");

  _mgr.setResult(aResult);
}


#ifndef __unix
extern "C"
{
  static void _impl_bpm()
  {
    RoiCounterManager *RoiCounterMgr = new RoiCounterManager();

    RoiCounterMgr->setMode(RoiCounterManager::Counter);
    RoiCounterMgr->getResult();
    std::list<RoiCounterResult> aResult;
    RoiCounterMgr->getHistory(aResult);
    RoiCounterMgr->resizeHistory(10);
    RoiCounterMgr->resetHistory();
    RoiCounterMgr->historySize();
    RoiCounterMgr->lastFrameNumber();
    RoiCounterMgr->ref();
    RoiCounterMgr->unref();

    RoiCounterTask *roiCounterTask = new RoiCounterTask(*RoiCounterMgr);
    roiCounterTask->ref();
    roiCounterTask->unref();
  }
}
#endif
