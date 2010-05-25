#include "RoiCounter.h"
using namespace Tasks;
#include <math.h>

RoiCounterTask::RoiCounterTask(RoiCounterManager &aMgr) :
  SinkTask<RoiCounterResult>(aMgr),
  _x(0),_y(0),
  _width(0),_height(0)
{
}

RoiCounterTask::RoiCounterTask(const RoiCounterTask &aTask) :
  SinkTask<RoiCounterResult>(aTask),
  _x(aTask._x),_y(aTask._y),
  _width(aTask._width),_height(aTask._height)
{
}

template<class IN> static void _get_average_std(const IN *aSrcPt,
						int x,int y,
						int width,int height,
						RoiCounterResult &aResult)
{
  double aSum = 0.;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      const IN *aLinePt = aSrcPt + lineId * width + x;
      for(int i = 0;i < width;++i,++aLinePt)
	aSum += double(*aLinePt);
    }

  aResult.sum = aSum;
  aResult.average = aSum / (width * height);
  
  //STD
  aSum = 0.;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      const IN *aLinePt = aSrcPt + lineId * width + x;
      for(int i = 0;i < width;++i,++aLinePt)
	{
	  double diff = *aLinePt - aResult.average;
	  diff *= diff;
	  aSum += diff;
	}
    }
  aSum /= width * height;
  aResult.std = sqrt(aSum);
}

template<class IN> static void _get_average_std_with_mask(const IN *aSrcPt,
							  const char *aMaskPt,
							  int x,int y,
							  int width,int height,
							  RoiCounterResult &aResult)
{
  double aSum = 0.;
  int usedSize = width * height;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      int offset = lineId * width + x;
      const IN *aLinePt = aSrcPt + offset;
      const char *aMaskLinePt = aMaskPt + offset;
      for(int i = 0;i < width;++i,++aLinePt,++aMaskLinePt)
	{
	  if(*aMaskLinePt)
	    aSum += double(*aLinePt);
	  else
	    --usedSize;
	}
    }

  aResult.sum = aSum;
  if(usedSize > 0)
    aResult.average = aSum / usedSize;
  else
    aResult.average = 0.;
  
  //STD
  aSum = 0.;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      int offset = lineId * width + x;
      const IN *aLinePt = aSrcPt + offset;
      const char *aMaskLinePt = aMaskPt + offset;
      for(int i = 0;i < width;++i,++aLinePt,++aMaskLinePt)
	{
	  if(*aMaskLinePt) continue;

	  double diff = *aLinePt - aResult.average;
	  diff *= diff;
	  aSum += diff;
	}
    }

  if(usedSize > 0)
    {
      aSum /= usedSize;
      aResult.std = sqrt(aSum);
    }
  else
    aResult.std = 0.;
}

void RoiCounterTask::process(Data &aData)
{
  //_check_roi(_x,_y,_width,_height,aData);
  RoiCounterResult aResult;
  aResult.frameNumber = aData.frameNumber;
  if(_mask.empty())
    {
      switch(aData.type)
	{
	case Data::UINT8: 
	  _get_average_std((unsigned char*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT8:
	  _get_average_std((char*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;

	case Data::UINT16:
	  _get_average_std((unsigned short*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT16:
	  _get_average_std((short*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT32:
	  _get_average_std((unsigned int*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT32:
	  _get_average_std((int*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT64:
	  _get_average_std((unsigned long long*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT64:
	  _get_average_std((long long*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;

	case Data::FLOAT:
	  _get_average_std((float*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;
	case Data::DOUBLE:
	  _get_average_std((double*)aData.data(),
			   _x,_y,_width,_height,aResult);
	  break;
	default: 
	  break;				// error
	}
    }
  else if(_mask.width == aData.width &&
	  _mask.height == aData.height)
    {
      switch(aData.type)
	{
	case Data::UINT8: 
	  _get_average_std_with_mask((unsigned char*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT8:
	  _get_average_std_with_mask((char*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;

	case Data::UINT16:
	  _get_average_std_with_mask((unsigned short*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT16:
	  _get_average_std_with_mask((short*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT32:
	  _get_average_std_with_mask((unsigned int*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT32:
	  _get_average_std_with_mask((int*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT64:
	  _get_average_std_with_mask((unsigned long long*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT64:
	  _get_average_std_with_mask((long long*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;

	case Data::FLOAT:
	  _get_average_std_with_mask((float*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	case Data::DOUBLE:
	  _get_average_std_with_mask((double*)aData.data(),
				     (char*)_mask.data(),
				     _x,_y,_width,_height,aResult);
	  break;
	default: 
	  break;				// error
	}
    }
  else
    std::cerr << "RoiCounter : Source image size differ from mask" << std::endl;

  _mgr.setResult(aResult);
}

void RoiCounterTask::setRoi(int x,int y,int width,int height)
{
  _x = x,_y = y;
  _width = width,_height = height;
}
