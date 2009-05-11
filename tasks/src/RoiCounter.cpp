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

void RoiCounterTask::process(Data &aData)
{
  //_check_roi(_x,_y,_width,_height,aData);
  RoiCounterResult aResult;
  aResult.frameNumber = aData.frameNumber;
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
  _mgr.setResult(aResult);
}

void RoiCounterTask::setRoi(int x,int y,int width,int height)
{
  _x = x,_y = y;
  _width = width,_height = height;
}
