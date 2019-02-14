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
#include "processlib/SinkTask.h"

namespace Tasks
{
  struct RoiCounterResult;
  
  typedef SinkTaskMgr<RoiCounterResult> RoiCounterManager;

  struct PROCESSLIB_EXPORT RoiCounterResult
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

  class PROCESSLIB_EXPORT RoiCounterTask : public SinkTask<RoiCounterResult>
  {
  public:
    enum type {UNDEF,SQUARE,ARC,LUT,MASK};
    RoiCounterTask(RoiCounterManager&);
    RoiCounterTask(const RoiCounterTask&);
    virtual void process(Data&);
    
    void setMask(Data &aMask) {_mask = aMask;}
    void getType(type& aType) {aType = _type;}
    //Method for SQUARE roi
    void setRoi(int x,int y,int width,int height);
    void getRoi(int &x,int &y,int &width,int &height);
    //Method for LUT roi
    void setLut(int x,int y,Data &lut);
    void getLut(int &x,int &y,Data &lut);
    //Method for Mask
    void setLutMask(int x,int y,Data &mask);
    void getLutMask(int &x,int &y,Data &mask);
    //Method helper for arcs
    void setArcMask(double centerX,double centerY,
		    double rayon1,double rayon2,
		    double angle_start,double angle_end);
    void getArcMask(double &centerX,double &centerY,
		    double &rayon1,double &rayon2,
		    double &angle_start,double &angle_end);
  private:
    void _check_roi_with_data_size(Data&);

    typedef struct ArcRoi {
      double x, y;
      double r1, r2;
      double a1, a2;
      ArcRoi() : x(0), y(0), r1(0), r2(0), a1(0), a2(0) 
      {}
      ArcRoi(double dx, double dy, 
	     double dr1, double dr2,
	     double da1, double da2) 
	: x(dx), y(dy), r1(dr1), r2(dr2), a1(da1), a2(da2) 
      {}
      bool isSet() 
      { return (r1 != 0) || (r2 != 0) || (a1 != 0) || (a2 != 0); }
    } ArcRoi;

    type _type;
    int _x,_y;
    int _width,_height;
    ArcRoi _arc_roi;
    Data _mask;
    Data _lut;
  };
}
#endif
