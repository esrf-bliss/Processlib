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
#define NOMINMAX
#endif
#include "processlib/ProcessExceptions.h"
#include "processlib/Binning.h"
#include "processlib/Stat.h"
#include <sstream>
#include <limits>
using namespace Tasks;

//static function
template<class INPUT> static INPUT max_value(const INPUT &)
{
  return std::numeric_limits<INPUT>::max();
}

/** @brief generique binning but not optimized at all
 */
template<class INPUT>
inline void _default_binning(Data &aSrcData,Data &aDstData,
           int xFactor,int yFactor)
{
  INPUT *aSrcPt = (INPUT*)aSrcData.data();
  INPUT *aDstPt = (INPUT*)aDstData.data();
  INPUT MAX_VALUE = max_value(*aSrcPt);
  
  int maxLine = aSrcData.dimensions[1] / yFactor * yFactor;
  int maxColumn = aSrcData.dimensions[0] / xFactor * xFactor;

  for(int lineId = 0;lineId < maxLine;++lineId)
    {
      INPUT *aLineSrcPt = aSrcPt + aSrcData.dimensions[0] * lineId;
      INPUT *aLineDstPt = aDstPt + aDstData.dimensions[0] * (lineId / yFactor);
      for(int columnId = 0;columnId < maxColumn;++columnId)
      {
        unsigned long long result = aLineDstPt[columnId / xFactor];
        result += aLineSrcPt[columnId];
        if(result > MAX_VALUE)
          aLineDstPt[columnId / xFactor] = MAX_VALUE;
        else
          aLineDstPt[columnId / xFactor] = (INPUT) result;
      }
    }
}

/** @brief binning 2 x 2
 */
template<class INPUT>
static void _binning2x2(Data &aSrcData,Data &aDstData,int Factor)
{
  INPUT *aSrcFirstLinePt = (INPUT*)aSrcData.data();
  INPUT *aSrcSecondLinePt = aSrcFirstLinePt + aSrcData.dimensions[0];
  INPUT *aDstPt = (INPUT*)aDstData.data();
  INPUT MAX_VALUE = max_value(*aDstPt);
  for(int lineId = 0;lineId < aSrcData.dimensions[1];lineId += 2)
    {
      for(int columnId = 0;columnId < aSrcData.dimensions[0];columnId += 2,
      ++aDstPt,aSrcFirstLinePt += 2,aSrcSecondLinePt += 2)
      {
        // avoid overflow by promoting to ULL before add
        unsigned long long result = *aSrcFirstLinePt;
        result += *(aSrcFirstLinePt + 1);
        result += *aSrcSecondLinePt;
        result += *(aSrcSecondLinePt + 1);
        if(result > MAX_VALUE)
          *aDstPt = MAX_VALUE;
        else
          *aDstPt = INPUT(result);
      }
      aSrcFirstLinePt = aSrcSecondLinePt;
      aSrcSecondLinePt = aSrcFirstLinePt + aSrcData.dimensions[0];
    }
  aDstData.dimensions[0] >>= 1;
  aDstData.dimensions[1] >>= 1;
  if(Factor > 2)
    _binning2x2<INPUT>(aDstData,aDstData,Factor >> 1);
}

/**
 * Process a binning with a vertical scan line.
 *
 * The reduction method is mean.
 *
 * Only the full bins are processed. the destination pixels
 * are only set for the full bins, others are undefined.
 */
 template<class INPUT, class LINE>
 inline void _mean_binning(Data &aSrcData,Data &aDstData,
           int xFactor,int yFactor)
{
  INPUT *aSrcPt = (INPUT*)aSrcData.data();
  INPUT *aDstPt = (INPUT*)aDstData.data();

  int binLine = aSrcData.dimensions[1] / yFactor;
  int binColumn = aSrcData.dimensions[0] / xFactor;
  int binSize = xFactor * yFactor;

  std::vector<LINE> aScanLine(binLine);

  // For each vertical bins
  for(int j1 = 0; j1 < binLine; j1++) {
    // Initialize the scan line
    std::fill(aScanLine.begin(), aScanLine.end(), 0);

    // Compute accumulation in scan line
    int j2;
    for (j2 = 0; j2 < yFactor; j2++)
      for (int i1 = 0; i1 < binColumn; i1++)
        for (int i2 = 0; i2 < xFactor; i2++)
          aScanLine[i1] += aSrcPt[aSrcData.dimensions[0] * (j1 * yFactor + j2) + i1 * xFactor + i2];

    // Copy to destination
    for (int i1 = 0; i1 < binColumn; i1++)
      // Mean accumulated bin result
      aDstPt[i1] = aScanLine[i1] / binSize;

    aDstPt += binColumn;
  }
}

