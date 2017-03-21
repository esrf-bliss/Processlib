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
#ifndef __unix
#pragma warning(disable:4251)
#pragma warning(disable:4244)
#endif

#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <map>
#include <vector>
#include <string>
#include <cstring>

#include "processlib/Compatibility.h"
#include "processlib/ProcessExceptions.h"

#ifndef __DATA_H
#define __DATA_H
struct DLL_EXPORT Buffer
{
  class DLL_EXPORT Callback
  {
  public:
    virtual ~Callback() {}
    virtual void destroy(void *dataPt) = 0;
  };

  enum Ownership {MAPPED,SHARED};
  ~Buffer()
  {
    if(callback)
      callback->destroy(data);
    pthread_mutex_destroy(&_lock);
  }
  Buffer() : owner(SHARED),refcount(1),data(NULL),callback(NULL)
  {
    pthread_mutex_init(&_lock,NULL);
  }
  explicit Buffer(int aSize) :owner(SHARED),refcount(1),callback(NULL)
  {
#ifdef __unix
    if(posix_memalign(&data,16,aSize))
#else  /* window */
    data = _aligned_malloc(aSize,16);
    if(!data)
#endif
      std::cerr << "Can't allocate memory" << std::endl;
    pthread_mutex_init(&_lock,NULL);
  }
  void unref()
  {
    while(pthread_mutex_lock(&_lock)) ;
    if(!(--refcount))
      {
	if(owner == SHARED && data)
#ifdef __unix
	  free(data);
#else
	_aligned_free(data);
#endif
	pthread_mutex_unlock(&_lock);
	delete this;
      }
    else
      pthread_mutex_unlock(&_lock);
  }
  void ref()
  {
    while(pthread_mutex_lock(&_lock)) ;
    ++refcount;
    pthread_mutex_unlock(&_lock);
  }
  Ownership		owner;
  volatile int          refcount;
  void			*data;
  pthread_mutex_t	_lock;
  Callback*		callback;
};
struct DLL_EXPORT Data
{
  class DLL_EXPORT HeaderContainer
  {
  public:
    typedef std::map<std::string,std::string> Header;
    
    HeaderContainer();
    HeaderContainer(const HeaderContainer &cnt);
    ~HeaderContainer();
    
    void insert(const char *key,const char *value);
    void insertOrIncKey(const std::string &key,const std::string &value);
    void erase(const char *key);
    void clear();

    const char* get(const char *key,const char *defaultValue = NULL) const;
    int size() const;

    const char* operator[](const char *aKey) const {return get(aKey);}

    HeaderContainer& operator=(const HeaderContainer&);

    // ExpertMethodes for macro insertion a loop
    void lock();
    void unlock();
    pthread_mutex_t* mutex() const;
    Header& header();
    const Header& header() const;
  private:
    struct HeaderHolder;
    HeaderHolder *_header;
  };
  enum TYPE {UNDEF,UINT8,INT8,UINT16,INT16,UINT32,INT32,UINT64,INT64,FLOAT,DOUBLE};
  Data() : type(UNDEF),frameNumber(-1),timestamp(0.),buffer(NULL) {}
  Data(const Data &aData) : buffer(NULL)
  {
    *this = aData;
  }
  ~Data() 
  {
    if(buffer)
      buffer->unref();
  }
  void setData(Data &aData)
  {
    setBuffer(aData.buffer);
    type = aData.type;
    dimensions = aData.dimensions;
  }
  void setBuffer(Buffer *aBuffer)
  {
    if(aBuffer) aBuffer->ref();
    if(buffer) buffer->unref();
    buffer = aBuffer;
  }
  void releaseBuffer() 
  {
    if(buffer) buffer->unref();
    buffer = NULL;
    type = UNDEF,frameNumber = -1;
    dimensions.clear();
  }
  inline void * data() {return buffer ? buffer->data : NULL;}
  inline const void * data() const {return buffer ? buffer->data : NULL;}
  inline int size() const 
  {
    int returnSize = depth();
    for(std::vector<int>::const_iterator i = dimensions.begin();
	i != dimensions.end();++i)
      returnSize *= *i;
    return returnSize;
  }
  inline int depth() const
  {
    int depth;
    switch(type)
      {
      case UINT8:
      case INT8:
	depth = 1;break;
      case UINT16:
      case INT16:
	depth = 2;break;
      case UINT32:
      case INT32:
	depth = 4;break;
      case UINT64:
      case INT64:
	depth = 8;break;
      case FLOAT: depth = 4;break;
      case DOUBLE: depth = 8;break;
      default: 
	depth = 0;break;
      }
    return depth;
  }
  inline bool is_signed() const
  {
    bool signedFlag;
    switch(type)
      {
      case UINT8:
      case UINT16:
      case UINT32:
      case UINT64:
	signedFlag = false;break;
      default:
	signedFlag = true;break;
      }
    return signedFlag;
  }

