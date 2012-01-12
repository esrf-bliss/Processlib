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
#include "ProcessExceptions.h"
#include "SoftRoi.h"
#include "Stat.h"
#include <string.h>
using namespace Tasks;

SoftRoi::SoftRoi() : 
  LinkTask(),
  _x1(-1),_x2(-1),
  _y1(-1),_y2(-1)
{}

SoftRoi::SoftRoi(const SoftRoi &anOther) :
  LinkTask(anOther),
  _x1(anOther._x1),_x2(anOther._x2),
  _y1(anOther._y1),_y2(anOther._y2)
{
}
/** @brief set the roi 
    @param x1 the first column
    @param x2 the last column
    @param y1 the first line
    @param y2 the last line
*/
void SoftRoi::setRoi(int x1,int x2,
		     int y1,int y2)
{
   _x1 = x1,_x2 = x2;
   _y1 = y1,_y2 = y2;
}

#ifdef __unix
static inline int min(int a,int b) {return a < b ? a : b;}
#endif

Data SoftRoi::process(Data &aData)
{
  if(aData.dimensions.size() != 2)
    throw ProcessException("SoftRoi : Only manage 2D data");
  else if(_x1 >= 0 && _y1 >= 0 &&
	  _x2 >=0 && _y2 >= 0)
    {
      int depth = aData.depth();
      int startLineId = min(_y1,aData.dimensions[1] - 1);
      int endLineId = min(_y2 + 1,aData.dimensions[1]);
      int startColumnId = min(_x1,aData.dimensions[0] - 1);
      int endColumnId = min(_x2 + 1,aData.dimensions[0]);
      int cropSize = (endColumnId - startColumnId) * depth;
      int lineSize = aData.dimensions[0] * depth;

      std::stringstream info;
      info << "SoftRoi " ;
      info << "(" << startColumnId;
      info << "," << startLineId << ")";
      info << " (" << endColumnId - 1;
      info << "," << endLineId - 1 << ")";
      Stat aStat(aData,info.str()); 

      char *aSrcPt = (char*)aData.data();
      aSrcPt += (startLineId * aData.dimensions[0] + startColumnId) * depth;
      if(_processingInPlaceFlag)
	{
	  char *aDst = (char*)aData.data();
	  for(int lineId = startLineId;lineId < endLineId;
	      ++lineId,aSrcPt += lineSize,aDst += cropSize)
	    memmove(aDst,aSrcPt,cropSize);
	  aData.dimensions[0] = (endColumnId - startColumnId);
	  aData.dimensions[1] = (endLineId - startLineId);
	  return aData;
	}
      else
	{
	  Data aNewData;
	  aNewData.dimensions.push_back(endColumnId - startColumnId);
	  aNewData.dimensions.push_back(endLineId - startLineId);
	  aNewData.type = aData.type;
	  aNewData.frameNumber = aData.frameNumber;
	  aNewData.timestamp = aData.timestamp;
	  aNewData.header = aData.header;
	  aNewData.buffer = new Buffer(cropSize * aNewData.dimensions[1]);
	  char *aDst = (char*)aNewData.buffer->data;
	  for(int lineId = startLineId;lineId < endLineId;
	      ++lineId,aSrcPt += lineSize,aDst += cropSize)
	    memcpy(aDst,aSrcPt,cropSize);
	  return aNewData;
	}
    } 
  return Data();		// ERROR
}
