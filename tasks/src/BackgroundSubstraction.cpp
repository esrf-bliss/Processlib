#include <iostream>
#ifdef __SSE2__
#include <emmintrin.h>
#endif
#include "BackgroundSubstraction.h"
using namespace Tasks;

template<class IN>
static void _substract_on_itself(void *src,int aSize,void *background)
{
  IN *aSrcPt,*aBackPt;
  aSrcPt = (IN*)src;
  aBackPt = (IN*)background;
  for(int i = aSize / sizeof(IN);i;--i,++aSrcPt,++aBackPt)
    {
      if(*aSrcPt > *aBackPt)
	*aSrcPt -= *aBackPt;
      else
	*aSrcPt = IN(0);
    }
}
#ifdef __SSE2__
template<>
static void _substract_on_itself<unsigned short>(void *source,int aSize,void *background)
{
  unsigned short *srcShort,*backgroundShort;
  srcShort = (unsigned short*)source;
  backgroundShort = (unsigned short*)background;
  int n = aSize >> 1;		// div / 2
  if(!((long)srcShort & 15))	// aligned to 128 bits
    {
      for(;n >= 8;n -= 8)
	{
	  __m128i src,back;
	  src = _mm_loadu_si128((__m128i*)srcShort);
	  back = _mm_loadu_si128((__m128i*)backgroundShort);
	  src = _mm_subs_epu16(src,back);
	  _mm_store_si128((__m128i*)srcShort,src);
	  srcShort += 8,backgroundShort += 8;
	}
    }
  for(;n;--n,++srcShort,++backgroundShort)
    {
      if(*srcShort > *backgroundShort)
	*srcShort -= *backgroundShort;
      else
	*srcShort = 0;
    }
}
#endif

template<class IN>
static void _substract(void *src,void *dst,int aSize,void *background)
{
  IN *aSrcPt,*aBackPt,*aDestPt;
  aSrcPt = (IN*)src;
  aBackPt = (IN*)background;
  aDestPt = (IN*)dst;
  for(int i = aSize / sizeof(IN);i;--i,++aSrcPt,++aBackPt,++aDestPt)
    {
      if(*aSrcPt > *aBackPt)
	*aDestPt = *aSrcPt - *aBackPt;
      else
	*aDestPt = IN(0);
    }
}

#ifdef __SSE2__
template<>
static void _substract<unsigned short>(void *source,void *dst,int aSize,void *background)
{
  unsigned short *srcShort,*backgroundShort,*destshort;
  srcShort = (unsigned short*)source;
  backgroundShort = (unsigned short*)background;
  destshort = (unsigned short*)dst;
  int n = aSize >> 1;		// div / 2
  if(!((long)srcShort & 15))	// aligned to 128 bits
    {
      for(;n >= 8;n -= 8)
	{
	  __m128i src,back;
	  src = _mm_loadu_si128((__m128i*)srcShort);
	  back = _mm_loadu_si128((__m128i*)backgroundShort);
	  src = _mm_subs_epu16(src,back);
	  _mm_store_si128((__m128i*)destshort,src);
	  srcShort += 8,backgroundShort += 8,destshort += 8;
	}
    }
  for(;n;--n,++srcShort,++backgroundShort)
    {
      if(*srcShort > *backgroundShort)
	*destshort = *srcShort - *backgroundShort;
      else
	*destshort = 0;
    }
}
#endif

BackgroundSubstraction::BackgroundSubstraction() : LinkTask()
{
}

BackgroundSubstraction::BackgroundSubstraction(const BackgroundSubstraction &aBackgroundSubstraction) :
  LinkTask(aBackgroundSubstraction),_backgroundImageData(aBackgroundSubstraction._backgroundImageData)
{
}

void BackgroundSubstraction::setBackgroundImageData(Data &aData)
{
  _backgroundImageData.setData(aData);
}

Data BackgroundSubstraction::process(Data &aData) 
{
  if(aData.type == _backgroundImageData.type &&
     aData.height == _backgroundImageData.height &&
     aData.width == _backgroundImageData.width)
    {
      if(_processingInPlaceFlag) // @todo processing in place
	{
	  switch(aData.type)
	    {
	    case Data::UINT8:
	      _substract_on_itself<unsigned char>(aData.data(),aData.size(),
						  _backgroundImageData.data());break;
	    case Data::UINT16:
	      _substract_on_itself<unsigned short>(aData.data(),aData.size(),
						   _backgroundImageData.data());break;
	    case Data::UINT32:
	      _substract_on_itself<unsigned int>(aData.data(),aData.size(),
						 _backgroundImageData.data());break;
	    default: break;
	    }
	  return aData;
	}
      else
	{
	  Data aNewData = aData.copyHeader(aData.type);	// get a new data buffer
	  switch(aData.type)
	    {
	    case Data::UINT8:
	      _substract<unsigned char>(aData.data(),aNewData.data(),aData.size(),
					_backgroundImageData.data());break;
	    case Data::UINT16:
	      _substract<unsigned short>(aData.data(),aNewData.data(),aData.size(),
					 _backgroundImageData.data());break;
	    case Data::UINT32:
	      _substract<unsigned int>(aData.data(),aNewData.data(),aData.size(),
				       _backgroundImageData.data());break;
	    default: break;
	    }
	  return aNewData;
	}
    }
  else
    std::cerr << "BackgroundSubstraction : Source image differ from background image" << std::endl;
  return Data();		// empty result
}

LinkTask* BackgroundSubstraction::copy() const
{
  return new BackgroundSubstraction(*this);
}
