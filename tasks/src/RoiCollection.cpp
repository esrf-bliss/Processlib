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
#include <limits>
#include <algorithm>

#include "processlib/RoiCollection.h"

#ifndef __unix
#undef max
#undef min
#endif

using namespace Tasks;

RoiCollectionManager::RoiCollectionManager(int historySize) :
  SinkTaskMgr<RoiCollectionResult>(historySize)
{
  pthread_mutex_init(&_roi_lock,NULL);
}

RoiCollectionManager::~RoiCollectionManager()
{
  pthread_mutex_destroy(&_roi_lock);
}

void RoiCollectionManager::setRois(const std::list<Roi>& rois)
{
  PoolThreadMgr::Lock aLock(&_roi_lock);
  _roi_tasks.clear();
  _rois.clear();
  _rois.reserve(rois.size());
  for(const auto& roi: rois)
    _rois.push_back({roi.x,roi.y,roi.width,roi.height});
}

void RoiCollectionManager::clearRois()
{
  PoolThreadMgr::Lock aLock(&_roi_lock);
  _roi_tasks.clear();
  _rois.clear();
}

void RoiCollectionManager::prepare()
{
  PoolThreadMgr::Lock aLock(&_roi_lock);
  if(_roi_tasks.empty())
    {
      int roi_id = 0;
      _bbox = {std::numeric_limits<int>::max(),std::numeric_limits<int>::max(), 0,0};
      for(const auto& roi: _rois)
	{
	  int m = std::min(0,2);
	  _bbox = {std::min(_bbox.x,roi.x),std::min(_bbox.y,roi.y),
		   std::max(_bbox.lastx,roi.x + roi.width),
		   std::max(_bbox.lasty,roi.y + roi.height)};
	  for(int line=0;line<roi.height;++line)
	    _roi_tasks.push_back({roi.x,roi.y+line,roi.width,roi_id});
	  ++roi_id;
	}
      
      std::sort(_roi_tasks.begin(), _roi_tasks.end(), 
		[](const RoiLineTask & a, const RoiLineTask & b)
		{
		  if(a.y == b.y)
		    return a.x < b.x;
		  else
		    return a.y < b.y;
		});
    }
}

void RoiCollectionManager::_check_roi_with_data_size(Data &aData)
{
  if(_bbox.x < 0 || _bbox.y < 0)
    throw ProcessException("RoiCollectionManager : roi origin must be positive");

  long width = aData.dimensions[0];
  long height = aData.dimensions[1];
  
  if(_bbox.lastx > width || _bbox.lasty > height)
    throw ProcessException("RoiCollectionManager : some roi are outside the image");
}

template<class INPUT,class SUM>
void RoiCollectionManager::_process_with_no_mask(Data &aData,RoiCollectionResult& aResult)
{
  const INPUT *aSrcPt = (INPUT*)aData.data();
  int widthStep = aData.dimensions[0];

  for(const auto &task: _roi_tasks)
    {
      const INPUT *src = aSrcPt + task.x + task.y * widthStep;
      SUM sum_val = SUM(0);
      for(int c=0;c<task.width;++c,++src)
	sum_val += *src;
      int& total_val = aResult.spectrum[task.roi_id];
      total_val += sum_val;
    }
}

template<class INPUT,class SUM>
void RoiCollectionManager::_process_with_mask(Data &aData,RoiCollectionResult& aResult)
{
  const INPUT *aSrcPt = (INPUT*)aData.data();
  const char *aMaskPt = (char*)_mask.data();
  
  int widthStep = aData.dimensions[0];

  for(const auto &task: _roi_tasks)
    {
      const INPUT *src = aSrcPt + task.x + task.y * widthStep;
      const char* mask = aMaskPt + task.x + task.y * widthStep;
      SUM sum_val = SUM(0);
      for(int c=0;c<task.width;++c,++src,++mask)
	if(*mask)
	  sum_val += *src;
      int& total_val = aResult.spectrum[task.roi_id];
      total_val += sum_val;
    }
}

void RoiCollectionManager::process(Data& aData)
{
  if(aData.dimensions.size() != 2)
    throw ProcessException("RoiCollectionManager : Only manage 2D data");

  // should be called before
  prepare();

  _check_roi_with_data_size(aData);

  RoiCollectionResult aResult(_rois.size());
  aResult.frameNumber = aData.frameNumber;
  
  if(_mask.empty())
    {
      switch(aData.type)
	{
	case Data::UINT8: 
	  _process_with_no_mask<unsigned char,unsigned int>(aData,aResult);
	  break;
	case Data::INT8:
	  _process_with_no_mask<char,int>(aData,aResult);
	  break;
	case Data::UINT16:
	  _process_with_no_mask<unsigned short,unsigned int>(aData,aResult);
	  break;
	case Data::INT16:
	  _process_with_no_mask<short,int>(aData,aResult);
	  break;
	case Data::UINT32:
	  _process_with_no_mask<unsigned int,unsigned long long>(aData,aResult);
	  break;
	case Data::INT32:
	  _process_with_no_mask<int,long long>(aData,aResult);
	  break;
	case Data::UINT64:
	  _process_with_no_mask<unsigned long long,unsigned long long>(aData,aResult);
	  break;
	case Data::INT64:
	  _process_with_no_mask<long long,long long>(aData,aResult);
	  break;

	case Data::FLOAT:
	  _process_with_no_mask<float,double>(aData,aResult);
	  break;
	case Data::DOUBLE:
	  _process_with_no_mask<double,double>(aData,aResult);
	  break;
	default: 
	  break;				// error
	}
    }
  else if(_mask.dimensions == aData.dimensions)
    {
      switch(aData.type)
	{
	case Data::UINT8: 
	  _process_with_mask<unsigned char,unsigned int>(aData,aResult);
	  break;
	case Data::INT8:
	  _process_with_mask<char,int>(aData,aResult);
	  break;
	case Data::UINT16:
	  _process_with_mask<unsigned short,unsigned int>(aData,aResult);
	  break;
	case Data::INT16:
	  _process_with_mask<short,int>(aData,aResult);
	  break;
	case Data::UINT32:
	  _process_with_mask<unsigned int,unsigned long long>(aData,aResult);
	  break;
	case Data::INT32:
	  _process_with_mask<int,long long>(aData,aResult);
	  break;
	case Data::UINT64:
	  _process_with_mask<unsigned long long,unsigned long long>(aData,aResult);
	  break;
	case Data::INT64:
	  _process_with_mask<long long,long long>(aData,aResult);
	  break;

	case Data::FLOAT:
	  _process_with_mask<float,double>(aData,aResult);
	  break;
	case Data::DOUBLE:
	  _process_with_mask<double,double>(aData,aResult);
	  break;
	default: 
	  break;				// error
	}
    }
  else
    throw ProcessException("RoiCollectionManager : Source image size differ from mask");

  
  setResult(aResult);
}

RoiCollectionTask::RoiCollectionTask(RoiCollectionManager &aMgr) :
  SinkTask<RoiCollectionResult>(aMgr)
{
}

void RoiCollectionTask::process(Data& aData)
{
  dynamic_cast<RoiCollectionManager&>(_mgr).process(aData);
}
