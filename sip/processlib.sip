//###########################################################################
// This file is part of ProcessLib, a submodule of LImA project the
// Library for Image Acquisition
//
// Copyright (C) : 2009-2019
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
%Module(name=processlib)

%ModuleHeaderCode
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define NO_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL _processlibNumpyArray
#include "numpy/arrayobject.h"
#if (defined(NPY_API_VERSION) && NPY_API_VERSION >= 0x00000004 && PY_VERSION_HEX >= 0x03000000)
#include "numpy/npy_3kcompat.h"
#endif

#if PY_MAJOR_VERSION >= 3
extern "C" { int processlib_import_array();}
#else
extern "C" { void processlib_import_array();}
#endif

#include "processlib/Data.h"

struct Data_HeaderContainer_itemIterator
{
  Data_HeaderContainer_itemIterator(Data::HeaderContainer &cnt) :
    _cur(cnt.header().begin()),_end(cnt.header().end())
  {}

  std::map<std::string,std::string>::iterator _cur;
  std::map<std::string,std::string>::iterator _end;
};
%End

%PostInitialisationCode
processlib_import_array();
%End

%ModuleHeaderCode
#include <string>
#include <Python.h>

#ifndef LIMA_REPR_CODE
#if PY_MAJOR_VERSION >= 3
#define LIMA_REPR_CODE \
	std::ostringstream str; \
	str << *sipCpp;	\
	const std::string& tmpString = str.str(); \
	sipRes = PyUnicode_FromString(tmpString.c_str());
#else
#define LIMA_REPR_CODE \
	std::ostringstream str; \
	str << *sipCpp;	\
	const std::string& tmpString = str.str(); \
	sipRes = PyString_FromString(tmpString.c_str());
#endif
#endif //LIMA_REPR_CODE

inline PyObject *sipConvertToPyStr(const std::string& s)
{
    PyObject *newstring;

#if PY_MAJOR_VERSION >= 3
    // convert an std::string to a Python (unicode) string
    newstring = PyUnicode_FromStringAndSize(s.c_str(), s.length());
    if (newstring == NULL) {
        PyErr_Clear();
        newstring = PyString_FromString(s.c_str());
    }
#else
    // convert an std::string to a Python string - assume an ASCII string
    newstring = PyString_FromString(s.c_str());
#endif

  return newstring;
}

inline bool sipCheckConvertFromPyStr(PyObject *sipPy)
{
    return (PyString_Check(sipPy) || PyUnicode_Check(sipPy));
}

inline std::string sipConvertFromPyStr(PyObject *sipPy)
{
    std::string s;
    // Allow a Python string (or a unicode string) whenever a string is
    // expected.
    // If argument is a Unicode string, just decode it to UTF-8
    // If argument is a Python string, assume it's UTF-8
    if (PyUnicode_Check(sipPy)) {
        PyObject *pyS = PyUnicode_AsEncodedString(sipPy, "UTF-8", "");
        s = PyString_AS_STRING(pyS);
        Py_DECREF(pyS);
    } else if (PyString_Check(sipPy)) {
        s = PyString_AS_STRING(sipPy);
    } else if (sipPy != Py_None) {
        throw std::exception();
    }
    return s;
}
%End


%Exception Data_Header_Iter_Stop_iteration /PyName=StopIteration/
{
%TypeHeaderCode
#include <iostream>
%End
%RaiseCode
        SIP_BLOCK_THREADS
        PyErr_SetString(PyExc_Exception,"");
        SIP_UNBLOCK_THREADS
%End
};

struct Data_HeaderContainer_itemIterator
{
public:
  Data_HeaderContainer_itemIterator(Data::HeaderContainer&);
  SIP_PYOBJECT __next__();
%MethodCode
if(sipCpp->_cur != sipCpp->_end)
{
  sipRes = Py_BuildValue("(ss)",sipCpp->_cur->first.c_str(),sipCpp->_cur->second.c_str());
  ++(sipCpp->_cur);
}
else
 PyErr_SetString(PyExc_StopIteration,"");
%End

Data_HeaderContainer_itemIterator* __iter__();
%MethodCode
sipRes = sipCpp;
%End
};

struct Data
{
%TypeHeaderCode
#include "processlib/Data.h"
#include "processlib/PoolThreadMgr.h"
#include <sstream>
%End

