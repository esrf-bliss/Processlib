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
#ifndef __ROI2SPECTRUM_H
#define __ROI2SPECTRUM_H

#include "SinkTask.h"

namespace Tasks
{
  struct Roi2SpectrumResult;
  
  typedef SinkTaskMgr<Roi2SpectrumResult> Roi2SpectrumManager;

  struct DLL_EXPORT Roi2SpectrumResult
  {
    Roi2SpectrumResult() :
      frameNumber(-1),
      errorCode(Roi2SpectrumManager::OK)
    {}
    explicit Roi2SpectrumResult(Roi2SpectrumManager::ErrorCode code) :
      errorCode(code) {}
    
    Data 				spectrum;
    int 				frameNumber;
    Roi2SpectrumManager::ErrorCode 	errorCode;
  };

  inline std::ostream& operator<<(std::ostream &os,
				  const Roi2SpectrumResult &aResult)
    {
      os << "<"
	 << "frameNumber=" << aResult.frameNumber << ", "
	 << "spectrum=" << aResult.spectrum;
      os << ">";
      return os;
    }

  class DLL_EXPORT Roi2SpectrumTask : public SinkTask<Roi2SpectrumResult>
  {
  public:
    enum Mode {LINES_SUM,COLUMN_SUM};

    Roi2SpectrumTask(Roi2SpectrumManager&);
    Roi2SpectrumTask(const Roi2SpectrumTask&);
    virtual void process(Data&);

    void setMode(Mode aMode) {_mode = aMode;}
    Mode getMode() const {return _mode;}

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
    Mode _mode;
  };
}
#endif
