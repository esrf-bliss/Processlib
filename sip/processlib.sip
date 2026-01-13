//###########################################################################
// This file is part of ProcessLib, a submodule of LImA project the
// Library for Image Acquisition
//
// Copyright (C) : 2009-2025
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
%Module(name=lima.processlib, use_limited_api=True)

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

%End

%PostInitialisationCode
processlib_import_array();
%End

%ModuleHeaderCode
#include <string>
#include <map>
#include <memory>
#include <Python.h>

inline PyObject *sipConvertToPyStr(const std::string& s)
{
    PyObject *newstring;

    // convert an std::string to a Python (unicode) string
    newstring = PyUnicode_DecodeUTF8(s.c_str(), s.length(), NULL);
    if (newstring == NULL) {
        PyErr_Clear();
        newstring = PyUnicode_FromString(s.c_str());
    }
  return newstring;
}

inline bool sipCheckConvertFromPyStr(PyObject *sipPy)
{
    return (PyBytes_Check(sipPy) || PyUnicode_Check(sipPy));
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
        s = PyBytes_AsString(pyS);
        Py_DECREF(pyS);
    } else if (PyBytes_Check(sipPy)) {
        s = PyBytes_AsString(sipPy);
    } else if (sipPy != Py_None) {
        throw std::exception();
    }
    return s;
}

template <class T>
struct ContainerItemIterator
{
    typedef typename T::Map Map;
    typedef typename Map::iterator MapIter;
    typedef typename T::SharedLockedPtr SharedLockedPtr;

    ContainerItemIterator(SharedLockedPtr ref) : _ref(ref)
    {
      if (check()) {
        _cur = _ref->begin();
	_end = _ref->end();
      }
    }

    bool check() { return _ref.isLocked(); }

    MapIter& getCur() { return *(check() ? &_cur : &_end); }
    MapIter& getEnd() { return _end; }

private:
    SharedLockedPtr _ref;
    MapIter _cur;
    MapIter _end;
};


template <class T>
class ContainerContextManager
{
public:
    typedef typename T::SharedLockedPtr SharedLockedPtr;

    static const char *LockedAttrName;

    static PyObject *enter(const sipTypeDef *sipRefDef, PyObject *sipPy,
                           T *sipCpp, int *sipIsErr)
    {
        bool is_locked = inContext(sipRefDef, sipPy, sipIsErr);
        if (is_locked && !*sipIsErr) {
            PyErr_SetString(PyExc_RuntimeError, "Container already running a "
	                                        "ContextManager");
            *sipIsErr = 1;
        }
	if (*sipIsErr)
	    return NULL;
	SharedLockedPtr *cppRef;
	Py_BEGIN_ALLOW_THREADS
	cppRef = new SharedLockedPtr(*sipCpp);
        Py_END_ALLOW_THREADS
	PyObject* pyRef = sipConvertFromNewType(cppRef, sipRefDef, Py_None);
	if (!pyRef) {
            PyErr_SetString(PyExc_RuntimeError, "Cannot create Container "
	                                        "SharedLockedPtr python object");
            delete cppRef;
	    *sipIsErr = 1;
	    return NULL;
        }
        if (PyObject_SetAttrString(sipPy, LockedAttrName, pyRef) != 0) {
            Py_DECREF(pyRef);
	    *sipIsErr = 1;
	    return NULL;
	}
        return pyRef;
    }

    static void exit(const sipTypeDef *sipRefDef, PyObject *sipPy, int *sipIsErr)
    {
        if (!PyObject_HasAttrString(sipPy, LockedAttrName)) {
	    PyErr_WarnEx(PyExc_RuntimeWarning, "Container ContextManager "
	                "does not have locked attribute", 1);
	    return;
        }
        PyObject *pyRef = PyObject_GetAttrString(sipPy, LockedAttrName);
	if (!pyRef) {
            PyErr_Format(PyExc_RuntimeError, "Cannot get %s Container "
	                 "ContextManager attribute", LockedAttrName);
	    *sipIsErr = 1;
	    return;
	}
	PyObject *res = PyObject_CallMethod(pyRef, "unLock", NULL);
	if (!res) {
	    *sipIsErr = 1;
	} else {
	    Py_DECREF(res);
            if (PyObject_DelAttrString(sipPy, LockedAttrName) != 0)
                *sipIsErr = 1;
	}
	Py_DECREF(pyRef);
    }

