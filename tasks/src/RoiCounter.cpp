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
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <cstdio>

#include "processlib/ProcessExceptions.h"
#include "processlib/RoiCounter.h"
using namespace Tasks;

RoiCounterTask::RoiCounterTask(RoiCounterManager &aMgr) :
  SinkTask<RoiCounterResult>(aMgr),
  _type(UNDEF),
  _x(0),_y(0),
  _width(0),_height(0)
{
}

RoiCounterTask::RoiCounterTask(const RoiCounterTask &aTask) :
  SinkTask<RoiCounterResult>(aTask),
  _type(aTask._type),
  _x(aTask._x),_y(aTask._y),
  _width(aTask._width),_height(aTask._height),
  _arc_roi(aTask._arc_roi),
  _mask(aTask._mask),
  _lut(aTask._lut)
{
}

void RoiCounterTask::setRoi(int x,int y,int width,int height)
{
  _type = SQUARE;
  _x = x,_y = y;
  _width = width,_height = height;
}
void RoiCounterTask::getRoi(int &x,int &y,int &width,int &height)
{
  if(_type != SQUARE)
    throw ProcessException("RoiCounterTask: This is not a SQUARE Roi");
  x = _x,y = _y,width = _width,height = _height;
}

void RoiCounterTask::setLut(int x,int y,Data &lut)
{
  if(lut.dimensions.size() < 1 || lut.dimensions.size() > 2)
    throw ProcessException("RoiCounterTask : Only manage 1 or 2D data");

  _type = LUT;
  _lut = lut.cast(Data::DOUBLE);
  _x = x,_y = y;
}
void RoiCounterTask::getLut(int &x,int &y,Data &lut)
{
  if(_type != LUT)
    throw ProcessException("RoiCounterTask: This is not a LUT Roi");

  x = _x,y = _y,lut = _lut;
}
void RoiCounterTask::setLutMask(int x,int y,Data &mask)
{
  _type = MASK;
  _lut = mask.mask();
  _x = x,_y = y;
}
void RoiCounterTask::getLutMask(int& x,int& y,Data &mask)
{
  if(_type != MASK)
    throw ProcessException("RoiCounterTask: This is not a MASK Roi");
  x = _x,y = _y,mask = _lut;
}
void RoiCounterTask::setArcMask(double centerX,double centerY,
				double rayon1,double rayon2,
				double angle_start,double angle_end)
{
  if(rayon1 > rayon2)
    {
      double rayon_tmp = rayon2;
      rayon2 = rayon1;
      rayon1 = rayon_tmp;
    }
  _type = ARC;
  _arc_roi = ArcRoi(centerX, centerY, rayon1, rayon2, angle_start, angle_end);
  //find the bounding box
  double x_min,x_max;
  double y_min,y_max;
  double rayons[] = {rayon1,rayon2,rayon1,rayon2};
  double angles[] = {angle_start,angle_start,angle_end,angle_end};
  // Init
  x_min = x_max = centerX + rayons[0] * cos(angles[0] * M_PI / 180.);
  y_min = y_max = centerY + rayons[0] * sin(angles[0] * M_PI / 180.);
  for(int i = 1;i <= 3;++i)
    {
      double x = centerX + rayons[i] * cos(angles[i] * M_PI / 180.);
      double y = centerY + rayons[i] * sin(angles[i] * M_PI / 180.);
      if(x > x_max) x_max = x;
      else if(x < x_min) x_min = x;

      if(y > y_max) y_max = y;
      else if(y < y_min) y_min = y;
    }
  double a_start,a_end;
  if(angle_end > angle_start) 
    a_start = angle_start,a_end = angle_end;
  else
    a_start = angle_end,a_end = angle_start;
  //find all intersected axis
  for(int angle = int((a_start + 90) / 90) * 90;
      angle < a_end;angle += 90)
    {
      double x = centerX + rayon2 * cos(angle * M_PI / 180.);
      double y = centerY + rayon2 * sin(angle * M_PI / 180.);
      if(x > x_max) x_max = x;
      else if(x < x_min) x_min = x;

      if(y > y_max) y_max = y;
      else if(y < y_min) y_min = y;
    }
  _x = int(floor(x_min));
  _y = int(floor(y_min));
  
  if(_x < 0 || _y < 0)
    {
      _type = UNDEF;
      char buffer[512];
      snprintf(buffer,sizeof(buffer),
	       "RoiCounterTask arc calculation give an origin (%d,%d) which is below index 0",_x,_y);
      throw ProcessException(buffer);
    }

  _width = int(ceil(x_max)) - _x + 1;
  _height = int(ceil(y_max)) - _y + 1;
  
  //Allocation memory for the lut
  Buffer *buffer = new Buffer(_width * _height * sizeof(char));
  _lut.dimensions.clear();
  _lut.dimensions.push_back(_width);
  _lut.dimensions.push_back(_height);
  _lut.type = Data::INT8;
  _lut.setBuffer(buffer);
  buffer->unref();

  double angle_diff = angle_end - angle_start;
  double rayon1sqr = rayon1 * rayon1;
  double rayon2sqr = rayon2 * rayon2;
  char *bufferPt = (char*)_lut.data();
  for(int y = _y;y < _y + _height;++y)
    {
      for(int x = _x;x < _x + _width;++x,++bufferPt)
	{
	  double r1sqrt = pow(x - centerX,2) + pow(y - centerY,2);
	  double r2sqrt = pow(x - centerX,2) + pow(y - centerY,2);
	  double X = x - centerX;
	  double Y = y - centerY;
	  double angle = 0.;
	  if(r1sqrt >= rayon1sqr && r2sqrt <= rayon2sqr)
	    {
	      if(fabs(X - 1e-6) > 1e-6)
		angle = atan(Y/X) * 180 / M_PI;
	      else
		angle = Y > 0 ? 90 : -90;

	      if(X < 0)
		{
		  if(Y > 0) angle += 180.;
		  else angle -= 180;
		}
	      double angle_min_diff = angle - angle_start;
	      if(angle_diff > 0)
		{
		  if(angle_min_diff < 0) angle_min_diff += 360;
		  *bufferPt = angle_min_diff >= 0 && angle_min_diff <= angle_diff;
		}
	      else
		{
		  if(angle_min_diff > 0) angle_min_diff -= 360;
		  *bufferPt = angle_min_diff <= 0 && angle_min_diff >= angle_diff;
		}
	    }
	  else
	    *bufferPt = 0;
	}
    }
}
void RoiCounterTask::getArcMask(double &centerX,double &centerY,
				double &rayon1,double &rayon2,
				double &angle_start,double &angle_end)
{
  if(_type != ARC)
    throw ProcessException("RoiCounterTask: This is not a ARC Roi");

  centerX = _arc_roi.x;
  centerY = _arc_roi.y;
  rayon1 = _arc_roi.r1;
  rayon2 = _arc_roi.r2;
  angle_start = _arc_roi.a1;
  angle_end = _arc_roi.a2;
}
template<class INPUT> static void _get_average_std(const Data& aData,
						   int x,int y,
						   int width,int height,
						   RoiCounterResult &aResult)
{
  const INPUT *aSrcPt = (const INPUT*)aData.data();
  int widthStep = aData.dimensions[0];
  INPUT aMin,aMax;
  aMin = aMax = *(aSrcPt + y * widthStep + x); // Init
  double aSum = 0.;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      const INPUT *aLinePt = aSrcPt + lineId * widthStep + x;
      for(int i = 0;i < width;++i,++aLinePt)
	{
	  aSum += double(*aLinePt);
	  if(*aLinePt > aMax)
	    aMax = *aLinePt;
	  else if(*aLinePt < aMin)
	    aMin = *aLinePt;
	}
    }

  aResult.sum = aSum;
  aResult.average = aSum / (width * height);
  aResult.minValue = double(aMin);
  aResult.maxValue = double(aMax);

  //STD
  aSum = 0.;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      const INPUT *aLinePt = aSrcPt + lineId * widthStep + x;
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

