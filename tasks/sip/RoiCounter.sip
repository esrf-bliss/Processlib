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
#include <RoiCounter.h>
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
#include <RoiCounter.h>
%End
    double                       sum;
    double                       average;
    double                       std;
    double			 minValue;
    double			 maxValue;
    int                          frameNumber;
    Tasks::RoiCounterManager::ErrorCode errorCode;

const char* __repr__();
%MethodCode
std::stringstream str;
str << *sipCpp;
const std::string& tmpString = str.str();
sipRes = tmpString.c_str();
%End
  };
  
  class RoiCounterTask : SinkTaskBase
  {
%TypeHeaderCode
#include <RoiCounter.h>
%End
  public:
    RoiCounterTask(Tasks::RoiCounterManager&);
    RoiCounterTask(const Tasks::RoiCounterTask&);
    virtual void process(Data&);

    void setRoi(int,int,int,int);
    void getRoi(int &x /Out/,int &y /Out/,
		int &width /Out/,int &height /Out/);
   };
};