  inline bool empty() const { return buffer ? !buffer->data : true;}
  //@brief return a data with the same header but with different buffer.
  inline Data copyHeader(TYPE aType)
  {
    Data aReturnData;
    aReturnData.type = aType;
    aReturnData.dimensions = dimensions;
    aReturnData.frameNumber = frameNumber;
    aReturnData.timestamp = timestamp;
    aReturnData.header = header;
    aReturnData.buffer = new Buffer(aReturnData.size());
    return aReturnData;
  }
  inline Data copy() const
  {
    Data aReturnData;
    aReturnData.type = type;
    aReturnData.dimensions = dimensions;
    aReturnData.frameNumber = frameNumber;
    aReturnData.timestamp = timestamp;
    aReturnData.header = header;
    if(buffer)
      {
	aReturnData.buffer = new Buffer(size());
	memcpy(aReturnData.data(),data(),size());
      }
    else
      aReturnData.buffer = NULL;
    return aReturnData;
  }
  inline Data cast(Data::TYPE aType);
  inline Data mask() const
  {
    Data aReturnData;
    aReturnData.dimensions = dimensions;
    aReturnData.frameNumber = frameNumber;
    aReturnData.timestamp = timestamp;
    aReturnData.header = header;
    if(depth() != 1)
      {
	aReturnData.type = INT8;
	aReturnData.buffer = new Buffer(aReturnData.size());
	switch(type)
	  {
	  case UINT16:
	    _make_mask<unsigned short>(*this,aReturnData);break;
	  case INT16:
	    _make_mask<short>(*this,aReturnData);break;
	  case UINT32:
	    _make_mask<unsigned int>(*this,aReturnData);break;
	  case INT32:
	    _make_mask<int>(*this,aReturnData);break;
	  case UINT64:
	    _make_mask<unsigned long long>(*this,aReturnData);break;
	  case INT64:
	    _make_mask<long long>(*this,aReturnData);break;
	  default:
	    return Data();	/* Not managed should not happen */
	  }
      }
    else
      {
	aReturnData.type = type;
	aReturnData.setBuffer(buffer);
      }
    return aReturnData;
  }

  inline Data& operator=(const Data &aData)
    {
      type = aData.type;
      dimensions = aData.dimensions;
      frameNumber = aData.frameNumber;
      timestamp = aData.timestamp;
      header    = aData.header;
      setBuffer(aData.buffer);
      return *this;
    }

  TYPE      			type;
  std::vector<int> 		dimensions;
  int       			frameNumber;
  double    			timestamp;
  mutable HeaderContainer 	header;
  mutable Buffer *		buffer;

 private:
  template<class INPUT>
    static void _make_mask(const Data &src,Data &dst)
    {
      const INPUT *aSrcPt;
      aSrcPt = (const INPUT*)src.data();
      char *aDstPt = (char*)dst.data();

      int pixelnb = src.size() / src.depth();
      for(int i = 0;i < pixelnb;++i,++aSrcPt,++aDstPt)
	*aDstPt = char(*aSrcPt);
    }
  template<class OUTPUT,class INPUT> 
    static void _cast(OUTPUT* dst,INPUT* src,int nbItems)
  {
    while(nbItems)
      {
	*dst = OUTPUT(*src);
	++dst,++src,--nbItems;
      }
  }
};