    static bool inContext(const sipTypeDef *sipRefDef, PyObject *sipPy,
                          int *sipIsErr)
    {
        if (!PyObject_HasAttrString(sipPy, LockedAttrName))
	    return false;
        PyObject *pyRef = PyObject_GetAttrString(sipPy, LockedAttrName);
	if (!pyRef) {
            PyErr_Format(PyExc_RuntimeError, "Cannot get %s Container "
	                 "ContextManager attribute", LockedAttrName);
	    *sipIsErr = 1;
	    return false;
	}
	bool is_locked = false;
	PyObject *locked = PyObject_CallMethod(pyRef, "isLocked", NULL);
	if (!locked) {
	    *sipIsErr = 1;
	} else {
            if (!PyBool_Check(locked)) {
                PyErr_SetString(PyExc_RuntimeError, "Container.SharedLockedPtr.isLocked "
		                "returned non-boolean object");
                *sipIsErr = 1;
	    } else {
                is_locked = (locked == Py_True);
	    }
            Py_DECREF(locked);
	}
	Py_DECREF(pyRef);
	return is_locked;
    }

    // this function must be called only if inContext
    // it will rise an exception otherwise
    static SharedLockedPtr getLockedPtr(const sipTypeDef *sipRefDef, PyObject *sipPy)
    {
        if (!PyObject_HasAttrString(sipPy, LockedAttrName))
            throw ProcessException("Container ContextManager not active");
        PyObject *pyRef = PyObject_GetAttrString(sipPy, LockedAttrName);
	if (!pyRef)
	    throw ProcessException("Cannot get Container ContextManager attr");
        int state, sipIsErr = 0;
	void *p = sipConvertToType(pyRef, sipRefDef, NULL, SIP_NOT_NONE,
	                           &state, &sipIsErr);
	if (!p || sipIsErr) {
	    if (p)
	       sipReleaseType(p, sipRefDef, state);
	    Py_DECREF(pyRef);
	    throw ProcessException("Cannot convert to C++ LocketPtr type");
        }
	SharedLockedPtr *cppRef = (SharedLockedPtr *) p;
	SharedLockedPtr res = *cppRef;
	sipReleaseType(cppRef, sipRefDef, state);
	Py_DECREF(pyRef);
	return res;
    }
};

template <class T>
const char *ContainerContextManager<T>::LockedAttrName = "__locked_ptr";

typedef ContainerItemIterator<Data::HeaderContainer> Data_HeaderContainer_itemIterator;

%End


%UnitCode

#ifndef LIMA_REPR_CODE
#define LIMA_REPR_CODE \
	do { \
		std::ostringstream str; \
		str << *sipCpp;	\
		sipRes = sipConvertToPyStr(str.str()); \
	} while (0);
#endif //LIMA_REPR_CODE

#ifdef _DEBUG
# ifdef _MSC_VER  
   // VC8.0 will complain if system headers are #included both with
   // and without _DEBUG defined, so we have to #include all the
   // system headers used by pyconfig.h right here.
#  include <stddef.h>
#  include <stdarg.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <assert.h>
#  include <errno.h>
#  include <ctype.h>
#  include <wchar.h>
#  include <basetsd.h>
#  include <io.h>
#  include <limits.h>
#  include <float.h>
#  include <string.h>
#  include <math.h>
#  include <time.h>
# endif
# undef _DEBUG // Don't let Python force the debug library just because we're debugging.
# define DEBUG_WRAP_PYTHON_H
#endif

#include <Python.h>

