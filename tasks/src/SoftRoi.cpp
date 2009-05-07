#include "SoftRoi.h"
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

static inline int min(int a,int b) {return a < b ? a : b;}

Data SoftRoi::process(Data &aData)
{
  if(_x1 >= 0 && _y1 >= 0 &&
     _x2 >=0 && _y2 >= 0)
    {
      int depth = aData.depth();
      int startLineId = min(_y1,aData.height - 1);
      int endLineId = min(_y2 + 1,aData.height);
      int startColumnId = min(_x1,aData.width - 1);
      int endColumnId = min(_x2 + 1,aData.width);
      int cropSize = (endColumnId - startColumnId) * depth;
      int lineSize = aData.width * depth;
      char *aSrcPt = (char*)aData.data();
      aSrcPt += (startLineId * aData.width + startColumnId) * depth;
      if(_processingInPlaceFlag)
	{
	  char *aDst = (char*)aData.data();
	  for(int lineId = startLineId;lineId < endLineId;
	      ++lineId,aSrcPt += lineSize,aDst += cropSize)
	    memmove(aDst,aSrcPt,cropSize);
	  aData.width = endColumnId - startColumnId;
	  aData.height = endLineId - startLineId;
	  return aData;
	}
      else
	{
	  Data aNewData;
	  aNewData.width = endColumnId - startColumnId;
	  aNewData.height = endLineId - startLineId;
	  aNewData.type = aData.type;
	  aNewData.frameNumber = aData.frameNumber;
	  aNewData.buffer = new Buffer(cropSize * aNewData.height);
	  char *aDst = (char*)aNewData.buffer->data;
	  for(int lineId = startLineId;lineId < endLineId;
	      ++lineId,aSrcPt += lineSize,aDst += cropSize)
	    memcpy(aDst,aSrcPt,cropSize);
	  return aNewData;
	}
    } 
  return Data();		// ERROR
}