Data Data::cast(Data::TYPE aType)
{
  if(aType == type)
    return *this;

  Data aReturnData;
  aReturnData.type = aType;
  aReturnData.dimensions = dimensions;
  aReturnData.frameNumber = frameNumber;
  aReturnData.timestamp = timestamp;
  aReturnData.header = header;

  aReturnData.buffer = new Buffer(aReturnData.size());
  int nbItems = aReturnData.size() / aReturnData.depth();
    
  switch(type)
    {
    case UINT8:
      switch(aReturnData.type)
	{
	case UINT16: 
	  Data::_cast((unsigned short*)aReturnData.data(),
		      (unsigned char*)data(),nbItems);
	  break;
	case INT16:
	  Data::_cast((short*)aReturnData.data(),
		      (unsigned char*)data(),nbItems);
	  break;
	case UINT32:	    
	  Data::_cast((unsigned int*)aReturnData.data(),
		      (unsigned char*)data(),nbItems);
	  break;
	case INT32:
	  Data::_cast((int*)aReturnData.data(),
		      (unsigned char*)data(),nbItems);
	  break;
	case UINT64:
	  Data::_cast((unsigned long long*)aReturnData.data(),
		      (unsigned char*)data(),nbItems);
	  break;
	case INT64:
	  Data::_cast((long long*)aReturnData.data(),
		      (unsigned char*)data(),nbItems);
	  break;
	case FLOAT:
	  Data::_cast((float*)aReturnData.data(),
		      (unsigned char*)data(),nbItems);
	  break;
	case DOUBLE:
	  Data::_cast((double*)aReturnData.data(),
		      (unsigned char*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    case INT8:
      switch(aReturnData.type)
	{
	case UINT16: 
	  Data::_cast((unsigned short*)aReturnData.data(),
		      (char*)data(),nbItems);
	  break;
	case INT16:
	  Data::_cast((short*)aReturnData.data(),
		      (char*)data(),nbItems);
	  break;
	case UINT32:	    
	  Data::_cast((unsigned int*)aReturnData.data(),
		      (char*)data(),nbItems);
	  break;
	case INT32:
	  Data::_cast((int*)aReturnData.data(),
		      (char*)data(),nbItems);
	  break;
	case UINT64:
	  Data::_cast((unsigned long long*)aReturnData.data(),
		      (char*)data(),nbItems);
	  break;
	case INT64:
	  Data::_cast((long long*)aReturnData.data(),
		      (char*)data(),nbItems);
	  break;
	case FLOAT:
	  Data::_cast((float*)aReturnData.data(),
		      (char*)data(),nbItems);
	  break;
	case DOUBLE:
	  Data::_cast((double*)aReturnData.data(),
		      (char*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    case UINT16:
      switch(aReturnData.type)
	{
	case UINT32:	    
	  Data::_cast((unsigned int*)aReturnData.data(),
		      (unsigned short*)data(),nbItems);
	  break;
	case INT32:
	  Data::_cast((int*)aReturnData.data(),
		      (unsigned short*)data(),nbItems);
	  break;
	case UINT64:
	  Data::_cast((unsigned long long*)aReturnData.data(),
		      (unsigned short*)data(),nbItems);
	  break;
	case INT64:
	  Data::_cast((long long*)aReturnData.data(),
		      (unsigned short*)data(),nbItems);
	  break;
	case FLOAT:
	  Data::_cast((float*)aReturnData.data(),
		      (unsigned short*)data(),nbItems);
	  break;
	case DOUBLE:
	  Data::_cast((double*)aReturnData.data(),
		      (unsigned short*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    case INT16:
      switch(aReturnData.type)
	{
	case UINT16:	    
	  Data::_cast((unsigned short*)aReturnData.data(),
		      (short*)data(),nbItems);
	  break;
	case UINT32:	    
	  Data::_cast((unsigned int*)aReturnData.data(),
		      (short*)data(),nbItems);
	  break;
	case INT32:
	  Data::_cast((int*)aReturnData.data(),
		      (short*)data(),nbItems);
	  break;
	case UINT64:
	  Data::_cast((unsigned long long*)aReturnData.data(),
		      (short*)data(),nbItems);
	  break;
	case INT64:
	  Data::_cast((long long*)aReturnData.data(),
		      (short*)data(),nbItems);
	  break;
	case FLOAT:
	  Data::_cast((float*)aReturnData.data(),
		      (short*)data(),nbItems);
	  break;
	case DOUBLE:
	  Data::_cast((double*)aReturnData.data(),
		      (short*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    case UINT32:
      switch(aReturnData.type)
	{
	case UINT64:
	  Data::_cast((unsigned long long*)aReturnData.data(),
		      (unsigned int*)data(),nbItems);
	  break;
	case INT64:
	  Data::_cast((long long*)aReturnData.data(),
		      (unsigned int*)data(),nbItems);
	  break;
	case FLOAT:
	  Data::_cast((float*)aReturnData.data(),
		      (unsigned int*)data(),nbItems);
	  break;
	case DOUBLE:
	  Data::_cast((double*)aReturnData.data(),
		      (unsigned int*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    case INT32:
      switch(aReturnData.type)
	{
	case UINT32:	    
	  Data::_cast((unsigned int*)aReturnData.data(),
		      (int*)data(),nbItems);
	  break;
	case UINT64:
	  Data::_cast((unsigned long long*)aReturnData.data(),
		      (int*)data(),nbItems);
	  break;
	case INT64:
	  Data::_cast((long long*)aReturnData.data(),
		      (int*)data(),nbItems);
	  break;
	case FLOAT:
	  Data::_cast((float*)aReturnData.data(),
		      (int*)data(),nbItems);
	  break;
	case DOUBLE:
	  Data::_cast((double*)aReturnData.data(),
		      (int*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    case UINT64:
      switch(aReturnData.type)
	{
	case FLOAT:
	  Data::_cast((float*)aReturnData.data(),
		      (unsigned long long*)data(),nbItems);
	  break;
	case DOUBLE:
	  Data::_cast((double*)aReturnData.data(),
		      (unsigned long long*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    case INT64:
      switch(aReturnData.type)
	{
	case UINT64:
	  Data::_cast((unsigned long long*)aReturnData.data(),
		      (long long*)data(),nbItems);
	  break;
	case FLOAT:
	  Data::_cast((float*)aReturnData.data(),
		      (long long*)data(),nbItems);
	  break;
	case DOUBLE:
	  Data::_cast((double*)aReturnData.data(),
		      (long long*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    case FLOAT:
      switch(aReturnData.type)
	{
	case INT32:
	  Data::_cast((int*)aReturnData.data(),
		      (float*)data(),nbItems);
	  break;
	case INT64:
	  Data::_cast((long long*)aReturnData.data(),
		      (float*)data(),nbItems);
	  break;
	case DOUBLE:
	  Data::_cast((double*)aReturnData.data(),
		      (float*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    case DOUBLE:
      switch(aReturnData.type)
	{
	case INT32:
	  Data::_cast((int*)aReturnData.data(),
		      (double*)data(),nbItems);
	  break;
	case INT64:
	  Data::_cast((long long*)aReturnData.data(),
		      (double*)data(),nbItems);
	  break;
	case FLOAT:
	  Data::_cast((float*)aReturnData.data(),
		      (double*)data(),nbItems);
	  break;
	default:
	  throw ProcessException("This cast is not manage");
	}
      break;
    default:
      throw ProcessException("This cast is not manage");
    }
  return aReturnData;
}

DLL_EXPORT std::ostream& operator<<(std::ostream &os,
				    const Data::HeaderContainer &aHeader);

inline std::ostream& operator<<(std::ostream &os,const Buffer &aBuffer)
{
  const char *anOwnerShip = (aBuffer.owner == 
			     Buffer::MAPPED) ? "Mapped" : "Shared";
  os << "<"
     << "owner=" << anOwnerShip << ", "
     << "refcount=" << aBuffer.refcount << ", "
     << "data=" << aBuffer.data 
     << ">";
  return os;
}

inline std::ostream& operator<<(std::ostream &os,const Data &aData)
{
  const char* aHumanTypePt;
  switch(aData.type)
    {
    case Data::UINT8: 	aHumanTypePt = "UINT8";break;
    case Data::INT8: 	aHumanTypePt = "INT8";break;
    case Data::UINT16: 	aHumanTypePt = "UINT16";break;
    case Data::INT16: 	aHumanTypePt = "INT16";break;
    case Data::UINT32: 	aHumanTypePt = "UINT32";break;
    case Data::INT32: 	aHumanTypePt = "INT32";break;
    case Data::UINT64: 	aHumanTypePt = "UINT64";break;
    case Data::INT64: 	aHumanTypePt = "INT64";break;
    case Data::FLOAT: 	aHumanTypePt = "FLOAT";break;
    case Data::DOUBLE: 	aHumanTypePt = "DOUBLE";break;
    default: aHumanTypePt = "UNKNOWN"; break;
    }
  os << "<"
     << "type=" << aData.type << " (" << aHumanTypePt << "), ";

  int dimNb = 0;
  for(std::vector<int>::const_iterator i = aData.dimensions.begin();
      i != aData.dimensions.end();++i)
    os << "dimension_" << dimNb++ << "=" << *i << ", ";

  os << "frameNumber=" << aData.frameNumber << ", "
     << "timestamp=" << aData.timestamp << ", "
     << "header=" << aData.header << ", ";
  if(aData.buffer)
    os << "buffer=" << *aData.buffer;
  else
    os << "buffer=" << "NULL";
  os << ">";
  return os;
}


#endif