#ifdef DEBUG_WRAP_PYTHON_H
# define _DEBUG
#endif

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
  Data_HeaderContainer_itemIterator(Data::HeaderContainer::SharedLockedPtr ref);

  bool check();

  SIP_PYOBJECT __next__();
  %MethodCode
  if (!sipCpp->check()) {
    PyErr_SetString(PyExc_RuntimeError, "HeaderContainer::SharedLockedPtr un-locked "
                    "during item iteration");
    sipIsErr = 1;
  } else {
    Data_HeaderContainer_itemIterator::MapIter& cur = sipCpp->getCur();
    if(cur != sipCpp->getEnd()) {
      sipRes = PyTuple_Pack(2, sipConvertToPyStr(cur->first),
			       sipConvertToPyStr(cur->second));
      ++cur;
    } else {
      PyErr_SetString(PyExc_StopIteration,"");
    }
  }
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
    %TypeHeaderCode
    #ifndef SIP_DATA_HEADERCONTAINER
    #define SIP_DATA_HEADERCONTAINER

    typedef Data::HeaderContainer::Map HeaderMap;

    inline PyObject *sipDataHeaderContainerRepr(
                         Data::HeaderContainer::SharedLockedPtr headerPtr)
    {
        std::stringstream str;
        str << "{";
        HeaderMap::iterator it, end = headerPtr->end();
        bool first = true;
        for (it = headerPtr->begin(); it != end; ++it, first = false) {
            if (!first)
                str << ", ";
            str << "'" << it->first << "': ";
            str << "'" << it->second << "'";
        }
        str << "}";
        return sipConvertToPyStr(str.str());
    }

    inline PyObject *sipDataHeaderContainerValues(
                         Data::HeaderContainer::SharedLockedPtr headerPtr)
    {
        PyObject *sipRes = PyList_New(headerPtr->size());
        int index = 0;
        HeaderMap::iterator it, end = headerPtr->end();
        for (it = headerPtr->begin(); it != end; ++it, ++index)
            PyList_SetItem(sipRes, index, sipConvertToPyStr(it->second));
        return sipRes;
    }

    inline PyObject *sipDataHeaderContainerPop(
                         Data::HeaderContainer::SharedLockedPtr headerPtr,
			 PyObject *pyKey, PyObject *defVal, int *sipIsErr)
    {
        if (!sipCheckConvertFromPyStr(pyKey) ||
	    (defVal != NULL && !sipCheckConvertFromPyStr(defVal))) {
            PyErr_SetString(PyExc_TypeError,
	                    "Only string key and string value are supported");
            *sipIsErr = 1;
	    return NULL;
        }

        PyObject *sipRes = NULL;
        std::string cppKey = sipConvertFromPyStr(pyKey);
        HeaderMap::iterator it = headerPtr->find(cppKey);
        if (it != headerPtr->end()) {
            sipRes = sipConvertToPyStr(it->second);
            headerPtr->erase(it);
        } else if (defVal) {
            Py_INCREF(defVal);
            sipRes = defVal;
        } else {
            PyErr_Format(PyExc_KeyError,"Header has no key %s", cppKey.c_str());
            *sipIsErr = 1;
        }

        return sipRes;
    }

    inline void sipDataHeaderContainerUpdate(
                    Data::HeaderContainer::SharedLockedPtr headerPtr,
		    PyObject *dict, int *sipIsErr)
    {
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(dict, &pos, &key, &value)) {
            if (!sipCheckConvertFromPyStr(key) ||
                !sipCheckConvertFromPyStr(value)) {
                PyErr_SetString(PyExc_TypeError,
	                        "Only string key and string value are supported");
                *sipIsErr = 1;
                return;
            }
        }

        pos = 0;
        while (PyDict_Next(dict, &pos, &key, &value)) {
	    std::string& item = (*headerPtr)[sipConvertFromPyStr(key).c_str()];
	    item = sipConvertFromPyStr(value).c_str();
        }
    }

    inline PyObject *sipDataHeaderContainerKeys(
                         Data::HeaderContainer::SharedLockedPtr headerPtr)
    {
        PyObject *sipRes = PyList_New(headerPtr->size());
        int index = 0;
        HeaderMap::iterator it, end = headerPtr->end();
        for (it = headerPtr->begin(); it != end; ++it, ++index)
            PyList_SetItem(sipRes, index, sipConvertToPyStr(it->first));
	return sipRes;
    }

    inline PyObject *sipDataHeaderContainerItems(
                         Data::HeaderContainer::SharedLockedPtr headerPtr)
    {
        PyObject *sipRes = PyList_New(headerPtr->size());
        int index = 0;
        HeaderMap::iterator it, end = headerPtr->end();
        for (it = headerPtr->begin(); it != end; ++it, ++index) {
            PyObject *val = PyTuple_Pack(2, sipConvertToPyStr(it->first),
    				        sipConvertToPyStr(it->second));
            PyList_SetItem(sipRes, index, val);
        }
	return sipRes;
    }

    inline bool sipDataHeaderContainerHasKey(
                    Data::HeaderContainer::SharedLockedPtr headerPtr,
		    PyObject *pyKey, int *sipIsErr)
    {
        if (!sipCheckConvertFromPyStr(pyKey)) {
            PyErr_Format(PyExc_TypeError,
	                 "Only string key is supported, but got %s",
                         Py_TYPE(pyKey)->tp_name);    
            *sipIsErr = 1;
	    return false;
        }
        std::string key = sipConvertFromPyStr(pyKey);
        return (headerPtr->find(key) != headerPtr->end());
    }

    inline void sipDataHeaderContainerSetItem(
                    Data::HeaderContainer::SharedLockedPtr headerPtr,
		    PyObject *pyKey, PyObject *pyVal, int *sipIsErr)
    {
        if (!sipCheckConvertFromPyStr(pyKey) ||
	    !sipCheckConvertFromPyStr(pyVal)) {
            PyErr_SetString(PyExc_TypeError,
	                    "Only string key and string value are supported");
	    *sipIsErr = 1;
	    return;
        }
        std::string key, value;
        key = sipConvertFromPyStr(pyKey); value = sipConvertFromPyStr(pyVal);
        (*headerPtr)[key] = value;
    }

    inline PyObject *sipDataHeaderContainerGetItem(
                         Data::HeaderContainer::SharedLockedPtr headerPtr,
		         PyObject *pyKey, int *sipIsErr)
    {
        if (!sipCheckConvertFromPyStr(pyKey)) {
            PyErr_SetString(PyExc_TypeError,
	                    "Only string key and string value are supported");
	    *sipIsErr = 1;
	    return NULL;
        }
        std::string key = sipConvertFromPyStr(pyKey);
	HeaderMap::iterator it = headerPtr->find(key);
	if (it == headerPtr->end()) {
            PyErr_Format(PyExc_KeyError,"Header has no key %s", key.c_str());
            *sipIsErr = 1;
	    return NULL;
        }
        return sipConvertToPyStr(it->second);
    }

    inline PyObject *sipDataHeaderContainerGet(
                         Data::HeaderContainer::SharedLockedPtr headerPtr,
			 PyObject *pyKey, PyObject *defVal, int *sipIsErr)
    {
        if (!sipCheckConvertFromPyStr(pyKey) ||
	    (defVal != NULL && !sipCheckConvertFromPyStr(defVal))) {
            PyErr_SetString(PyExc_TypeError,
	                    "Only string key and string value are supported");
            *sipIsErr = 1;
	    return NULL;
        }

	PyObject *sipRes;
        std::string key = sipConvertFromPyStr(pyKey);
	HeaderMap::iterator it = headerPtr->find(key);
        if (it != headerPtr->end()) {
           sipRes = sipConvertToPyStr(it->second);
        } else if (defVal) {
            Py_INCREF(defVal);
            sipRes = defVal;
        } else {
            Py_INCREF(Py_None);
            sipRes = Py_None;
        }

        return sipRes;
    }

    typedef
    ContainerContextManager<Data::HeaderContainer> HeaderContextManager;
    #endif //SIP_DATA_HEADERCONTAINER
    %End

  public:
    class SharedLockedPtr
    {
    public:
	SharedLockedPtr(const SharedLockedPtr& o);
	SharedLockedPtr(const Data::HeaderContainer& cont);

	bool isLocked() const;
	void unLock();

        SIP_PYOBJECT __repr__() const;
        %MethodCode
	if (!sipCpp->isLocked())
            sipRes = sipConvertToPyStr("<Null LockRef>");
        else
            sipRes = sipDataHeaderContainerRepr(*sipCpp);
        %End

        SIP_PYOBJECT __str__();
        %MethodCode
	if (!sipCpp->isLocked())
            sipRes = sipConvertToPyStr("<Null LockRef>");
        else
            sipRes = sipDataHeaderContainerRepr(*sipCpp);
        %End

        SIP_PYOBJECT values();
        %MethodCode
	if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = sipDataHeaderContainerValues(*sipCpp);
        %End

        SIP_PYOBJECT pop(SIP_PYOBJECT,SIP_PYOBJECT = NULL);
        %MethodCode
	if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = sipDataHeaderContainerPop(*sipCpp, a0, a1, &sipIsErr);
        %End

        void update(SIP_PYDICT);
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipDataHeaderContainerUpdate(*sipCpp, a0, &sipIsErr);
        %End

        SIP_PYOBJECT keys();
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = sipDataHeaderContainerKeys(*sipCpp);
        %End

        SIP_PYOBJECT items();
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = sipDataHeaderContainerItems(*sipCpp);
        %End

        Data_HeaderContainer_itemIterator iteritems();
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = new Data_HeaderContainer_itemIterator(*sipCpp);
        %End

        bool has_key(SIP_PYOBJECT);
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = sipDataHeaderContainerHasKey(*sipCpp, a0, &sipIsErr);
        %End

        Py_ssize_t __len__();
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else {
            sipRes = (*sipCpp)->size();
	}
        %End

        void  __setitem__(SIP_PYOBJECT, SIP_PYOBJECT);
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipDataHeaderContainerSetItem(*sipCpp, a0, a1, &sipIsErr);
        %End

        SIP_PYOBJECT __getitem__(SIP_PYOBJECT) const;
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = sipDataHeaderContainerGetItem(*sipCpp, a0, &sipIsErr);
        %End

        SIP_PYOBJECT get(SIP_PYOBJECT, SIP_PYOBJECT = NULL) const;
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = sipDataHeaderContainerGet(*sipCpp, a0, a1, &sipIsErr);
        %End
    };

    HeaderContainer();

    SIP_PYOBJECT pop(SIP_PYOBJECT,SIP_PYOBJECT = NULL);
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = sipDataHeaderContainerPop(headerPtr, a0, a1, &sipIsErr);
    %End

    void update(SIP_PYDICT);
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipDataHeaderContainerUpdate(headerPtr, a0, &sipIsErr);
    %End

    SIP_PYOBJECT __repr__() const;
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = sipDataHeaderContainerRepr(headerPtr);
    %End

    SIP_PYOBJECT __str__();
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = sipDataHeaderContainerRepr(headerPtr);
    %End

    SIP_PYOBJECT __enter__();
    %MethodCode
    const sipTypeDef *refTypeDef = sipType_Data_HeaderContainer_SharedLockedPtr;
    sipRes = HeaderContextManager::enter(refTypeDef, sipSelf, sipCpp, &sipIsErr);
    %End

    void __exit__(SIP_PYOBJECT,SIP_PYOBJECT,SIP_PYOBJECT);
    %MethodCode
    const sipTypeDef *refTypeDef = sipType_Data_HeaderContainer_SharedLockedPtr;
    HeaderContextManager::exit(refTypeDef, sipSelf, &sipIsErr);
    %End

    SIP_PYOBJECT values();
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = sipDataHeaderContainerValues(headerPtr);
    %End

    SIP_PYOBJECT keys();
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = sipDataHeaderContainerKeys(headerPtr);
    %End

    SIP_PYOBJECT items();
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = sipDataHeaderContainerItems(headerPtr);
    %End

    Data_HeaderContainer_itemIterator iteritems();
    %MethodCode
    typedef HeaderContextManager::SharedLockedPtr SharedLockedPtr;
    const sipTypeDef *refTypeDef = sipType_Data_HeaderContainer_SharedLockedPtr;
    try {
        SharedLockedPtr ref = HeaderContextManager::getLockedPtr(refTypeDef, sipSelf);
        sipRes = new Data_HeaderContainer_itemIterator(ref);
    } catch (ProcessException& e) {
        PyErr_Format(PyExc_RuntimeError, "Error getting lock: %s",
	             e.getErrMsg().c_str());
        sipIsErr = 1;
    }
    %End

    bool has_key(SIP_PYOBJECT);
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = sipDataHeaderContainerHasKey(headerPtr, a0, &sipIsErr);
    %End

    Py_ssize_t __len__();
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = headerPtr->size();
    %End

    void  __setitem__(SIP_PYOBJECT, SIP_PYOBJECT);
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipDataHeaderContainerSetItem(headerPtr, a0, a1, &sipIsErr);
    %End

    SIP_PYOBJECT __getitem__(SIP_PYOBJECT) const;
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = sipDataHeaderContainerGetItem(headerPtr, a0, &sipIsErr);
    %End

    SIP_PYOBJECT get(SIP_PYOBJECT, SIP_PYOBJECT = NULL) const;
    %MethodCode
    Data::HeaderContainer::SharedLockedPtr headerPtr(*sipCpp);
    sipRes = sipDataHeaderContainerGet(headerPtr, a0, a1, &sipIsErr);
    %End

    int size() const;
    void clear();
  };

  class SidebandContainer
  {
    %TypeHeaderCode
    #ifndef SIP_DATA_SIDEBANDCONTAINER
    #define SIP_DATA_SIDEBANDCONTAINER

    typedef Data::SidebandContainer::Map SidebandMap;

    inline PyObject *sipDataSidebandContainerRepr(
                         Data::SidebandContainer::SharedLockedPtr sidebandPtr)
    {
        std::stringstream str;
        str << "{";
        SidebandMap::iterator it, end = sidebandPtr->end();
        bool first = true;
        for (it = sidebandPtr->begin(); it != end; ++it, first = false) {
            if (!first)
                str << ", ";
            str << "'" << it->first << "': ";
            str << "'" << it->second->repr() << "'";
        }
        str << "}";
        return sipConvertToPyStr(str.str());
    }

    inline PyObject *sipDataSidebandContainerKeys(
                         Data::SidebandContainer::SharedLockedPtr sidebandPtr)
    {
        PyObject *sipRes = PyList_New(sidebandPtr->size());
        int index = 0;
        SidebandMap::iterator it, end = sidebandPtr->end();
        for (it = sidebandPtr->begin(); it != end; ++it, ++index)
            PyList_SetItem(sipRes, index, sipConvertToPyStr(it->first));
	return sipRes;
    }

    inline bool sipDataSidebandContainerHasKey(
                    Data::SidebandContainer::SharedLockedPtr sidebandPtr,
		    PyObject *pyKey, int *sipIsErr)
    {
        if (!sipCheckConvertFromPyStr(pyKey)) {
            PyErr_Format(PyExc_TypeError,
	                 "Only string key is supported, but got %s",
                         Py_TYPE(pyKey)->tp_name);    
            *sipIsErr = 1;
	    return false;
        }
        std::string key = sipConvertFromPyStr(pyKey);
        return (sidebandPtr->find(key) != sidebandPtr->end());
    }

    typedef
    ContainerContextManager<Data::SidebandContainer> SidebandContextManager;

    #endif //SIP_DATA_SIDEBANDCONTAINER
    %End

  public:
    class SharedLockedPtr
    {
    public:
	SharedLockedPtr(const SharedLockedPtr& o);
	SharedLockedPtr(const Data::SidebandContainer& cont);

	bool isLocked();
	void unLock();

        SIP_PYOBJECT __repr__() const;
        %MethodCode
        if (!sipCpp->isLocked())
            sipRes = sipConvertToPyStr("<Null LockRef>");
        else
            sipRes = sipDataSidebandContainerRepr(*sipCpp);
        %End

        SIP_PYOBJECT __str__();
        %MethodCode
        if (!sipCpp->isLocked())
            sipRes = sipConvertToPyStr("<Null LockRef>");
        else
            sipRes = sipDataSidebandContainerRepr(*sipCpp);
        %End

        SIP_PYOBJECT keys();
        %MethodCode
	if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = sipDataSidebandContainerKeys(*sipCpp);
        %End

        bool has_key(SIP_PYOBJECT);
        %MethodCode
	if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else
  	    sipRes = sipDataSidebandContainerHasKey(*sipCpp, a0, &sipIsErr);
        %End

        Py_ssize_t __len__();
        %MethodCode
        if (!sipCpp->isLocked()) {
	    PyErr_SetString(PyExc_ValueError, "Null LockRef");
	    sipIsErr = 1;
	} else {
            sipRes = (*sipCpp)->size();
	}
        %End
    };

    SidebandContainer();

    SIP_PYOBJECT __enter__();
    %MethodCode
    const sipTypeDef *refTypeDef = sipType_Data_SidebandContainer_SharedLockedPtr;
    sipRes = SidebandContextManager::enter(refTypeDef, sipSelf, sipCpp, &sipIsErr);
    %End

    void __exit__(SIP_PYOBJECT,SIP_PYOBJECT,SIP_PYOBJECT);
    %MethodCode
    const sipTypeDef *refTypeDef = sipType_Data_SidebandContainer_SharedLockedPtr;
    SidebandContextManager::exit(refTypeDef, sipSelf, &sipIsErr);
    %End

    int size() const;
    void clear();
  };

  enum TYPE { UNDEF,UINT8,INT8,UINT16,INT16,UINT32,INT32,UINT64,INT64,FLOAT,DOUBLE };

  int frameNumber;
  double timestamp;
  Data::HeaderContainer header;
  Data::SidebandContainer sideband;
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

namespace sideband {

struct Data /Abstract/
{
%TypeHeaderCode
#include "processlib/sideband/Data.h"
%End
  virtual ~Data();

  SIP_PYOBJECT __repr__() const;
%MethodCode
  sipRes = sipConvertToPyStr(sipCpp->repr());
%End

};

}; //namespace sideband

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
  static PoolThreadMgr& get();

  void addProcess(TaskMgr *,bool = true);
%MethodCode
 Py_BEGIN_ALLOW_THREADS
 sipCpp->addProcess(new TaskMgr(*a0));
 Py_END_ALLOW_THREADS
%End
  void setNumberOfThread(int);
  int getNumberOfThread();
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
%Include RoiCollection.sip
