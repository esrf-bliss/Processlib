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
  class PeakFinderManager
   {
%TypeHeaderCode
#include "processlib/PeakFinder.h"
%End
    public:
    enum ErrorCode {OK,NOT_MANAGED,NO_MORE_AVAILABLE,TIMEDOUT};
    enum RUN_MODE {Counter,Monitor};
    PeakFinderManager(int = 4);

    Tasks::PeakFinderResult    getResult(double = 0.,int = -1) const;
    void	resizeHistory(int aSize);
    void  resetHistory();
    int	  historySize() const;
    int   lastFrameNumber() const;
    protected:
    virtual ~PeakFinderManager();
%MethodCode
 Py_BEGIN_ALLOW_THREADS;
 sipCpp->unref();
 sipCppV = NULL;
 Py_END_ALLOW_THREADS;
%End
   };


  struct PeakFinderResult
  {
%TypeHeaderCode
#include "processlib/PeakFinder.h"
%End
    double                       x_peak;
    double                       y_peak;
    int                          frameNumber;
    Tasks::PeakFinderManager::ErrorCode errorCode;

SIP_PYOBJECT __repr__() const;
%MethodCode
LIMA_REPR_CODE
%End
  };
  
  class PeakFinderTask : SinkTaskBase
  {
%TypeHeaderCode
#include "processlib/PeakFinder.h"
%End
  public:
    enum ComputingMode {MAXIMUM,CM};

    PeakFinderTask(Tasks::PeakFinderManager&);
    PeakFinderTask(const Tasks::PeakFinderTask&);
    virtual void process(Data&);

    void setMask(Data &aMask);
    void setComputingMode(Tasks::PeakFinderTask::ComputingMode);
    void getComputingMode(Tasks::PeakFinderTask::ComputingMode& /Out/) const;
  };
};
