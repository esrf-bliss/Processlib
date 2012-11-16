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
  _x(0),_y(0),
  _width(0),_height(0)
{
}

RoiCounterTask::RoiCounterTask(const RoiCounterTask &aTask) :
  SinkTask<RoiCounterResult>(aTask),
  _x(aTask._x),_y(aTask._y),
  _width(aTask._width),_height(aTask._height)
{
}

template<class INPUT> static void _get_average_std(const INPUT *aSrcPt,
						   int widthStep,
						   int x,int y,
						   int width,int height,
						   RoiCounterResult &aResult)
{
  INPUT aMin,aMax;
  aMin = aMax = *aSrcPt; // Init
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

template<class INPUT> static void _get_average_std_with_mask(const INPUT *aSrcPt,
							     int widthStep,
							     const char *aMaskPt,
							     int x,int y,
							     int width,int height,
							     RoiCounterResult &aResult)
{
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
	  _get_average_std((unsigned char*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT8:
	  _get_average_std((char*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
	  break;

	case Data::UINT16:
	  _get_average_std((unsigned short*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT16:
	  _get_average_std((short*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT32:
	  _get_average_std((unsigned int*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT32:
	  _get_average_std((int*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT64:
	  _get_average_std((unsigned long long*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT64:
	  _get_average_std((long long*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
	  break;

	case Data::FLOAT:
	  _get_average_std((float*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::DOUBLE:
	  _get_average_std((double*)aData.data(),
			   aData.dimensions[0],
			   _x,_y,_width,_height,aResult);
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
	  _get_average_std_with_mask((unsigned char*)aData.data(),
				     aData.dimensions[0],
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT8:
	  _get_average_std_with_mask((char*)aData.data(),
				     aData.dimensions[0],
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;

	case Data::UINT16:
	  _get_average_std_with_mask((unsigned short*)aData.data(),
				     aData.dimensions[0],
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT16:
	  _get_average_std_with_mask((short*)aData.data(),
				     aData.dimensions[0],
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT32:
	  _get_average_std_with_mask((unsigned int*)aData.data(),
				     aData.dimensions[0],
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT32:
	  _get_average_std_with_mask((int*)aData.data(),
				     aData.dimensions[0],
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT64:
	  _get_average_std_with_mask((unsigned long long*)aData.data(),
				     aData.dimensions[0],
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT64:
	  _get_average_std_with_mask((long long*)aData.data(),
				     aData.dimensions[0],
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;

	case Data::FLOAT:
	  _get_average_std_with_mask((float*)aData.data(),
				     aData.dimensions[0],
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::DOUBLE:
	  _get_average_std_with_mask((double*)aData.data(),
				     aData.dimensions[0],
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