template<class INPUT> static void _get_average_std_with_mask(const Data& aData,
							     const char *aMaskPt,
							     int x,int y,
							     int width,int height,
							     RoiCounterResult &aResult)
{
  const INPUT *aSrcPt = (INPUT*)aData.data();
  int widthStep = aData.dimensions[0];
  INPUT aMin = INPUT(0.),aMax = INPUT(0.);
  //min max init;
  bool continueFlag = true;
  for(int lineId = y;continueFlag && lineId < y + height;++lineId)
    {
      int offset = lineId * widthStep + x;
      const INPUT *aLinePt = aSrcPt + offset;
      const char *aMaskLinePt = aMaskPt + offset;
      for(int i = 0;continueFlag && i < width;++i,++aLinePt,++aMaskLinePt)
	{
	  if(*aMaskLinePt)
	    aMin = aMax = *aLinePt,continueFlag = false;
	}
    }

  double aSum = 0.;
  int usedSize = width * height;
  for(int lineId = y;lineId < y + height;++lineId)
    {
      int offset = lineId * widthStep + x;
      const INPUT *aLinePt = aSrcPt + offset;
      const char *aMaskLinePt = aMaskPt + offset;
      for(int i = 0;i < width;++i,++aLinePt,++aMaskLinePt)
	{
	  if(*aMaskLinePt)
	    {
	      aSum += double(*aLinePt);
	      if(*aLinePt > aMax) aMax = *aLinePt;
	      else if(*aLinePt < aMin) aMin = *aLinePt;
	    }
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
      int offset = lineId * widthStep + x;
      const INPUT *aLinePt = aSrcPt + offset;
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

  aResult.minValue = aMin;
  aResult.maxValue = aMax;
}

template<class INPUT> static void _lut_get_average_std(const Data& data,
						       int x,int y,Data& lut,
						       RoiCounterResult &aResult)
{
  double *lutPt = (double*)lut.data();
  const INPUT* aSrcPt = (const INPUT*)data.data();
  
  int widthStep = data.dimensions[0];
  double aMin,aMax;
  //jump to start
  aSrcPt += y * widthStep + x;

  int nbItems = 1;
  for(std::vector<int>::iterator i = lut.dimensions.begin();
      i != lut.dimensions.end();++i)
    nbItems *= *i;
  
  int totalItems = nbItems;
  //min max initialization
  aMin = aMax = 0.;
  double aSum = 0.;
  double aWeight = 0.;
  int cId = 0;
  bool continueFlag = true;
  while(continueFlag && nbItems)
    {
      for(cId = 0;continueFlag && cId < lut.dimensions[0];++cId,--nbItems,++lutPt)
	{
	  if(*lutPt != 0.)
	    {
	      double value = aSrcPt[cId] * *lutPt; 
	      aMax = aMin = value;
	      aSum = value;
	      aWeight = *lutPt;
	      continueFlag = false;
	    }
	}
      if(continueFlag)
	aSrcPt += widthStep;
    }
  
  while(nbItems)
    {
      for(;cId < lut.dimensions[0];++cId,--nbItems,++lutPt)
	{
	  if(*lutPt != 0.)
	    {
	      double value = aSrcPt[cId] * *lutPt;
	      if(value > aMax) aMax = value;
	      else if(value < aMin) aMin = value;
	      aSum += value;
	      aWeight += *lutPt;
	    }
	}
      aSrcPt += widthStep,cId = 0;
    }

  aResult.minValue = aMin;
  aResult.maxValue = aMax;
  aResult.sum = aSum;
  if(aWeight > 0.)
    aResult.average = aSum / aWeight;
  else
    aResult.average = 0.;
  
  lutPt = (double*)lut.data();
  aSrcPt = (const INPUT*)data.data();
  aSrcPt += y * widthStep + x;
  
  nbItems = totalItems;
  aSum = 0.;
  while(nbItems)
    {
      for(int cId = 0;cId < lut.dimensions[0];++cId,--nbItems,++lutPt)
	{
	  if(*lutPt != 0.)
	    {
	      double value = aSrcPt[cId] * *lutPt;
	      double diff = value - aResult.average;
	      diff *= diff;
	      aSum += diff;
	    }
	}
      aSrcPt += widthStep;
    }
  
  if(aWeight != 0.)
    aResult.std = sqrt(aSum / aWeight);
  else
    aResult.std = 0.;
}

template<class INPUT> static void _mask_get_average_std(const Data& data,
							int x,int y,Data& mask,
							RoiCounterResult &aResult)
{
  char *maskPt = (char*)mask.data();
  const INPUT* aSrcPt = (const INPUT*)data.data();

  int widthStep = data.dimensions[0];
  INPUT aMin,aMax;
  //jump to start
  aSrcPt += y * widthStep + x;

  int nbItems = 1;
  for(std::vector<int>::iterator i = mask.dimensions.begin();
      i != mask.dimensions.end();++i)
    nbItems *= *i;
  
  int totalItems = nbItems;
  //min max initialization
  aMin = aMax = INPUT(0);
  long long aSum = 0;
  int aNbPixel = 0;
  int cId = 0;
  bool continueFlag = true;
  while(continueFlag && nbItems)
    {
      for(cId = 0;continueFlag && cId < mask.dimensions[0];++cId,--nbItems,++maskPt)
	{
	  if(*maskPt)
	    {
	      INPUT value = aSrcPt[cId];
	      aMax = aMin = value;
	      aSum = value;
	      ++aNbPixel;
	      continueFlag = false;
	    }
	}
      if(continueFlag)
	aSrcPt += widthStep;
    }
  
  while(nbItems)
    {
      for(;cId < mask.dimensions[0];++cId,--nbItems,++maskPt)
	{
	  if(*maskPt)
	    {
	      INPUT value = aSrcPt[cId];
	      if(value > aMax) aMax = value;
	      else if(value < aMin) aMin = value;
	      aSum += value;
	      ++aNbPixel;
	    }
	}
      aSrcPt += widthStep,cId = 0;
    }

  aResult.minValue = aMin;
  aResult.maxValue = aMax;
  aResult.sum = aSum;
  if(aNbPixel)
    aResult.average = double(aSum) / aNbPixel;
  else
    aResult.average = 0.;
  
  maskPt = (char*)mask.data();
  aSrcPt = (const INPUT*)data.data();
  aSrcPt += y * widthStep + x;
  
  nbItems = totalItems;
  aSum = 0.;
  while(nbItems)
    {
      for(int cId = 0;cId < mask.dimensions[0];++cId,--nbItems,++maskPt)
	{
	  if(*maskPt)
	    {
	      double value = aSrcPt[cId];
	      double diff = value - aResult.average;
	      diff *= diff;
	      aSum += diff;
	    }
	}
      aSrcPt += widthStep;
    }
  
  if(aNbPixel)
    aResult.std = sqrt(double(aSum) / aNbPixel);
  else
    aResult.std = 0.;
}

void RoiCounterTask::_check_roi_with_data_size(Data& data)
{
  if(_x < 0 || _y < 0)
    throw ProcessException("RoiCounter : roi origin must be positive");

  long width = data.dimensions[0];
  long height = data.dimensions[1];

  switch(_type)
    {
    case SQUARE:
      {
	if(width < _x + _width || height < _y + _height)
	  {
	    char buffer[1024];
	    snprintf(buffer,sizeof(buffer),"RoiCounter : roi <%d,%d>-<%dx%d>"
		     " is not contained into data <%ldx%ld>",
		     _x,_y,_width,_height,width,height);
	    throw ProcessException(buffer);
	  }
	break;
      }
    case LUT:
      if(_lut.dimensions.size() != data.dimensions.size())
	throw ProcessException("RoiLutCounter lut and data must have the same dimension");
      if(_lut.dimensions[0] + _x > data.dimensions[0])
	throw ProcessException("RoiLutCounter lut width + origin go outside of the data bounding box");
      if(_lut.dimensions.size() > 1 && 
	 _lut.dimensions[1] + _y > data.dimensions[1])
	throw ProcessException("RoiLutCounter lut height + origin got outside of the data bounding box");
      break;
    case ARC:
    case MASK:
      if(_lut.dimensions.size() != data.dimensions.size())
	throw ProcessException("RoiLutCounter mask and data must have the same dimension");
      if(_lut.dimensions[0] + _x > data.dimensions[0])
	throw ProcessException("RoiLutCounter mask width + origin go outside of the data bounding box");
      if(_lut.dimensions.size() > 1 && 
	 _lut.dimensions[1] + _y > data.dimensions[1])
	throw ProcessException("RoiLutCounter mask height + origin got outside of the data bounding box");
      break;
    default:
      throw ProcessException("RoiCounter : roi is not defined");
    }
}

#define GET_AVERAGE_STD(TYPE)					\
  {								\
  if(_type == SQUARE)						\
    _get_average_std<TYPE>(aData,_x,_y,_width,_height,aResult);	\
  else if(_type == LUT)						\
    _lut_get_average_std<TYPE>(aData,_x,_y,_lut,aResult);	\
  else if((_type == MASK) || (_type == ARC))			\
    _mask_get_average_std<TYPE>(aData,_x,_y,_lut,aResult);	\
  }

void RoiCounterTask::process(Data &aData)
{
  //_check_roi(_x,_y,_width,_height,aData);
  if(aData.dimensions.size() != 2)
    throw ProcessException("RoiCounterTask : Only manage 2D data");

  _check_roi_with_data_size(aData);

  RoiCounterResult aResult;
  aResult.frameNumber = aData.frameNumber;
  if(_mask.empty())
    {
      switch(aData.type)
	{
	case Data::UINT8: 
	  GET_AVERAGE_STD(unsigned char);
	  break;
	case Data::INT8:
	  GET_AVERAGE_STD(char);
	  break;
	case Data::UINT16:
	  GET_AVERAGE_STD(unsigned short);
	  break;
	case Data::INT16:
	  GET_AVERAGE_STD(short);
	  break;
	case Data::UINT32:
	  GET_AVERAGE_STD(unsigned int);
	  break;
	case Data::INT32:
	  GET_AVERAGE_STD(int);
	  break;
	case Data::UINT64:
	  GET_AVERAGE_STD(unsigned long long);
	  break;
	case Data::INT64:
	  GET_AVERAGE_STD(long long);
	  break;

	case Data::FLOAT:
	  GET_AVERAGE_STD(float);
	  break;
	case Data::DOUBLE:
	  GET_AVERAGE_STD(double);
	  break;
	default: 
	  break;				// error
	}
    }
  else if(_mask.dimensions == aData.dimensions)
    {
      switch(aData.type)
	{
	case Data::UINT8: 
	  _get_average_std_with_mask<unsigned char>(aData,
						    (char*)_mask.data(),
						    _x,_y,_width,_height,aResult);
	  break;
	case Data::INT8:
	  _get_average_std_with_mask<char>(aData,
					   (char*)_mask.data(),
					   _x,_y,_width,_height,aResult);
	  break;

	case Data::UINT16:
	  _get_average_std_with_mask<unsigned short>(aData,
						     (char*)_mask.data(),
						     _x,_y,_width,_height,aResult);
	  break;
	case Data::INT16:
	  _get_average_std_with_mask<short>(aData,
					    (char*)_mask.data(),
					    _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT32:
	  _get_average_std_with_mask<unsigned int>(aData,
						   (char*)_mask.data(),
						   _x,_y,_width,_height,aResult);
	  break;
	case Data::INT32:
	  _get_average_std_with_mask<int>(aData,
					  (char*)_mask.data(),
					  _x,_y,_width,_height,aResult);
	  break;
	case Data::UINT64:
	  _get_average_std_with_mask<unsigned long long>(aData,
							 (char*)_mask.data(),
							 _x,_y,_width,_height,aResult);
	  break;
	case Data::INT64:
	  _get_average_std_with_mask<long long>(aData,
						(char*)_mask.data(),
						_x,_y,_width,_height,aResult);
	  break;

	case Data::FLOAT:
	  _get_average_std_with_mask<float>(aData,
					    (char*)_mask.data(),
					    _x,_y,_width,_height,aResult);
	  break;
	case Data::DOUBLE:
	  _get_average_std_with_mask<double>(aData,
					     (char*)_mask.data(),
					     _x,_y,_width,_height,aResult);
	  break;
	default: 
	  break;				// error
	}
    }
  else
    throw ProcessException("RoiCounter : Source image size differ from mask");

  _mgr.setResult(aResult);
}


#ifndef __unix
extern "C"
{
  static void _impl_bpm()
  {
    RoiCounterManager *RoiCounterMgr = new RoiCounterManager();

    RoiCounterMgr->setMode(RoiCounterManager::Counter);
    RoiCounterMgr->getResult();
    std::list<RoiCounterResult> aResult;
    RoiCounterMgr->getHistory(aResult);
    RoiCounterMgr->resizeHistory(10);
    RoiCounterMgr->resetHistory();
    RoiCounterMgr->historySize();
    RoiCounterMgr->lastFrameNumber();
    RoiCounterMgr->ref();
    RoiCounterMgr->unref();

    RoiCounterTask *roiCounterTask = new RoiCounterTask(*RoiCounterMgr);
    roiCounterTask->ref();
    roiCounterTask->unref();
  }
}
#endif
