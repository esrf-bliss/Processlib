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
#ifndef __ENERGYROICOUNTER_H__
#define __ENERGYROICOUNTER_H__

#include <sstream>
#include <vector>
#include <map>
#include <pthread.h>

#include "processlib/SinkTask.h"

namespace Tasks
{
  struct EnergyCounterResult;
  
  class DLL_EXPORT EnergyRoiCounterManager : public SinkTaskMgr<EnergyCounterResult>
  {
  public:
    EnergyRoiCounterManager(int historySize);
    ~EnergyRoiCounterManager();
    
    void addRoi(int x,int y,int width,int height,double energy);
    void clearRoi();
    void getEnergy(std::vector<double>&);

    void setMask(Data &aMask) {_mask = aMask;}

    // will build the lut
    void prepare();
    void process(Data&);
  private:
    template<class INPUT> void _process_with_no_mask(Data&,EnergyCounterResult&);
    template<class INPUT> void _process_with_mask(Data&,EnergyCounterResult&);
    void _check_roi_with_data_size(Data&);
    
    struct Roi
    {
      int x,y;
      int width,height;
    };

    struct BBox
    {
      int x,y;
      int lastx,lasty;
    };

    struct RoiLineTask
    {
      long x,y;
      int width;
      int roi_id;
    };

    mutable pthread_mutex_t	_roi_lock;
    std::map<double,Roi>	_rois;
    std::vector<RoiLineTask>	_roi_tasks;
    BBox 			_bbox;
    Data			_mask;
  };
  
  struct DLL_EXPORT EnergyCounterResult
  {
  EnergyCounterResult(int spectrum_size=0) :
    spectrum(spectrum_size),
    frameNumber(-1),errorCode(EnergyRoiCounterManager::OK)
      {};
     explicit EnergyCounterResult(EnergyRoiCounterManager::ErrorCode code) :
    errorCode(code) {}

    std::vector<double>			spectrum;
    int					frameNumber;
    EnergyRoiCounterManager::ErrorCode	errorCode;
  };

  inline std::ostream& operator<<(std::ostream &os,const EnergyCounterResult &aResult)
  {
    os << "<"
       << "frameNumber=" << aResult.frameNumber << ", "
       << "spectrum size=" << aResult.spectrum.size();
    os << ">";
    return os;
  }

  class DLL_EXPORT EnergyCounterTask : public SinkTask<EnergyCounterResult>
  {
  public:
    EnergyCounterTask(EnergyRoiCounterManager&);
    virtual void process(Data&);
  };
}
#endif
