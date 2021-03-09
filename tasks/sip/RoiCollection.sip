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
namespace Tasks
{
  class RoiCollectionManager
  {
%TypeHeaderCode
#include "processlib/RoiCollection.h"
%End
  public:
  enum ErrorCode {OK,NOT_MANAGED,NO_MORE_AVAILABLE,TIMEDOUT};
  RoiCollectionManager(int=4);
  Tasks::RoiCollectionResult	getResult(double = 0.,int = -1) const;
  SIP_PYOBJECT	getHistory(int fromFrameNumber = 0) const;
%MethodCode
  std::list<Tasks::RoiCollectionResult> return_list;
  Py_BEGIN_ALLOW_THREADS
  sipCpp->getHistory(return_list,a0);
  Py_END_ALLOW_THREADS
  sipRes = PyList_New(return_list.size());
  int index = 0;
  for(const auto &src: return_list)
  {
    PyObject *wobj;
    Tasks::RoiCollectionResult *aResult = new Tasks::RoiCollectionResult(src);
    if(!(wobj = sipConvertFromNewType(aResult,sipType_Tasks_RoiCollectionResult,NULL)))
    {
      delete aResult;
      Py_DECREF(sipRes);
    }
    PyList_SET_ITEM(sipRes,index,wobj);
    ++index;
  }
  
%End
  void resizeHistory(int);
  void resetHistory();
  int historySize() const;
  int lastFrameNumber() const;

  void setRois(SIP_PYOBJECT);
%MethodCode
	  std::list<Tasks::RoiCollectionManager::Roi> rois;
	  int sequence_flag = PySequence_Check(a0);
	  if(!sequence_flag)
	    return NULL;	// raise a not very useful exception
	  Py_ssize_t sequence_size = PySequence_Size(a0);
	  for(int i=0;i<sequence_size;++i)
	    {
	      PyObject* roi_value = PySequence_GetItem(a0,i);
	      sequence_flag = PySequence_Check(roi_value);
	      int nb_roi_values = PySequence_Size(roi_value);
	      if(nb_roi_values != 4)
		return NULL;

	      int x,y,width,height;
	      std::vector<std::tuple<int,int&>> tuple_values = {{0,x},{1,y},{2,width},{3,height}};
	      for(const auto& val: tuple_values)
		{
		  PyObject* obj = PySequence_GetItem(roi_value,std::get<0>(val));
		  PyObject* py_long = PyNumber_Long(obj);
		  if(!py_long)
		    return NULL;
		  long value = PyLong_AsLong(py_long);
		  Py_DECREF(py_long);
		  std::get<1>(val) = value;
		}
	      rois.push_back({x,y,width,height});
	    }
	  Py_BEGIN_ALLOW_THREADS
	  sipCpp->setRois(rois);
	  Py_END_ALLOW_THREADS
%End
  void clearRois();
  void setMask(Data&);
  void prepare();
  void process(Data&);
protected:
  virtual ~RoiCollectionManager();
%MethodCode
 Py_BEGIN_ALLOW_THREADS;
 sipCpp->unref();
 sipCppV = NULL;
 Py_END_ALLOW_THREADS;
%End
  };

 struct RoiCollectionResult
 {
   int frameNumber;
   Tasks::RoiCollectionManager::ErrorCode errorCode;
   double *spectrum
   {
%GetCode
   if(sipCpp->spectrum.empty())
     Py_RETURN_NONE;

   void *data = sipCpp->spectrum.data();
   npy_intp dims[] = {int(sipCpp->spectrum.size())};
   sipPy = PyArray_SimpleNew(1,dims,NPY_INT32);
   PyArrayObject *tmpObject = (PyArrayObject*)PyArray_ContiguousFromObject(sipPy,NPY_NOTYPE,0,0);
   Py_BEGIN_ALLOW_THREADS
   memcpy(PyArray_DATA(tmpObject),data,dims[0]*sizeof(int));
   Py_END_ALLOW_THREADS

%End
%SetCode
//Not needed
sipErr=1;
%End
   };
};

class RoiCollectionTask : SinkTaskBase
{
%TypeHeaderCode
#include "processlib/RoiCollection.h"
%End
public:
  RoiCollectionTask(Tasks::RoiCollectionManager&);
  RoiCollectionTask(const Tasks::RoiCollectionTask&);
  virtual void process(Data&);
};

};