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
#ifndef __ROICOUNTER_H__
#define __ROICOUNTER_H__

#include <sstream>
#include "SinkTask.h"

namespace Tasks
{
  struct RoiCounterResult;
  
  typedef SinkTaskMgr<RoiCounterResult> RoiCounterManager;

  struct DLL_EXPORT RoiCounterResult
  {
    RoiCounterResult() : 
      sum(0.),average(0.),std(0.),
      minValue(0.),maxValue(0.),
      frameNumber(-1),
      errorCode(RoiCounterManager::OK)
    {}
    explicit RoiCounterResult(RoiCounterManager::ErrorCode code) : 
      errorCode(code) {}

    double                       sum;
    double                       average;
    double                       std;
    double			 minValue;
    double			 maxValue;
    int                          frameNumber;
    RoiCounterManager::ErrorCode errorCode;
  };

  inline std::ostream& operator<<(std::ostream &os,const RoiCounterResult &aRoiResult)
  {
    os << "<"
       << "frameNumber=" << aRoiResult.frameNumber << ", "
       << "sum=" << aRoiResult.sum << ", "
       << "average=" << aRoiResult.average << ", "
       << "std=" << aRoiResult.std << ", "
       << "minValue=" << aRoiResult.minValue << ", "
       << "maxValue=" << aRoiResult.maxValue;
    os << ">";
    return os;
  }

  class DLL_EXPORT RoiCounterTask : public SinkTask<RoiCounterResult>
  {
  public:
    RoiCounterTask(RoiCounterManager&);
    RoiCounterTask(const RoiCounterTask&);
    virtual void process(Data&);
    
    void setMask(Data &aMask) {_mask = aMask;}

    void setRoi(int x,int y,int width,int height)
    {
      _x = x,_y = y;
      _width = width,_height = height;
    }
    void getRoi(int &x,int &y,int &width,int &height)
    {
      x = _x,y = _y,width = _width,height = _height;
    }
  private:
    int _x,_y;
    int _width,_height;
    Data _mask;
  };
}
#endif