  class HeaderContainer
  {
  public:
    HeaderContainer();


SIP_PYOBJECT pop(SIP_PYOBJECT,SIP_PYOBJECT = NULL);
%MethodCode
if (!sipCheckConvertFromPyStr(a0) || (a1!=NULL && !sipCheckConvertFromPyStr(a1)))
{
    PyErr_SetString(PyExc_TypeError,"Only string key and string value are supported");
    sipIsErr = 1;
}
else
{
    PoolThreadMgr::Lock aLock(sipCpp->mutex());
    Data::HeaderContainer::Header &header = sipCpp->header();
    std::string key = sipConvertFromPyStr(a0);
    std::map<std::string,std::string>::iterator i = header.find(key);
    if(i != header.end())
    {
        sipRes = sipConvertToPyStr(i->second);
	header.erase(i);
    }
    else if (a1)
        sipRes = a1;
    else
    {
        Py_INCREF(Py_None);
        sipRes = Py_None;
    }
}
%End

void update(SIP_PYDICT);
%MethodCode
PyObject *key, *value;
SIP_SSIZE_T pos = 0;
PoolThreadMgr::Lock aLock(sipCpp->mutex());
Data::HeaderContainer::Header &header = sipCpp->header();
while (PyDict_Next(a0, &pos, &key, &value))
 {
   if (!sipCheckConvertFromPyStr(key) || !sipCheckConvertFromPyStr(value))
   {
    PyErr_SetString(PyExc_TypeError,"Only string key and string value are supported");
    sipIsErr = 1;
    break;
   }
std::pair<std::map<std::string,std::string>::iterator, bool> result =
header.insert(std::pair<std::string,std::string>(sipConvertFromPyStr(key),sipConvertFromPyStr(value)));
if(!result.second)
  result.first->second = sipConvertFromPyStr(value);
 }
%End

SIP_PYOBJECT __repr__() const;
%MethodCode
std::stringstream str;
str << "{";
PoolThreadMgr::Lock aLock(sipCpp->mutex());
Data::HeaderContainer::Header &header = sipCpp->header();
for(std::map<std::string,std::string>::iterator i = header.begin();
    i != header.end();++i)
    {
	str << "'" << i->first << "':";
	str << "'" << i->second << "',";
    }
if(str.str().size() > 1)
  str.seekp (-1, str.end);
str << "}";
const std::string& tmpString = str.str();
sipRes = sipConvertToPyStr(tmpString);
%End

const char* __str__();
%MethodCode
std::stringstream str;
str << "{";
PoolThreadMgr::Lock aLock(sipCpp->mutex());
Data::HeaderContainer::Header &header = sipCpp->header();
for(std::map<std::string,std::string>::iterator i = header.begin();
    i != header.end();++i)
    {
	str << "'" << i->first << "':";
	str << "'" << i->second << "',";
    }
if(str.str().size() > 1)
  str.seekp (-1, str.end);
str << "}";
const std::string& tmpString = str.str();
sipRes = tmpString.c_str();
%End
void __enter__();
%MethodCode
sipCpp->lock();
%End

void __exit__(SIP_PYOBJECT,SIP_PYOBJECT,SIP_PYOBJECT);
%MethodCode
sipCpp->unlock();
%End

SIP_PYOBJECT values();
%MethodCode
PoolThreadMgr::Lock aLock(sipCpp->mutex());
Data::HeaderContainer::Header &header = sipCpp->header();
sipRes = PyList_New(header.size());
int index = 0;
for(std::map<std::string,std::string>::iterator i = header.begin();
    i != header.end();++i,++index)
	PyList_SetItem(sipRes,index,
	               Py_BuildValue("s",i->second.c_str()));
%End

SIP_PYOBJECT keys();
%MethodCode
PoolThreadMgr::Lock aLock(sipCpp->mutex());
Data::HeaderContainer::Header &header = sipCpp->header();
sipRes = PyList_New(header.size());
int index = 0;
for(std::map<std::string,std::string>::iterator i = header.begin();
    i != header.end();++i,++index)
	PyList_SetItem(sipRes,index,
	               Py_BuildValue("s",i->first.c_str()));
%End

SIP_PYOBJECT items();
%MethodCode
PoolThreadMgr::Lock aLock(sipCpp->mutex());
Data::HeaderContainer::Header &header = sipCpp->header();
sipRes = PyList_New(header.size());
int index = 0;
for(std::map<std::string,std::string>::iterator i = header.begin();
    i != header.end();++i,++index)
	PyList_SetItem(sipRes,index,
	               Py_BuildValue("(ss)",i->first.c_str(),i->second.c_str()));
%End
Data_HeaderContainer_itemIterator iteritems();
%MethodCode
sipRes = new Data_HeaderContainer_itemIterator(*sipCpp);
%End

bool has_key(SIP_PYOBJECT);
%MethodCode
if (!sipCheckConvertFromPyStr(a0))
{
    PyErr_Format(PyExc_TypeError,"Only string key is supported, but got %s", Py_TYPE(a0)->tp_name);    
    sipIsErr = 1;
}
else
{
    std::string key = sipConvertFromPyStr(a0);
    PoolThreadMgr::Lock aLock(sipCpp->mutex());
    Data::HeaderContainer::Header &header = sipCpp->header();
    std::map<std::string,std::string>::iterator i = header.find(key);
    sipRes = i != header.end();
}
%End

int __len__();
%MethodCode
sipRes = sipCpp->size();
%End

void  __setitem__(SIP_PYOBJECT, SIP_PYOBJECT);
%MethodCode
   if (!sipCheckConvertFromPyStr(a0) || !sipCheckConvertFromPyStr(a1))
   {
       PyErr_SetString(PyExc_TypeError,"Only string key and string value are supported");    
       sipIsErr = 1;
   }
   else
   {
       std::string key, value;
       key = sipConvertFromPyStr(a0); value = sipConvertFromPyStr(a1);
       sipCpp->insert(key.c_str(), value.c_str());
   }
%End
SIP_PYOBJECT __getitem__(SIP_PYOBJECT) const;
%MethodCode
   if (!sipCheckConvertFromPyStr(a0))
   {
       PyErr_Format(PyExc_TypeError,"Only string key is supported");    
       sipIsErr = 1;   
   }
   else
  {
       std::string key = sipConvertFromPyStr(a0);
       PoolThreadMgr::Lock aLock(sipCpp->mutex());
       Data::HeaderContainer::Header &header = sipCpp->header();
       std::map<std::string,std::string>::iterator i = header.find(key);
       if(i != header.end())
         sipRes = sipConvertToPyStr(i->second);
       else
       {
           PyErr_Format(PyExc_KeyError,"Header has no key %s", key.c_str());
	   sipIsErr = 1;
       }
       
   }
%End