Binning::Binning() : mXFactor(-1),mYFactor(-1),mOperation(SUM) {};
Binning::Binning(const Binning &anOther) :
  LinkTask(anOther),
  mXFactor(anOther.mXFactor),
  mYFactor(anOther.mYFactor),
  mOperation(anOther.mOperation) {}

Data Binning::process(Data &aData)
{
  Data aNewData = aData;
  if(aData.dimensions.size() != 2)
    throw ProcessException("Binning : Only manage 2D data");
  if(aData.empty())
    throw ProcessException("Binning : Data is empty!");

  {
    std::stringstream info;
    info << "Binning " << mXFactor << " by " << mYFactor << " operation " << mOperation;
    Stat aStat(aNewData,info.str());

    if(mYFactor <= 0 || mXFactor <= 0)
      throw ProcessException("Binning : Factor as not been set");

    switch(mOperation)
    {
    case SUM:
      {
        if(((mXFactor == 2 && mYFactor == 2) ||
            (mXFactor == 4 && mYFactor == 4) ||
            (mXFactor == 8 && mYFactor == 8) ||
            (mXFactor == 16 && mYFactor == 16) ||
            (mXFactor == 32 && mYFactor == 32)) && // Factor 2 (Most used)
           !(aData.dimensions[0] % mXFactor) &&
           !(aData.dimensions[1] % mYFactor))
          {
            if(!_processingInPlaceFlag)
              {
                int aNewSize = aData.size() / (mYFactor * mXFactor);
                Buffer *aNewBuffer = new Buffer(aNewSize);
                aNewData.setBuffer(aNewBuffer);
                aNewBuffer->unref();
              }

            switch(aData.type)
            {
            case Data::UINT8:
              _binning2x2<unsigned char>(aData,aNewData,mXFactor);break;
            case Data::UINT16:
              _binning2x2<unsigned short>(aData,aNewData,mXFactor);break;
            case Data::UINT32:
              _binning2x2<unsigned int>(aData,aNewData,mXFactor);break;
            default:
              throw ProcessException("Binning : Data type not managed");
              break;
            }
          }
        else      // DEFAULT case is not optimized
          {
            int newWidth = aNewData.dimensions[0] / mXFactor;
            int newHeight = aNewData.dimensions[1] / mYFactor;
            int aNewSize = aData.depth() * newWidth * newHeight;
            Buffer *aNewBuffer = new Buffer(aNewSize);
            aNewData.setBuffer(aNewBuffer);
            aNewBuffer->unref();
            memset(aNewData.data(),0,aNewSize);
            aNewData.dimensions[0] /= mXFactor;
            aNewData.dimensions[1] /= mYFactor;

            switch(aData.type)
            {
            case Data::UINT8:
              _default_binning<unsigned char>(aData,aNewData,mXFactor,mYFactor);break;
            case Data::UINT16:
              _default_binning<unsigned short>(aData,aNewData,mXFactor,mYFactor);break;
            case Data::UINT32:
              _default_binning<unsigned int>(aData,aNewData,mXFactor,mYFactor);break;
            default:
              throw ProcessException("Binning : Data type not managed");
              break;
            }
            if(_processingInPlaceFlag) {
              memcpy(aData.data(),aNewData.data(),aNewData.size());
              aData.dimensions[0] = newWidth;
              aData.dimensions[1] = newHeight;
              return aData;
            }
      }
        break;
      }
    case MEAN:
    {
      if(_processingInPlaceFlag)
        {
          aNewData.dimensions[0] /= mXFactor;
          aNewData.dimensions[1] /= mYFactor;
        }
      else
        {
          int newWidth = aNewData.dimensions[0] / mXFactor;
          int newHeight = aNewData.dimensions[1] / mYFactor;
          int aNewSize = aData.depth() * newWidth * newHeight;
          Buffer *aNewBuffer = new Buffer(aNewSize);
          aNewData.setBuffer(aNewBuffer);
          aNewBuffer->unref();
          memset(aNewData.data(),0,aNewSize);
          aNewData.dimensions[0] /= mXFactor;
          aNewData.dimensions[1] /= mYFactor;
        }

      switch(aData.type)
      {
      case Data::UINT8:
        _mean_binning<unsigned char, short>(aData,aNewData,mXFactor,mYFactor);break;
      case Data::UINT16:
        _mean_binning<unsigned short, int>(aData,aNewData,mXFactor,mYFactor);break;
      case Data::UINT32:
        _mean_binning<unsigned int, long int>(aData,aNewData,mXFactor,mYFactor);break;
      default:
        throw ProcessException("Binning : Data type not managed");
        break;
      }
      break;
    }
    default:
      throw ProcessException("Binning : Operation not managed");
      break;
    }
  }
  return aNewData;
}

