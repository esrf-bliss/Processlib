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
#include <limits>
#include <algorithm>

#include "processlib/EnergyRoiCounter.h"

using namespace Tasks;

EnergyRoiCounterManager::EnergyRoiCounterManager(int historySize) :
  SinkTaskMgr<EnergyCounterResult>(historySize)
{
  pthread_mutex_init(&_roi_lock,NULL);
}

EnergyRoiCounterManager::~EnergyRoiCounterManager()
{
  pthread_mutex_destroy(&_roi_lock);
}

void EnergyRoiCounterManager::addRoi(int x,int y,int width,int height,double energy)
{
  PoolThreadMgr::Lock aLock(&_roi_lock);
  _roi_tasks.clear();
  _rois.insert({energy, {x,y,width,height}});
}

void EnergyRoiCounterManager::clearRoi()
{
  PoolThreadMgr::Lock aLock(&_roi_lock);
  _roi_tasks.clear();
  _rois.clear();
}

void EnergyRoiCounterManager::getEnergy(std::vector<double>& energy)
{
  energy.clear();
  energy.reserve(_rois.size());

  PoolThreadMgr::Lock aLock(&_roi_lock);
  for(const auto& i : _rois)
    energy.push_back(i.first);
}

void EnergyRoiCounterManager::prepare()
{
  PoolThreadMgr::Lock aLock(&_roi_lock);
  if(_roi_tasks.empty())
    {
      int roi_id = 0;
      _bbox = {std::numeric_limits<int>::max(),std::numeric_limits<int>::max(), 0,0};
      for(const auto& i: _rois)
	{
	  const Roi& roi = i.second;
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

void EnergyRoiCounterManager::_check_roi_with_data_size(Data &aData)
{
  if(_bbox.x < 0 || _bbox.y < 0)
    throw ProcessException("EnergyRoiCounterManager : roi origin must be positive");

  long width = aData.dimensions[0];
  long height = aData.dimensions[1];
  
  if(_bbox.lastx > width || _bbox.lasty > height)
    throw ProcessException("EnergyRoiCounterManager : some roi are outside the image");
}

template<class INPUT>
void EnergyRoiCounterManager::_process_with_no_mask(Data &aData,EnergyCounterResult& aResult)
{
  const INPUT *aSrcPt = (INPUT*)aData.data();
  int widthStep = aData.dimensions[0];

  for(const auto &task: _roi_tasks)
    {
      const INPUT *src = aSrcPt + task.x + task.y * widthStep;
      INPUT sum_val = INPUT(0);
      for(int c=0;c<task.width;++c,++src)
	sum_val += *src;
      int& total_val = aResult.spectrum[task.roi_id];
      total_val += sum_val;
    }
}

template<class INPUT>
void EnergyRoiCounterManager::_process_with_mask(Data &aData,EnergyCounterResult& aResult)
{
  const INPUT *aSrcPt = (INPUT*)aData.data();
  const char *aMaskPt = (char*)_mask.data();
  
  int widthStep = aData.dimensions[0];

  for(const auto &task: _roi_tasks)
    {
      const INPUT *src = aSrcPt + task.x + task.y * widthStep;
      const char* mask = aMaskPt + task.x + task.y * widthStep;
      INPUT sum_val = INPUT(0);
      for(int c=0;c<task.width;++c,++src,++mask)
	if(*mask)
	  sum_val += *src;
      int& total_val = aResult.spectrum[task.roi_id];
      total_val += sum_val;
    }
}

void EnergyRoiCounterManager::process(Data& aData)
{
  if(aData.dimensions.size() != 2)
    throw ProcessException("EnergyRoiCounterManager : Only manage 2D data");

  // should be called before
  prepare();

  _check_roi_with_data_size(aData);

  EnergyCounterResult aResult(_rois.size());
  aResult.frameNumber = aData.frameNumber;
  
  if(_mask.empty())
    {
      switch(aData.type)
	{
	case Data::UINT8: 
	  _process_with_no_mask<unsigned char>(aData,aResult);
	  break;
	case Data::INT8:
	  _process_with_no_mask<char>(aData,aResult);
	  break;
	case Data::UINT16:
	  _process_with_no_mask<unsigned short>(aData,aResult);
	  break;
	case Data::INT16:
	  _process_with_no_mask<short>(aData,aResult);
	  break;
	case Data::UINT32:
	  _process_with_no_mask<unsigned int>(aData,aResult);
	  break;
	case Data::INT32:
	  _process_with_no_mask<int>(aData,aResult);
	  break;
	case Data::UINT64:
	  _process_with_no_mask<unsigned long long>(aData,aResult);
	  break;
	case Data::INT64:
	  _process_with_no_mask<long long>(aData,aResult);
	  break;

	case Data::FLOAT:
	  _process_with_no_mask<float>(aData,aResult);
	  break;
	case Data::DOUBLE:
	  _process_with_no_mask<double>(aData,aResult);
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
	  _process_with_mask<unsigned char>(aData,aResult);
	  break;
	case Data::INT8:
	  _process_with_mask<char>(aData,aResult);
	  break;
	case Data::UINT16:
	  _process_with_mask<unsigned short>(aData,aResult);
	  break;
	case Data::INT16:
	  _process_with_mask<short>(aData,aResult);
	  break;
	case Data::UINT32:
	  _process_with_mask<unsigned int>(aData,aResult);
	  break;
	case Data::INT32:
	  _process_with_mask<int>(aData,aResult);
	  break;
	case Data::UINT64:
	  _process_with_mask<unsigned long long>(aData,aResult);
	  break;
	case Data::INT64:
	  _process_with_mask<long long>(aData,aResult);
	  break;

	case Data::FLOAT:
	  _process_with_mask<float>(aData,aResult);
	  break;
	case Data::DOUBLE:
	  _process_with_mask<double>(aData,aResult);
	  break;
	default: 
	  break;				// error
	}
    }
  else
    throw ProcessException("EnergyRoiCounterManager : Source image size differ from mask");

  
  setResult(aResult);
}

EnergyCounterTask::EnergyCounterTask(EnergyRoiCounterManager &aMgr) :
  SinkTask<EnergyCounterResult>(aMgr)
{
}

void EnergyCounterTask::process(Data& aData)
{
  dynamic_cast<EnergyRoiCounterManager&>(_mgr).process(aData);
}
