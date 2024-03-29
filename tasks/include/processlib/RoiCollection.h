//###########################################################################
// This file is part of ProcessLib, a submodule of LImA project the
// Library for Image Acquisition
//
// Copyright (C) : 2009-2021
// European Synchrotron Radiation Facility
// CS40220 38043 Grenoble Cedex 9 
// FRANCE
//
// Contact: lima@esrf.fr
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
#ifndef __ROICOLLECTIONCOUNTER_H__
#define __ROICOLLECTIONCOUNTER_H__

#include <sstream>
#include <vector>
#include <list>
#include <pthread.h>

#include "processlib/SinkTask.h"

namespace Tasks
{
  struct RoiCollectionResult;
  
  class DLL_EXPORT RoiCollectionManager : public SinkTaskMgr<RoiCollectionResult>
  {
  public:
    struct Roi
    {
      int x,y;
      int width,height;
    };

    RoiCollectionManager(int historySize);
    ~RoiCollectionManager();
    
    void setRois(const std::list<Roi>& rois);
    void clearRois();
    void setMask(Data &aMask) {_mask = aMask;}

    void getOverflowThreshold(long long&) const;
    void setOverflowThreshold(long long);
    
    // will build the lut
    void prepare();
    void process(Data&);
  private:
    template<class INPUT,class SUM> void _process_with_no_mask(Data&,RoiCollectionResult&);
    template<class INPUT,class SUM> void _process_with_mask(Data&,RoiCollectionResult&);
    template<class INPUT,class SUM> void _process_with_no_mask_with_threshold(Data&,RoiCollectionResult&,long long);
    template<class INPUT,class SUM> void _process_with_mask_with_threshold(Data&,RoiCollectionResult&,long long);
    void _check_roi_with_data_size(Data&);
    

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
    std::vector<Roi>		_rois;
    std::vector<RoiLineTask>	_roi_tasks;
    BBox 			_bbox;
    Data			_mask;
    long long			_overflow_threshold;
  };
  
  struct DLL_EXPORT RoiCollectionResult
  {
  RoiCollectionResult(int spectrum_size=0) :
    spectrum(spectrum_size),
    frameNumber(-1),errorCode(RoiCollectionManager::OK)
      {};
     explicit RoiCollectionResult(RoiCollectionManager::ErrorCode code) :
    errorCode(code) {}

    std::vector<int>			spectrum;
    int					frameNumber;
    RoiCollectionManager::ErrorCode	errorCode;
  };

  inline std::ostream& operator<<(std::ostream &os,const RoiCollectionResult &aResult)
  {
    os << "<"
       << "frameNumber=" << aResult.frameNumber << ", "
       << "spectrum size=" << aResult.spectrum.size();
    os << ">";
    return os;
  }

  class DLL_EXPORT RoiCollectionTask : public SinkTask<RoiCollectionResult>
  {
  public:
    RoiCollectionTask(RoiCollectionManager&);
    virtual void process(Data&);
  };
}
#endif
