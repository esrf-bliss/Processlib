%Module processlib 0

%ModuleHeaderCode
extern "C" { void processlib_import_array();}
%End

%PostInitialisationCode
processlib_import_array();
%End

%Exception Data_Header_Iter_Stop_iteration /PyName=StopIteration/
{
%TypeHeaderCode
#include "data_header_iterator.h"
#include <iostream>
%End
%RaiseCode
std::cout << "raise stop " << std::endl;
        SIP_BLOCK_THREADS
        PyErr_SetString(PyExc_Exception,"");
        SIP_UNBLOCK_THREADS
%End
};

struct Data_HeaderContainer_itemIterator
{
%TypeHeaderCode
#include "data_header_iterator.h"
%End

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
#include <Data.h>
#include <PoolThreadMgr.h>
#include <sstream>
#include "data_header_iterator.h"
%End

  class HeaderContainer
  {
  public:
    HeaderContainer();
    

SIP_PYOBJECT pop(const char *,const char * = NULL);
%MethodCode
PoolThreadMgr::Lock aLock(sipCpp->mutex());
Data::HeaderContainer::Header &header = sipCpp->header();
std::map<std::string,std::string>::iterator i = header.find(a0);
if(i != header.end())
  sipRes = PyString_FromStringAndSize(i->second.c_str(),i->second.size());
else if (a1)
  sipRes = PyString_FromString(a1);
else
  {
   Py_INCREF(Py_None);
   sipRes = Py_None;
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
  const char* keystr = PyString_AsString(key);
  const char* valuestr = PyString_AsString(value);
  if(!keystr || !valuestr)
   {
    std::cout << "Only string key and value are allowed" << std::endl;
    break;
   }
std::pair<std::map<std::string,std::string>::iterator, bool> result = 
header.insert(std::pair<std::string,std::string>(PyString_AsString(key),PyString_AsString(value)));
if(!result.second)
  result.first->second = PyString_AsString(value);
 }
%End

const char* __repr__();
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
sipRes = str.str().c_str();
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
sipRes = str.str().c_str();
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

bool has_key(const char*);
%MethodCode
PoolThreadMgr::Lock aLock(sipCpp->mutex());
Data::HeaderContainer::Header &header = sipCpp->header();
std::map<std::string,std::string>::iterator i = header.find(a0);
sipRes = i != header.end();
%End

int __len__();
%MethodCode
sipRes = sipCpp->size();
%End

void  __setitem__(const char*,const char*);
%MethodCode
sipCpp->insert(a0,a1);
%End

    const char* get(const char *key,const char *defaultValue = NULL) const;
    const char* operator[](const char *aKey) const;
    int size() const;
    void clear();
  };
  int frameNumber;
  double timestamp;
  Data::HeaderContainer header;
  void *buffer;
%GetCode
if(!sipCpp->buffer || !sipCpp->buffer->data)
   Py_RETURN_NONE;

npy_intp dims[] = {sipCpp->height,sipCpp->width};

switch(sipCpp->type)
{
  case Data::UINT8: sipPy = PyArray_SimpleNewFromData(2,dims,NPY_UBYTE,sipCpp->buffer->data);break;
  case Data::INT8: sipPy = PyArray_SimpleNewFromData(2,dims,NPY_BYTE,sipCpp->buffer->data);break;
  case Data::UINT16: sipPy = PyArray_SimpleNewFromData(2,dims,NPY_USHORT,sipCpp->buffer->data);break;
  case Data::INT16: sipPy = PyArray_SimpleNewFromData(2,dims,NPY_SHORT,sipCpp->buffer->data);break;
  case Data::UINT32: sipPy = PyArray_SimpleNewFromData(2,dims,NPY_UINT,sipCpp->buffer->data);break;
  case Data::INT32: sipPy = PyArray_SimpleNewFromData(2,dims,NPY_INT32,sipCpp->buffer->data);break;
  case Data::FLOAT: sipPy = PyArray_SimpleNewFromData(2,dims,NPY_FLOAT32,sipCpp->buffer->data);break;
  case Data::DOUBLE: sipPy = PyArray_SimpleNewFromData(2,dims,NPY_DOUBLE,sipCpp->buffer->data);break;
  default: return NULL; //Not yet managed
}
%End
%SetCode
PyArrayObject *tmpObject = (PyArrayObject*)PyArray_ContiguousFromObject(sipPy,NPY_NOTYPE,0,0);
#if(SIP_VERSION >= 0x041000)
if(!tmpObject) return -1; //should throw an error
#else
if(!tmpObject) return NULL; //should throw an error
#endif
int nbDim = PyArray_NDIM(tmpObject);
#if(SIP_VERSION >= 0x041000)
if(nbDim > 2) return -1;
#else
if(nbDim > 2) return NULL;
#endif
int width,height;
height =  PyArray_DIM(tmpObject,0);
if(nbDim < 2) width = 1;
else width = PyArray_DIM(tmpObject,1);

switch(tmpObject->descr->type_num)
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
#if(SIP_VERSION >= 0x041000)
        return -1;
#else
	return NULL;
#endif
}
sipCpp->width = width,sipCpp->height = height;
int aSize = sipCpp->size();
Buffer *aNewBuffer = new Buffer(aSize);
memcpy(aNewBuffer->data,PyArray_DATA(tmpObject),aSize);
Py_DECREF(tmpObject);
sipCpp->setBuffer(aNewBuffer);
aNewBuffer->unref();
%End

%TypeCode
#define PY_ARRAY_UNIQUE_SYMBOL _ProcessLibNumpyArray
#include "numpy/arrayobject.h"
#include <iostream>
using namespace std;
extern "C"
{
  void processlib_import_array()
  {
    import_array();
  }
}
%End
  SIP_PYOBJECT refcount();
%MethodCode
  Py_BEGIN_ALLOW_THREADS
  sipRes = PyLong_FromLong(sipCpp->buffer ? sipCpp->buffer->refcount : 0);
  Py_END_ALLOW_THREADS
%End

  const char* __repr__();
%MethodCode
	std::ostringstream str;
	str << *sipCpp;	
	sipRes = str.str().c_str();
%End
};


class TaskEventCallback
{
%TypeHeaderCode
#include <TaskEventCallback.h>
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
#include <LinkTask.h>
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
#include <SinkTask.h>
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
#include <TaskMgr.h>
%End
public:
  TaskMgr();
  TaskMgr(const TaskMgr&);
  ~TaskMgr();

  void setLinkTask(int aStage,LinkTask *);
  void addSinkTask(int aStage,SinkTaskBase*);
  void setInputData(Data &);
  void syncProcess();
};

class PoolThreadMgr
{
%TypeHeaderCode
#include <PoolThreadMgr.h>
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
  void setQueueLimit(int);
  int  queueLimit();
  void setTaskMgr(const TaskMgr *);
  void abort();
  void suspend(bool);
  bool wait(double timeout = -1.);
  void quit();
};
