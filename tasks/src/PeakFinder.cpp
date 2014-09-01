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
  _nb_peaks(1),
  _computing_mode(PeakFinderTask::MAXIMUM)
{
}

PeakFinderTask::PeakFinderTask(const PeakFinderTask &aTask) :
  SinkTask<PeakFinderResult>(aTask),
  _mask(aTask._mask),
  _nb_peaks(aTask._nb_peaks),
  _computing_mode(PeakFinderTask::MAXIMUM)
{
}

template<class INPUT> static void _compute_peaks(const Data& aData, PeakFinderResult &aResult, int computing_mode){

  const INPUT *aSrcPt = (const INPUT*)aData.data();
  INPUT dataMax;
  int x_max = 0;
  int y_max = 0;


#ifndef WIN32
  INPUT x_projection[aData.dimensions[0]];
  INPUT y_projection[aData.dimensions[1]];
#else
  INPUT *x_projection = (INPUT*)alloca(aData.dimensions[0] * sizeof(INPUT)); 
  INPUT *y_projection = (INPUT*)alloca(aData.dimensions[1] * sizeof(INPUT)); 
#endif
  INPUT x_sum = 0;
  INPUT y_sum = 0;
  INPUT x_background_value = 0;
  INPUT y_background_value = 0;

  int x_pos_sum = 0;
  int x_chan = 0;
  float x_pos;

  int y_pos_sum = 0;
  int y_chan = 0;
  float y_pos;

  // Enable the analysis of 1D data
  int y_dimension;
  y_dimension = aData.dimensions[1];
  if(aData.dimensions.size() != 2){
    if(y_dimension == 0) y_dimension = 1;
  }

  switch(computing_mode)
    { 
    case 0:
      dataMax = 0;
      for(int lineId = 0;lineId < y_dimension;lineId++){
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
      break;
    case 1:
      // Make projections
      for(int lineId = 0;lineId < y_dimension;lineId++){
	y_projection[lineId] = 0;
	for( int x_value = 0; x_value < aData.dimensions[0]; x_value++){
	  if(lineId == 0) x_projection[x_value] = 0;
	  const INPUT *aLinePt = aSrcPt + lineId *aData.dimensions[0] + x_value;
	  y_projection[lineId] = y_projection[lineId] + *aLinePt;
	  x_projection[x_value] = x_projection[x_value] + *aLinePt;
	}
      }
      
      // Compute background
      for(int i = 0; i < aData.dimensions[0]; i++){
	x_sum = x_sum + x_projection[i];
      }
      x_background_value = x_sum / aData.dimensions[0];
      for(int i = 0; i < y_dimension; i++){
	y_sum = y_sum + y_projection[i];
      }
      y_background_value = y_sum / y_dimension;
      
      // Substract background and compute center of signal
      for(int i = 0; i < aData.dimensions[0]; i++){
	x_projection[i] = x_projection[i] - x_background_value;
	if(x_projection[i] < 0.) x_projection[i] = 0;
	if (x_projection[i] > 0.){
	  x_pos_sum = x_pos_sum + i;
	  x_chan = x_chan + 1;
	}
      }
      
      x_pos = x_pos_sum / x_chan;

      for(int i = 0; i < y_dimension; i++){
	y_projection[i] = y_projection[i] - y_background_value;
	if(y_projection[i] < 0.) y_projection[i] = 0;
	if (y_projection[i] > 0.){
	  y_pos_sum = y_pos_sum + i;
	  y_chan = y_chan + 1;
	}
      }
      
      y_pos = y_pos_sum / y_chan;
      
      aResult.x_peak = x_pos;
      aResult.y_peak = y_pos;

      break;
    default:
      break;
    }  
}

void PeakFinderTask::setComputingMode(PeakFinderTask::ComputingMode aComputingMode){

  _computing_mode = aComputingMode;

}
void PeakFinderTask::getComputingMode(PeakFinderTask::ComputingMode &aComputingMode) const{

    aComputingMode = _computing_mode;

}

void PeakFinderTask::process(Data &aData)
{

  PeakFinderResult aResult;
  aResult.frameNumber = aData.frameNumber;

  switch(aData.type)
    {
    case Data::UINT8: 
      _compute_peaks<unsigned char>(aData, aResult, _computing_mode);
      break;
    case Data::INT8:
      _compute_peaks<char>(aData, aResult, _computing_mode);
      break;
      
    case Data::UINT16:
      _compute_peaks<unsigned short>(aData, aResult, _computing_mode);
      break;
    case Data::INT16:
      _compute_peaks<short>(aData, aResult, _computing_mode);
      break;
    case Data::UINT32:
      _compute_peaks<unsigned int>(aData, aResult, _computing_mode);
      break;
    case Data::INT32:
      _compute_peaks<int>(aData, aResult, _computing_mode);
      break;
    case Data::UINT64:
      _compute_peaks<unsigned long long>(aData, aResult, _computing_mode);
      break;
    case Data::INT64:
      _compute_peaks<long long>(aData, aResult, _computing_mode);
      break;      
    case Data::FLOAT:
      _compute_peaks<float>(aData, aResult, _computing_mode);
      break;
    case Data::DOUBLE:
      _compute_peaks<double>(aData, aResult, _computing_mode);
      break;
    default: 
      break;				// error
    }

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
