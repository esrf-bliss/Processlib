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
 class BpmManager
  {	
%TypeHeaderCode
#include "processlib/Bpm.h"
%End
    public:
    enum ErrorCode {OK,NO_MORE_AVAILABLE,TIMEDOUT};
    enum RUN_MODE {Counter,Monitor};
    BpmManager(int = 4);

    Tasks::BpmResult	getResult(double = 0.,int = -1) const;
    SIP_PYOBJECT getHistory(int fromFrameNumber = 0) const;
%MethodCode
std::list<Tasks::BpmResult> results;
sipCpp->getHistory(results,a0);
sipRes = PyList_New(results.size());
int index = 0;
for(std::list<Tasks::BpmResult>::iterator i = results.begin();
    i != results.end();++i,++index)
  {
    Tasks::BpmResult *cpp_result = new Tasks::BpmResult(*i);
    PyObject *py_result = sipConvertFromType(cpp_result,sipType_Tasks_BpmResult,Py_None);
    if(!py_result)
      {
	Py_DECREF(sipRes);
	delete cpp_result;
	return NULL;
      }
    PyList_SetItem(sipRes,index,py_result);
  }
%End
    void	resizeHistory(int aSize);
    void  resetHistory();
    int	  historySize() const;
    int lastFrameNumber() const;
    protected:
    virtual ~BpmManager();
%MethodCode
 Py_BEGIN_ALLOW_THREADS;
 sipCpp->unref();
 sipCppV = NULL;
 Py_END_ALLOW_THREADS;
%End
  };

  class BpmResult
    {
%TypeHeaderCode
#include "processlib/Bpm.h"
%End
    public:
      BpmResult();

      double		beam_intensity;
      double		beam_center_x;
      double		beam_center_y;
      double		beam_fwhm_x;
      int		beam_fwhm_min_x_index;
      int		beam_fwhm_max_x_index;
      double		beam_fwhm_y;
      int		beam_fwhm_min_y_index;
      int		beam_fwhm_max_y_index;
      unsigned int	max_pixel_value;
      unsigned int	max_pixel_x;
      unsigned int	max_pixel_y;
      Data		profile_x;
      Data		profile_y;

      int		frameNumber;
      
      bool		AOI_automatic;
      unsigned int	AOI_extension_factor;
      int		AOI_min_x;
      int		AOI_max_x;
      int		AOI_min_y;
      int		AOI_max_y;
      unsigned int	border_exclusion;
      double		mBackgroundLevelx;
      double		mBackgroundLevely;
      double		mBackgroundLevelTunex;
      double		mBackgroundLevelTuney;

      Tasks::BpmManager::ErrorCode errorCode;
      double		timestamp;
    };

 

 class BpmTask : SinkTaskBase
  {
%TypeHeaderCode
#include "processlib/Bpm.h"
%End
  public:
    BpmTask(Tasks::BpmManager &);
    BpmTask(const Tasks::BpmTask&);
    virtual void process(Data&); 
    void setRoi(int x1,int x2,
		int y1,int y2);
    void getRoi(int &x1 /Out/,int &x2 /Out/,
		int &y1 /Out/,int &y2 /Out/) const;

    bool		mFwhmTunning;
    double		mFwhmTunningExtension;
    double		mAoiExtension;
    int			mBorderExclusion;
    unsigned int	mThreshold;
    bool		mEnableX;
    bool		mEnableY;
    bool		mEnableBackgroundSubstration;
    bool		mRoiAutomatic;
  };


};
