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
#ifndef __PEAKFINDER_H__
#define __PEAKFINDER_H__

#include "processlib/SinkTask.h"
#include <sstream>

namespace Tasks {
struct PeakFinderResult;

typedef SinkTaskMgr<PeakFinderResult> PeakFinderManager;

struct DLL_EXPORT PeakFinderResult
{
    PeakFinderResult() : x_peak(0.), y_peak(0.), frameNumber(-1), errorCode(PeakFinderManager::OK) {}
    explicit PeakFinderResult(PeakFinderManager::ErrorCode code) : errorCode(code) {}

    double x_peak;
    double y_peak;
    int frameNumber;
    PeakFinderManager::ErrorCode errorCode;
};

inline std::ostream &operator<<(std::ostream &os, const PeakFinderResult &aPeakResult)
{
    os << "<"
       << "frameNumber=" << aPeakResult.frameNumber << ", "
       << "x_peak=" << aPeakResult.x_peak << ", "
       << "y_peak=" << aPeakResult.y_peak;
    os << ">";
    return os;
}

class DLL_EXPORT PeakFinderTask : public SinkTask<PeakFinderResult>
{
  public:
    enum ComputingMode { MAXIMUM, CM };
    PeakFinderTask(PeakFinderManager &);
    PeakFinderTask(const PeakFinderTask &);
    virtual void process(Data &);

    void setMask(Data &aMask) { _mask = aMask; }
    void setComputingMode(ComputingMode);
    void getComputingMode(ComputingMode &) const;

  private:
    Data _mask;
    int _nb_peaks;
    ComputingMode _computing_mode;

    //    void _compute_peaks(const Data& aData, PeakFinderResult &aResult);
};

} // namespace Tasks
#endif
