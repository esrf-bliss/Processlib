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
  class RoiCollectionManager
  {
%TypeHeaderCode
#include "processlib/RoiCollection.h"
%End
  public:
  enum ErrorCode {OK,NOT_MANAGED,NO_MORE_AVAILABLE,TIMEDOUT};
  RoiCollectionManager(int=4);
  Tasks::RoiCollectionCounterResult	getResult(double = 0.,int = -1) const;
  void resizeHistory(int);
  void resetHistory();
  int historySize() const;
  int lastFrameNumber() const;

  void addRoi(int x,int y,int width,int height,double energy);
  void clearRoi();
  SIP_PYOBJECT getEnergy();
%MethodCode
  double *data;
  std::vector<double> result;
  Py_BEGIN_ALLOW_THREADS
  sipCpp->getEnergy(result);
  data = &result[0];
  Py_END_ALLOW_THREADS
  npy_intp dims[] = {result.size()};
  sipRes = PyArray_SimpleNew(1, dims,NPY_DOUBLE);
  PyArrayObject *tmpObject = (PyArrayObject*)PyArray_ContiguousFromObject(sipRes,NPY_NOTYPE,0,0);

  Py_BEGIN_ALLOW_THREADS
  memcpy(PyArray_DATA(tmpObject),data,result.size()*sizeof(double));
  Py_END_ALLOW_THREADS
%End
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

 struct RoiCollectionCounterResult
 {
   int frameNumber;
   Tasks::RoiCollectionManager::ErrorCode errorCode;
   double *spectrum
   {
%GetCode
   if(sipCpp->spectrum.empty())
     Py_RETURN_NONE;

   void *data = sipCpp->spectrum.data();
   npy_intp dims[] = {sipCpp->spectrum.size()};
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

class RoiCollectionCounterTask : SinkTaskBase
{
%TypeHeaderCode
#include "processlib/RoiCollection.h"
%End
public:
  RoiCollectionCounterTask(Tasks::RoiCollectionManager&);
  RoiCollectionCounterTask(const Tasks::RoiCollectionCounterTask&);
  virtual void process(Data&);
};

};