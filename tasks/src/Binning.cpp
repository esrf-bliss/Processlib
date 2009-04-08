#include "Binning.h"
#include <iostream>

using namespace Tasks;

//static function
template<class IN> static IN max_value(const IN &)
{
  return IN((1ULL << (8 * sizeof(IN))) - 1);
}

/** @brief generique binning but not optimized at all
 */
template<class IN>
inline void _default_binning(Data &aSrcData,Data &aDstData,
			     int xFactor,int yFactor)
{
  IN *aSrcPt = (IN*)aSrcData.data();
  IN *aDstPt = (IN*)aDstData.data();
  IN MAX_VALUE = max_value(*aSrcPt);
  int *lineOffset = new int[yFactor];
  int aTmpOffSet = 0;
  for(int linId = 0;linId < yFactor;++linId,aTmpOffSet += aSrcData.width)
    lineOffset[linId] = aTmpOffSet;

  IN *aLineSrcPt = aSrcPt;
  IN *aLineDstPt = aDstPt;
  for(int lineId = 0;lineId <= (aSrcData.height - yFactor);
      lineId += yFactor,aLineSrcPt += aSrcData.width,aLineDstPt += aDstData.width)
    {
      int aDstColumnId = 0;
      for(int columnId = 0;columnId <= (aSrcData.width - xFactor);
	  columnId += xFactor,++aDstColumnId)
	{
	  unsigned long long result = 0;
	  for(int aPixelLine = 0;aPixelLine < yFactor;++aPixelLine)
	    for(int aPixelColumn = columnId;aPixelColumn < columnId + xFactor;++aPixelColumn)
	      result += *(aLineSrcPt + lineOffset[aPixelLine] + aPixelColumn);

	  if(result > MAX_VALUE)
	    aLineDstPt[aDstColumnId] = MAX_VALUE;
	  else
	    aLineDstPt[aDstColumnId] = IN(result);
	}
    }
  delete lineOffset;
}

/** @brief binning 2 x 2
 */
template<class IN>
static void _binning2x2(Data &aSrcData,Data &aDstData,int Factor)
{

  IN *aSrcFirstLinePt = (IN*)aSrcData.data();
  IN *aSrcSecondLinePt = aSrcFirstLinePt + aSrcData.width;
  IN *aDstPt = (IN*)aDstData.data();
  IN MAX_VALUE = max_value(*aDstPt);
  for(int lineId = 0;lineId < aSrcData.height;lineId += 2)
    {
      for(int columnId = 0;columnId < aSrcData.width;columnId += 2,
	    ++aDstPt,aSrcFirstLinePt += 2,aSrcSecondLinePt += 2)
	{
	  unsigned long long result = *aSrcFirstLinePt + *(aSrcFirstLinePt + 1) +
	    *aSrcSecondLinePt + *(aSrcSecondLinePt + 1);
	  if(result > MAX_VALUE)
	    *aDstPt = MAX_VALUE;
	  else
	    *aDstPt = IN(result);
	}
      aSrcFirstLinePt = aSrcSecondLinePt;
      aSrcSecondLinePt = aSrcFirstLinePt + aSrcData.width;
    }
  if(Factor > 2)
    _binning2x2<IN>(aDstData,aDstData,Factor >> 1);
}

Binning::Binning() : mXFactor(-1),mYFactor(-1) {};
Binning::Binning(const Binning &anOther) :
  Task(),
  mXFactor(anOther.mXFactor),
  mYFactor(anOther.mYFactor) {}

Task* Binning::copy() const
{
  return new Binning(*this);
}

Data Binning::process(Data &aData)
{
  Data aNewData;
  if(!aData.empty())
    {
      if(mYFactor > 0 && mXFactor > 0)
	{
	  aNewData.type = aData.type;
	  aNewData.width = aData.width / mXFactor;
	  aNewData.height = aData.height / mYFactor;
	  aNewData.frameNumber = aData.frameNumber;
	  if(mXFactor == 2 && mYFactor == 2 ||
	     mXFactor == 4 && mYFactor == 4 ||
	     mXFactor == 8 && mYFactor == 8 ||
	     mXFactor == 16 && mYFactor == 16 ||
	     mXFactor == 32 && mYFactor == 32) // Factor 2 (Most used)
	    {
	      if(_processingInPlaceFlag)
		{
		  aNewData.buffer = aData.buffer;
		  if(aNewData.buffer) aNewData.buffer->ref();
		}
	      else
		aNewData.buffer = new Buffer(aData.size() >> 2);

	      switch(aData.type)
		{
		case Data::UINT8:
		  _binning2x2<unsigned char>(aData,aNewData,mXFactor);break;
		case Data::UINT16:
		  _binning2x2<unsigned short>(aData,aNewData,mXFactor);break;
		case Data::UINT32:
		  _binning2x2<unsigned int>(aData,aNewData,mXFactor);break;
		default:
		  std::cerr << "Binning : Data type not managed" << std::endl;
		  break;
		}	  
	    }
	  else			// DEFAULT case is not optimized
	    {
	      if(_processingInPlaceFlag)
		{
		  aNewData.buffer = aData.buffer;
		  if(aNewData.buffer) aNewData.buffer->ref();
		}
	      else
		aNewData.buffer = new Buffer(aNewData.size());

	      switch(aData.type)
		{
		case Data::UINT8:
		  _default_binning<unsigned char>(aData,aNewData,mXFactor,mYFactor);break;
		case Data::UINT16:
		  _default_binning<unsigned short>(aData,aNewData,mXFactor,mYFactor);break;
		case Data::UINT32:
		  _default_binning<unsigned int>(aData,aNewData,mXFactor,mYFactor);break;
		default:
		  std::cerr << "Binning : Data type not managed" << std::endl;
		  break;
		}
	    }
	}
      else
	std::cerr << "Binning : Factor as not been set" << std::endl;
    }
  else
    std::cerr << "Binning : Data is empty!" << std::endl;
  return aNewData;
}

