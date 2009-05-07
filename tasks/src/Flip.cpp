#include "Flip.h"
using namespace Tasks;

Flip::Flip() : _mode(FLIP_NONE) {}

Flip::Flip(const Flip &aFlip) : 
  LinkTask(aFlip),_mode(aFlip._mode) 
{}

static void _flip_y_inplace(Data &aSrcData)
{
  int lineSize = aSrcData.width * aSrcData.depth();
  char *aSrcPt = (char*)aSrcData.data();
  char *aDestPt = (char*)aSrcData.data();
  aDestPt += aSrcData.size() - lineSize;

  Buffer *aTmpLineBuffer = new Buffer(lineSize);
  for(int aNbLine = aSrcData.height / 2;aNbLine;
      --aNbLine,aDestPt -= lineSize,aSrcPt += lineSize)
    {
      memcpy(aTmpLineBuffer->data,aDestPt,lineSize);
      memcpy(aDestPt,aSrcPt,lineSize);
      memcpy(aSrcPt,aTmpLineBuffer->data,lineSize);
    }
  aTmpLineBuffer->unref();
}

static void _flip_y(Data &aSrcData,Data &aDestData)
{
  int lineSize = aSrcData.width * aSrcData.depth();
  char *aSrcPt = (char*)aSrcData.data();
  char *aDestPt = (char*)aDestData.data();
  aDestPt += aSrcData.size() - lineSize;
  
  for(int aNbLine = aSrcData.height;aNbLine;
      --aNbLine,aDestPt -= lineSize,aSrcPt += lineSize)
    memcpy(aDestPt,aSrcPt,lineSize);
}

template<class IN>
inline static void _flip_x_inplace_template(IN *aSrcPt,int width, int height)
{
  IN *aDestPt = aSrcPt + width - 1;
  for(int aNbLine = height;aNbLine;
      --aNbLine,aDestPt += width + width / 2,aSrcPt += width - width / 2)
    for(int aNbColumn = width / 2 ;aNbColumn;--aNbColumn,--aDestPt,++aSrcPt)
      {
	IN aTmpValue = *aDestPt;
	*aDestPt = *aSrcPt;
	*aSrcPt = aTmpValue;
      }
}
static void _flip_x_inplace(Data &aSrcData)
{
   switch(aSrcData.depth())
    {
    case 1:
      _flip_x_inplace_template((uint8_t*)aSrcData.data(),
			       aSrcData.width,aSrcData.height);
      break;
    case 2:
      _flip_x_inplace_template((uint16_t*)aSrcData.data(),
		       aSrcData.width,aSrcData.height);
      break;
    case 4:
      _flip_x_inplace_template((uint32_t*)aSrcData.data(),
			       aSrcData.width,aSrcData.height);
      break;
    case 8:
      _flip_x_inplace_template((uint64_t*)aSrcData.data(),
			       aSrcData.width,aSrcData.height);
      break;
    }
}
template<class IN>
inline static void _flip_x_template(const IN *aSrcPt,IN *aDestPt,int width, int height)
{
  aDestPt += width - 1;
  for(int aNbLine = height;aNbLine;--aNbLine,aDestPt += (2 * width))
    for(int aNbColumn = width;aNbColumn;--aNbColumn,--aDestPt,++aSrcPt)
      *aDestPt = *aSrcPt;

}
static void _flip_x(Data &aSrcData,Data &aDestData)
{
  switch(aSrcData.depth())
    {
    case 1:
      _flip_x_template((const uint8_t*)aSrcData.data(),(uint8_t*)aDestData.data(),
		       aSrcData.width,aSrcData.height);
      break;
    case 2:
      _flip_x_template((const uint16_t*)aSrcData.data(),(uint16_t*)aDestData.data(),
		       aSrcData.width,aSrcData.height);
      break;
    case 4:
      _flip_x_template((const uint32_t*)aSrcData.data(),(uint32_t*)aDestData.data(),
		       aSrcData.width,aSrcData.height);
      break;
    case 8:
      _flip_x_template((const uint64_t*)aSrcData.data(),(uint64_t*)aDestData.data(),
		       aSrcData.width,aSrcData.height);
      break;
    }
}

template<class IN>
inline static void _flip_all_inplace_template(IN *aSrcPt,int width, int height)
{
  IN *aDestPt = aSrcPt + width * height - 1;
  for(int aNbPixel = (height * width) / 2;aNbPixel;
      --aNbPixel,--aDestPt,++aSrcPt)
    {
      IN aPixelBuff = *aDestPt;
      *aDestPt = *aSrcPt;
      *aSrcPt = aPixelBuff;
    }
}

static void _flip_all_inplace(Data &aSrcData)
{
   switch(aSrcData.depth())
    {
    case 1:
      _flip_all_inplace_template((uint8_t*)aSrcData.data(),
				 aSrcData.width,aSrcData.height);
      break;
    case 2:
      _flip_all_inplace_template((uint16_t*)aSrcData.data(),
				 aSrcData.width,aSrcData.height);
      break;
    case 4:
      _flip_all_inplace_template((uint32_t*)aSrcData.data(),
				 aSrcData.width,aSrcData.height);
      break;
    case 8:
      _flip_all_inplace_template((uint64_t*)aSrcData.data(),
				 aSrcData.width,aSrcData.height);
      break;
    }
}
template<class IN>
inline static void _flip_all_template(const IN *aSrcPt,IN *aDestPt,int width, int height)
{
  aDestPt += width * height - 1;
  for(int aNbPixel = height * width;aNbPixel;
      --aNbPixel,--aDestPt,++aSrcPt)
      *aDestPt = *aSrcPt;
}
static void _flip_all(Data &aSrcData,Data &aDestData)
{
  switch(aSrcData.depth())
    {
    case 1:
      _flip_all_template((const uint8_t*)aSrcData.data(),(uint8_t*)aDestData.data(),
			 aSrcData.width,aSrcData.height);
      break;
    case 2:
      _flip_all_template((const uint16_t*)aSrcData.data(),(uint16_t*)aDestData.data(),
			 aSrcData.width,aSrcData.height);
      break;
    case 4:
      _flip_all_template((const uint32_t*)aSrcData.data(),(uint32_t*)aDestData.data(),
			 aSrcData.width,aSrcData.height);
      break;
    case 8:
      _flip_all_template((const uint64_t*)aSrcData.data(),(uint64_t*)aDestData.data(),
			 aSrcData.width,aSrcData.height);
      break;
    }
}

Data Flip::process(Data &aData)
{

  if(_processingInPlaceFlag)
    {
      switch(_mode)
	{
	case FLIP_X:
	  _flip_x_inplace(aData);
	  break;
	case FLIP_Y:
	  _flip_y_inplace(aData);
	  break;
	case FLIP_ALL:
	  _flip_all_inplace(aData);
	  break;
	default:
	  break;
	}
      return aData;
    }
  else
    {
      Data aNewData = aData.copyHeader(aData.type);
      switch(_mode)
	{
	case FLIP_X:
	  _flip_x(aData,aNewData);
	  break;
	case FLIP_Y:
	  _flip_y(aData,aNewData);
	  break;
	case FLIP_ALL:
	  _flip_all(aData,aNewData);
	  break;
	default:
	  memcpy(aNewData.data(),aData.data(),aData.size());
	  break;
	}
      return aNewData;
    }
}

void Flip::setFlip(FLIP_MODE aMode)
{
  _mode = aMode;
}
