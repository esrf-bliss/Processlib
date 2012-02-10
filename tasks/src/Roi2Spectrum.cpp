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
#include "Roi2Spectrum.h"
using namespace Tasks;

Roi2SpectrumTask::Roi2SpectrumTask(Roi2SpectrumManager &aMgr) :
  SinkTask<Roi2SpectrumResult>(aMgr),
  _x(0),_y(0),
  _width(0),_height(0),
  _mode(Roi2SpectrumTask::LINES_SUM)
{
}

Roi2SpectrumTask::Roi2SpectrumTask(const Roi2SpectrumTask &aTask) :
  SinkTask<Roi2SpectrumResult>(aTask),
  _x(aTask._x),_y(aTask._y),
  _width(aTask._width),_height(aTask._height),
  _mode(aTask._mode)
{
}

template<class INPUT,class OUTPUT> 
static void _sum(const INPUT *aSrcPt,
		 int widthStep,
		 int x,int y,
		 int width,int height,
		 OUTPUT *aDstPt,Roi2SpectrumTask::Mode aMode)
{
  if(aMode == Roi2SpectrumTask::LINES_SUM)
    {
      for(int lineId = y;lineId < y + height;++lineId)
	{
	  const INPUT *aLinePt = aSrcPt + lineId * widthStep + x;
	  OUTPUT *dst = aDstPt;
	  for(int i = 0;i < width;++i,++aLinePt,++dst)
	    *dst += OUTPUT(*aLinePt);
	}
    }
  else
    {
      OUTPUT *dst = aDstPt;
      for(int lineId = y;lineId < y + height;++lineId,++dst)
	{
	  const INPUT *aLinePt = aSrcPt + lineId * widthStep + x;
	  for(int i = 0;i < width;++i,++aLinePt)
	    *dst += OUTPUT(*aLinePt);
	}
    }
}

void Roi2SpectrumTask::process(Data &aData)
{
  Roi2SpectrumResult aResult;
  aResult.frameNumber = aData.frameNumber;
  if(aData.dimensions.size() != 2)
    throw ProcessException("Roi2SpectrumResult : Only manage 2D data");

  if(_width > 0 && _height > 0)
    {

      switch(aData.type)
	{
	case Data::UINT8:
	case Data::INT8:
	case Data::UINT16:
	case Data::INT16:
	case Data::UINT32:
	case Data::INT32:
	  aResult.spectrum.type = Data::INT32;
	  break; 
	case Data::FLOAT:
	case Data::DOUBLE:
	  aResult.spectrum.type = Data::DOUBLE;
	  break;
	default:
	  aResult.errorCode = Roi2SpectrumManager::NOT_MANAGED;
	  throw ProcessException("Roi2SpectrumTask : type not yet managed");
	}

      aResult.spectrum.dimensions.push_back((_mode == LINES_SUM) ? _width : _height);
      int aSize = aResult.spectrum.size();
      Buffer *aBufferPt = new Buffer(aSize);
      memset(aBufferPt->data,0,aSize);
      aResult.spectrum.setBuffer(aBufferPt);
      aBufferPt->unref();

      switch(aData.type)
	{
	case Data::UINT8:
	  _sum((unsigned char*)aData.data(),
	       aData.dimensions[0],
	       _x,_y,_width,_height,
	       (int*)aResult.spectrum.data(),_mode);
	  break;
	case Data::INT8:
	  _sum((char*)aData.data(),
	       aData.dimensions[0],
	       _x,_y,_width,_height,
	       (int*)aResult.spectrum.data(),_mode);
	  break;
	case Data::UINT16:
	  _sum((unsigned short*)aData.data(),
	       aData.dimensions[0],
	       _x,_y,_width,_height,
	       (int*)aResult.spectrum.data(),_mode);
	  break;
	case Data::INT16:
	  _sum((short*)aData.data(),
	       aData.dimensions[0],
	       _x,_y,_width,_height,
	       (int*)aResult.spectrum.data(),_mode);
	  break;
	case Data::UINT32:
	  _sum((unsigned int*)aData.data(),
	       aData.dimensions[0],
	       _x,_y,_width,_height,
	       (int*)aResult.spectrum.data(),_mode);
	  break;
	case Data::INT32:
	  _sum((int*)aData.data(),
	       aData.dimensions[0],
	       _x,_y,_width,_height,
	       (int*)aResult.spectrum.data(),_mode);
	  break;
	case Data::FLOAT:
	  _sum((float*)aData.data(),
	       aData.dimensions[0],
	       _x,_y,_width,_height,
	       (double*)aResult.spectrum.data(),_mode);
	  break;

	case Data::DOUBLE:
	  _sum((float*)aData.data(),
	       aData.dimensions[0],
	       _x,_y,_width,_height,
	       (double*)aResult.spectrum.data(),_mode);
	  break;
	default:
	  break;
	}
    }

  _mgr.setResult(aResult);
}

#ifndef __unix
extern "C"
{
  static void _impl_bpm()
  {
    Roi2SpectrumManager *roi2SpectrumMgr = new Roi2SpectrumManager();

    roi2SpectrumMgr->setMode(Roi2SpectrumManager::Counter);
    roi2SpectrumMgr->getResult();
    std::list<Roi2SpectrumResult> aResult;
    roi2SpectrumMgr->getHistory(aResult);
    roi2SpectrumMgr->resizeHistory(10);
    roi2SpectrumMgr->resetHistory();
    roi2SpectrumMgr->historySize();
    roi2SpectrumMgr->lastFrameNumber();
    roi2SpectrumMgr->ref();
    roi2SpectrumMgr->unref();

    Roi2SpectrumTask *roiCounterTask = new Roi2SpectrumTask(*roi2SpectrumMgr);
    roiCounterTask->ref();
    roiCounterTask->unref();
  }
}
#endif