    const char* get(const char *key,const char *defaultValue = NULL) const;
    const char* operator[](const char *aKey) const;
    int size() const;
    void clear();
  };
  int frameNumber;
  double timestamp;
  Data::HeaderContainer header;
  void *buffer
{
%GetCode
if(!sipCpp->buffer || !sipCpp->buffer->data)
   Py_RETURN_NONE;

  int ndims = sipCpp->dimensions.size();
  npy_intp *dims = new npy_intp[ndims];
  std::vector<int>::const_reverse_iterator i = sipCpp->dimensions.rbegin();
  std::vector<int>::const_reverse_iterator end = sipCpp->dimensions.rend();
  for(int k = 0;i != end;++i,++k)
    dims[k] = *i;

switch(sipCpp->type)
{
  case Data::UINT8: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_UBYTE,sipCpp->buffer->data);break;
  case Data::INT8: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_BYTE,sipCpp->buffer->data);break;
  case Data::UINT16: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_USHORT,sipCpp->buffer->data);break;
  case Data::INT16: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_SHORT,sipCpp->buffer->data);break;
  case Data::UINT32: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_UINT,sipCpp->buffer->data);break;
  case Data::UINT64: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_UINT64,sipCpp->buffer->data);break;
  case Data::INT64: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_INT64,sipCpp->buffer->data);break;
  case Data::INT32: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_INT32,sipCpp->buffer->data);break;
  case Data::FLOAT: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_FLOAT32,sipCpp->buffer->data);break;
  case Data::DOUBLE: sipPy = PyArray_SimpleNewFromData(ndims,dims,NPY_DOUBLE,sipCpp->buffer->data);break;
  default: sipPy = NULL; //Not yet managed
}
delete [] dims;
%End
%SetCode
PyArrayObject *tmpObject = (PyArrayObject*)PyArray_ContiguousFromObject(sipPy,NPY_NOTYPE,0,0);
if(!tmpObject) sipErr=1;
else
 {
    switch(PyArray_TYPE(tmpObject))
    {
     case NPY_BYTE:
	sipCpp->type = Data::INT8;break;
     case NPY_UBYTE:
	sipCpp->type = Data::UINT8;break;
     case NPY_SHORT:
	sipCpp->type = Data::INT16;break;
     case NPY_USHORT:
        sipCpp->type = Data::UINT16;break;
     case NPY_INT32:
	sipCpp->type = Data::INT32;break;
     case NPY_UINT:
        sipCpp->type = Data::UINT32;break;
     case NPY_FLOAT32:
	sipCpp->type = Data::FLOAT;break;
     case NPY_DOUBLE:
	sipCpp->type = Data::DOUBLE;break;
     default:
        sipErr=1;
	Py_DECREF(tmpObject);
	break;
    }
 }
