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
namespace Tasks
{
  class RoiCounterManager
   {
%TypeHeaderCode
#include "processlib/RoiCounter.h"
%End
    public:
    enum ErrorCode {OK,NOT_MANAGED,NO_MORE_AVAILABLE,TIMEDOUT};
    enum RUN_MODE {Counter,Monitor};
    RoiCounterManager(int = 4);

    Tasks::RoiCounterResult	getResult(double = 0.,int = -1) const;
    void	resizeHistory(int aSize);
    void  resetHistory();
    int	  historySize() const;
    int   lastFrameNumber() const;
    protected:
    virtual ~RoiCounterManager();
%MethodCode
 Py_BEGIN_ALLOW_THREADS;
 sipCpp->unref();
 sipCppV = NULL;
 Py_END_ALLOW_THREADS;
%End
   };


  struct RoiCounterResult
  {
%TypeHeaderCode
#include "processlib/RoiCounter.h"
%End
    double                       sum;
    double                       average;
    double                       std;
    double			 minValue;
    double			 maxValue;
    int                          frameNumber;
    Tasks::RoiCounterManager::ErrorCode errorCode;

    SIP_PYOBJECT __repr__();
%MethodCode 
    LIMA_REPR_CODE
%End
  };
  
  class RoiCounterTask : SinkTaskBase
  {
%TypeHeaderCode
#include "processlib/RoiCounter.h"
%End
  public:
    enum type {UNDEF,SQUARE,ARC,LUT,MASK};
    RoiCounterTask(Tasks::RoiCounterManager&);
    RoiCounterTask(const Tasks::RoiCounterTask&);
    virtual void process(Data&);

    void setMask(Data &aMask);
    void getType(type& aType /Out/);
    //Method for SQUARE roi
    void setRoi(int,int,int,int);
    void getRoi(int &x /Out/,int &y /Out/,
		int &width /Out/,int &height /Out/);
    //Method for LUT roi
    void setLut(int x,int y,Data &lut);
    void getLut(int &x /Out/,int &y /Out/,Data &lut /Out/);
    //Method for Mask
    void setLutMask(int x,int y,Data &mask);
    void getLutMask(int &x /Out/,int &y /Out/,Data &mask /Out/);
    //Method helper for arcs
    void setArcMask(double centerX,double centerY,
		    double rayon1,double rayon2,
		    double angle_start,double angle_end);
    void getArcMask(double &centerX /Out/,double &centerY /Out/,
		    double &rayon1 /Out/,double &rayon2 /Out/,
		    double &angle_start /Out/,double &angle_end /Out/);
  };
};
