#include <stdlib.h>
#include <iostream>
#ifndef __DATA_H
#define __DATA_H
struct Buffer
{
  enum Ownership {MAPPED,SHARED};
  Buffer() : owner(SHARED),refcount(1),data(NULL) {}
  explicit Buffer(int aSize) :owner(SHARED),refcount(1)
  {
    if(posix_memalign(&data,16,aSize))
      std::cerr << "Can't allocate memory" << std::endl;
  }
  void unref()
  {
    if(!(--refcount))
      {
	if(owner == SHARED && data)
	  free(data);
	delete this;
      }
  }
  void ref()
  {
    ++refcount;
  }
  Ownership owner;
  int	    refcount;
  void      *data;
};
struct Data
{
  enum TYPE {UNDEF,UINT8,INT8,UINT16,INT16,UINT32,INT32,UINT64,INT64,FLOAT,DOUBLE};
  Data() : type(UNDEF),width(-1),height(-1),frameNumber(-1),buffer(NULL) {}
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
    if(buffer) buffer->unref();
    if(aData.buffer) aData.buffer->ref();
    buffer = aData.buffer;
    type = aData.type;
    width = aData.width;
    height = aData.height;
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
    type = UNDEF,width = -1,height = -1,frameNumber = -1;
  }
  inline void * data() {return buffer ? buffer->data : NULL;}
  inline int size() const 
  {
    return depth() * width * height;
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
  inline bool empty() const { return buffer ? !buffer->data : true;}
  //@brief return a data with the same header but with different buffer.
  inline Data copyHeader(TYPE aType)
  {
    Data aReturnData;
    aReturnData.type = aType;
    aReturnData.width = width;
    aReturnData.height = height;
    aReturnData.frameNumber = frameNumber;
    aReturnData.buffer = new Buffer(this->size());
    return aReturnData;
  }
  inline Data& operator=(const Data &aData)
  {
    type = aData.type;
    width = aData.width;
    height = aData.height;
    frameNumber = aData.frameNumber;
    setBuffer(aData.buffer);
    return *this;
  }
  TYPE      type;
  int       width;
  int       height;
  int       frameNumber;
  mutable Buffer *buffer;
};
#endif
