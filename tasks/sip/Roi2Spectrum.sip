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
  class Roi2SpectrumManager
  {
%TypeHeaderCode
#include "processlib/Roi2Spectrum.h"
%End

  public:
    enum ErrorCode {OK,NOT_MANAGED,NO_MORE_AVAILABLE,TIMEDOUT};
    enum RUN_MODE {Counter,Monitor};

    explicit Roi2SpectrumManager(int = 4);
  
    void	setMode(Tasks::Roi2SpectrumManager::RUN_MODE);

    Tasks::Roi2SpectrumResult getResult(double timeout = 0.,
					int frameNumber = -1) const;

    void resizeHistory(int aSize);
    void resetHistory();
    int	historySize() const;
    //@brief return the last available frame with no hole before
    int lastFrameNumber() const;
  protected:
    virtual ~Roi2SpectrumManager();
%MethodCode
 Py_BEGIN_ALLOW_THREADS;
 sipCpp->unref();
 sipCppV = NULL;
 Py_END_ALLOW_THREADS;
%End

  };

  struct Roi2SpectrumResult
  {
%TypeHeaderCode
#include "processlib/Roi2Spectrum.h"
%End
    Data 				spectrum;
    int 				frameNumber;
    Tasks::Roi2SpectrumManager::ErrorCode 	errorCode;
    
    SIP_PYOBJECT __repr__() const;
%MethodCode
    LIMA_REPR_CODE
%End
  };

  class Roi2SpectrumTask : SinkTaskBase
  {
%TypeHeaderCode
#include "processlib/Roi2Spectrum.h"
%End
  public:
    enum Mode {LINES_SUM,COLUMN_SUM};

    Roi2SpectrumTask(Tasks::Roi2SpectrumManager&);
    Roi2SpectrumTask(const Tasks::Roi2SpectrumTask&);
    virtual void process(Data&);

    void setMode(Mode aMode);
    Mode getMode() const;

    void setRoi(int x,int y,int width,int height);
    void getRoi(int &x /Out/,int &y /Out/,int &width /Out/,int &height /Out/);
  };
};
