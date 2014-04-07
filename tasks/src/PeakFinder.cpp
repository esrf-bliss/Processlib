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
#include "PeakFinder.h"
using namespace Tasks;
#ifndef __unix
#define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <cstdio>

PeakFinderTask::PeakFinderTask(PeakFinderManager &aMgr) :
  SinkTask<PeakFinderResult>(aMgr),
  _nb_peaks(1)
{
}

PeakFinderTask::PeakFinderTask(const PeakFinderTask &aTask) :
  SinkTask<PeakFinderResult>(aTask),
  _nb_peaks(aTask._nb_peaks),
  _mask(aTask._mask)
{
}

template<class INPUT> static void _compute_peaks(const Data& aData, PeakFinderResult &aResult){

  const INPUT *aSrcPt = (const INPUT*)aData.data();
  INPUT dataMax;
  int x_max, y_max;

  dataMax = 0;
  for(int lineId = 0;lineId < aData.dimensions[1];lineId++){
    for( int x_value = 0; x_value < aData.dimensions[0]; x_value++){
      const INPUT *aLinePt = aSrcPt + lineId *aData.dimensions[0] + x_value;
      if(*aLinePt > dataMax){
	dataMax = *aLinePt;
	x_max = x_value;
	y_max = lineId;
      }
    }
  }

  aResult.x_peak = x_max;
  aResult.y_peak = y_max;
}


void PeakFinderTask::process(Data &aData)
{

  if(aData.dimensions.size() != 2)
    throw ProcessException("PeakFinderTask : Only manage 2D data");

  PeakFinderResult aResult;
  aResult.frameNumber = aData.frameNumber;

  switch(aData.type)
    {
    case Data::UINT8: 
      _compute_peaks<unsigned char>(aData, aResult);
      break;
    case Data::INT8:
      _compute_peaks<char>(aData, aResult);
      break;
      
    case Data::UINT16:
      _compute_peaks<unsigned short>(aData, aResult);
      break;
    case Data::INT16:
      _compute_peaks<short>(aData, aResult);
      break;
    case Data::UINT32:
      _compute_peaks<unsigned int>(aData, aResult);
      break;
    case Data::INT32:
      _compute_peaks<int>(aData, aResult);
      break;
    case Data::UINT64:
      _compute_peaks<unsigned long long>(aData, aResult);
      break;
    case Data::INT64:
      _compute_peaks<long long>(aData, aResult);
      break;
      
    case Data::FLOAT:
      _compute_peaks<float>(aData, aResult);
      break;
    case Data::DOUBLE:
      _compute_peaks<double>(aData, aResult);
      break;
    default: 
      break;				// error
    }

  //  if(_mask.empty()){
  //} else if(_mask.dimensions == aData.dimensions){
  //}else
  //   throw ProcessException("PeakFinder : Source image size differ from mask");

  _mgr.setResult(aResult);
}


#ifndef __unix
extern "C"
{
  static void _impl_bpm()
  {
    PeakFinderManager *PeakFinderMgr = new PeakFinderManager();

    PeakFinderMgr->setMode(PeakFinderManager::Counter);
    PeakFinderMgr->getResult();
    std::list<PeakFinderResult> aResult;
    PeakFinderMgr->getHistory(aResult);
    PeakFinderMgr->resizeHistory(10);
    PeakFinderMgr->resetHistory();
    PeakFinderMgr->historySize();
    PeakFinderMgr->lastFrameNumber();
    PeakFinderMgr->ref();
    PeakFinderMgr->unref();

    PeakFinderTask *peakFinderTask = new PeakFinderTask(*PeakFinderMgr);
    peakFinderTask->ref();
    peakFinderTask->unref();
  }
}
#endif