if(!sipErr)
{
  int nbDim = PyArray_NDIM(tmpObject);
  sipCpp->dimensions.clear();
  for(int i = nbDim - 1;i >= 0;--i)
    sipCpp->dimensions.push_back(PyArray_DIM(tmpObject,i));
 int aSize = sipCpp->size();
 Buffer *aNewBuffer = new Buffer(aSize);
 memcpy(aNewBuffer->data,PyArray_DATA(tmpObject),aSize);
 Py_DECREF(tmpObject);
 sipCpp->setBuffer(aNewBuffer);
 aNewBuffer->unref();
}
%End
};

  void releaseBuffer();

  SIP_PYOBJECT refcount();
%MethodCode
  Py_BEGIN_ALLOW_THREADS
  sipRes = PyLong_FromLong(sipCpp->buffer ? sipCpp->buffer->refcount : 0);
  Py_END_ALLOW_THREADS
%End

  Data mask() const;

  SIP_PYOBJECT __repr__() const;
%MethodCode
  LIMA_REPR_CODE
%End
};


class TaskEventCallback
{
%TypeHeaderCode
#include "processlib/TaskEventCallback.h"
%End
 public:
  TaskEventCallback();
  virtual void started(Data &);
  virtual void finished(Data &);
  virtual void error(Data &,const char*);
 protected:
  virtual ~TaskEventCallback();
%MethodCode
  Py_BEGIN_ALLOW_THREADS
  sipCpp->unref();
  sipCppV = NULL;
  Py_END_ALLOW_THREADS
%End
};

class LinkTask
{
%TypeHeaderCode
#include "processlib/LinkTask.h"
%End
 public:
  LinkTask(bool = true);
  virtual Data process(Data&);
  void setProcessingInPlace(bool);
  void setEventCallback(TaskEventCallback *);
  TaskEventCallback* getEventCallback();
 protected:
  ~LinkTask();
%MethodCode
 Py_BEGIN_ALLOW_THREADS;
 sipCpp->unref();
 sipCppV = NULL;
 Py_END_ALLOW_THREADS;
%End
};

class SinkTaskBase
{
%TypeHeaderCode
#include "processlib/SinkTask.h"
%End
public:
  virtual void process(Data&);
  void setEventCallback(TaskEventCallback *);
  TaskEventCallback* getEventCallback();
protected:
  virtual ~SinkTaskBase();
%MethodCode
 Py_BEGIN_ALLOW_THREADS;
 sipCpp->unref();
 sipCppV = NULL;
 Py_END_ALLOW_THREADS;
%End
};

class TaskMgr
{
%TypeHeaderCode
#include "processlib/TaskMgr.h"
%End
public:
  class EventCallback
  {
  public:
    virtual void error(Data&,const char*);
  };

  TaskMgr(int priority=0);
  TaskMgr(const TaskMgr&);
  ~TaskMgr();

  void setLinkTask(int aStage,LinkTask *);
  void addSinkTask(int aStage,SinkTaskBase*);
  void setInputData(Data &);
  void setEventCallback(TaskMgr::EventCallback *);
  Data syncProcess();
};

class PoolThreadMgr
{
%TypeHeaderCode
#include "processlib/PoolThreadMgr.h"
%End

 public:
  static PoolThreadMgr& get() throw();

  void addProcess(TaskMgr *,bool = true);
%MethodCode
 Py_BEGIN_ALLOW_THREADS
 sipCpp->addProcess(new TaskMgr(*a0));
 Py_END_ALLOW_THREADS
%End
  void setNumberOfThread(int);
  void setTaskMgr(const TaskMgr *);
  void abort();
  void suspend(bool);
  bool wait(double timeout = -1.);
  void quit();
};

%Feature BPM_SUPPORT
%Include BackgroundSubstraction.sip
%Include Binning.sip
%If (BPM_SUPPORT)
%Include Bpm.sip
%End
%Include FlatfieldCorrection.sip
%Include Flip.sip
%Include Mask.sip
%Include PeakFinder.sip
%Include Roi2Spectrum.sip
%Include RoiCounter.sip
%Include Rotation.sip
%Include SoftRoi.sip
