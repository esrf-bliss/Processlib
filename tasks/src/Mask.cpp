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
#ifdef __AVX__
#include <immintrin.h>
#endif
#include <stdio.h>
#include "processlib/ProcessExceptions.h"
#include "processlib/Mask.h"
using namespace Tasks;

Mask::Mask() : LinkTask(),_type(Mask::STANDARD) {}

Mask::Mask(const Mask &aTask) :
  LinkTask(aTask),
  _MaskImage(aTask._MaskImage),
  _type(Mask::STANDARD)
{}

void Mask::setMaskImageData(Data &aMaskImage)
{
  if(!aMaskImage.empty())
    {
      Data copiedData = aMaskImage.copy();
      _MaskImage.setData(copiedData);
    }
  else
    _MaskImage.setData(aMaskImage);
}

void Mask::setType(Mask::Type aType)
{
  _type = aType;
}

void Mask::getType(Mask::Type &aType) const
{
  aType = _type;
}
template<class INPUT,class MASK>
static void _mask_inplace(INPUT *src,int aSize,MASK *mask)
{
  for(int i = aSize / sizeof(INPUT);i;--i,++src,++mask)
    if(!*mask) *src = INPUT(0);
}

template<class INPUT,class MASK>
static void _mask(INPUT *src,INPUT *dst,int aSize,MASK *mask)
{
  for(int i = aSize / sizeof(INPUT);i;--i,++src,++dst,++mask)
    {
      if(!*mask) 
	*dst = INPUT(0);
      else
	*dst = *src;
    }
}

#ifdef __AVX2__
//			------- UINT8 -------
template<>
void _mask<unsigned char,unsigned char>(unsigned char *src,
					unsigned char *dst,int aSize,
					unsigned char *mask)
{
  int i = aSize;
  if(!((long)src & 31))		// aligned to 256 bits
    {
      __m256i cmp_to_0 = _mm256_set_epi32(0,0,0,0,0,0,0,0);
      for(;i >= 32;i -= 32,dst += 32,src += 32,mask += 32)
	{
	  __m256i mask_src = _mm256_stream_load_si256((__m256i*)mask);
	  __m256i src_regiter = _mm256_stream_load_si256((__m256i*)src);
	  __m256i mask_register = _mm256_cmpgt_epi8 (mask_src,cmp_to_0);
	  __m256i dst_regiter = _mm256_and_si256(src_regiter,mask_register);
	  _mm256_stream_si256((__m256i*)dst,dst_regiter);
	}
    }
  for(;i;--i,++src,++dst,++mask)
    {
      if(!*mask) 
	*dst = 0;
      else
	*dst = *src;
    }
}
template<>
void _mask<char,char>(char *src,
		      char *dst,int aSize,
		      char *mask)
{
  _mask((unsigned char*)src,(unsigned char*)dst,
	aSize,mask);
}

//			------- UINT16 -------
template<>
void _mask<unsigned short,unsigned char>(unsigned short *src,
					 unsigned short *dst,int aSize,
					 unsigned char *mask)
{
  int i = aSize / 2;
  if(!((long)src & 31))		// aligned to 256 bits
    {
      __m256i cmp_to_0 = _mm256_set_epi32(0,0,0,0,0,0,0,0);
      for(;i >= 32;i -= 32,dst += 32,src += 32,mask += 32)
	{
	  __m256i mask_src = _mm256_stream_load_si256((__m256i*)mask);
	  __m256i src1_regiter = _mm256_stream_load_si256((__m256i*)src);
	  __m256i src2_regiter = _mm256_stream_load_si256((__m256i*)(src + 16));

	  __m128i mask1,mask2;
	  _mm256_storeu2_m128i (&mask2,&mask1,mask_src);
	  //mask src1
	  __m256i mask_tmp1 = _mm256_cvtepu8_epi16(mask1);
	  __m256i mask_src1 = _mm256_cmpgt_epi16(mask_tmp1,cmp_to_0);
	  __m256i dst1_regiter = _mm256_and_si256(src1_regiter,mask_src1);
	  _mm256_stream_si256((__m256i*)dst,dst1_regiter);
	  //mask src2
	  __m256i mask_tmp2 = _mm256_cvtepu8_epi16(mask2);
	  __m256i mask_src2 = _mm256_cmpgt_epi16(mask_tmp2,cmp_to_0);
	  __m256i dst2_regiter = _mm256_and_si256(src2_regiter,mask_src2);
	  _mm256_stream_si256((__m256i*)(dst + 16),dst2_regiter);
	}
    }
  for(;i;--i,++src,++dst,++mask)
    {
      if(!*mask) 
	*dst = 0;
      else
	*dst = *src;
    }
}
template<>
void _mask<short,unsigned char>(short *src,
				short *dst,int aSize,
				unsigned char *mask)
{
  _mask((unsigned short*)src,(unsigned short*)dst,
	aSize,mask);
}
//			------- UINT32 -------
template<>
void _mask<unsigned int,unsigned char>(unsigned int *src,
					 unsigned int *dst,int aSize,
					 unsigned char *mask)
{
  int i = aSize / 4;
  if(!((long)src & 31))		// aligned to 256 bits
    {
      __m256i cmp_to_0 = _mm256_set_epi32(0,0,0,0,0,0,0,0);
      for(;i >= 32;i -= 32,dst += 32,src += 32,mask += 32)
	{
	  __m256i mask_src = _mm256_stream_load_si256((__m256i*)mask);
	  __m256i src1_regiter = _mm256_stream_load_si256((__m256i*)src);
	  __m256i src2_regiter = _mm256_stream_load_si256((__m256i*)(src + 8));
	  __m256i src3_regiter = _mm256_stream_load_si256((__m256i*)(src + 16));
	  __m256i src4_regiter = _mm256_stream_load_si256((__m256i*)(src + 24));

	  __m128i mask1,mask3;
	  _mm256_storeu2_m128i (&mask3,&mask1,mask_src);
	  __m128i mask2,mask4;
	  __m256i mask_src_2 = _mm256_bsrli_epi128 (mask_src, 8);
	  _mm256_storeu2_m128i (&mask4,&mask2,mask_src_2);
	  //mask src1
	  __m256i mask_tmp1 = _mm256_cvtepu8_epi32(mask1);
	  __m256i mask_src1 = _mm256_cmpgt_epi32(mask_tmp1,cmp_to_0);
	  __m256i dst1_regiter = _mm256_and_si256(src1_regiter,mask_src1);
	  _mm256_stream_si256((__m256i*)dst,dst1_regiter);
	  //mask src2
	  __m256i mask_tmp2 = _mm256_cvtepu8_epi32(mask2);
	  __m256i mask_src2 = _mm256_cmpgt_epi32(mask_tmp2,cmp_to_0);
	  __m256i dst2_regiter = _mm256_and_si256(src2_regiter,mask_src2);
	  _mm256_stream_si256((__m256i*)(dst + 8),dst2_regiter);
	  //mask src3
	  __m256i mask_tmp3 = _mm256_cvtepu8_epi32(mask3);
	  __m256i mask_src3 = _mm256_cmpgt_epi32(mask_tmp3,cmp_to_0);
	  __m256i dst3_regiter = _mm256_and_si256(src3_regiter,mask_src3);
	  _mm256_stream_si256((__m256i*)(dst + 16),dst3_regiter);
	  //mask src4
	  __m256i mask_tmp4 = _mm256_cvtepu8_epi32(mask4);
	  __m256i mask_src4 = _mm256_cmpgt_epi32(mask_tmp4,cmp_to_0);
	  __m256i dst4_regiter = _mm256_and_si256(src4_regiter,mask_src4);
	  _mm256_stream_si256((__m256i*)(dst + 24),dst4_regiter);
	}
    }
  for(;i;--i,++src,++dst,++mask)
    {
      if(!*mask) 
	*dst = 0;
      else
	*dst = *src;
    }
}
template<>
void _mask<int,unsigned char>(int *src,
			      int *dst,int aSize,
			      unsigned char *mask)
{
  _mask((unsigned int*)src,(unsigned int*)dst,
	aSize,mask);
}

#endif

template<class INPUT,class MASK>
static void _dummy_inplace(INPUT *src,int aSize,MASK *mask)
{
  for(int i = aSize / sizeof(INPUT);i;--i,++src,++mask)
    if(*mask) *src = INPUT(*mask);
}

template<class INPUT,class MASK>
static void _dummy(INPUT *src,INPUT *dst,int aSize,MASK *mask)
{
  for(int i = aSize / sizeof(INPUT);i;--i,++src,++dst,++mask)
    {
      if(*mask) 
	*dst = INPUT(*mask);
      else
	*dst = *src;
    }
}

Data Mask::process(Data &aData)
{
#define _INPLACE(data,size,datamask)		\
  if(_type == Mask::STANDARD)			\
    _mask_inplace(data,size,datamask);		\
  else						\
    _dummy_inplace(data,size,datamask);

#define _COPY(src,dst,size,datamask)		\
  if(_type == Mask::STANDARD)			\
    _mask(src,dst,size,datamask);		\
  else						\
    _dummy(src,dst,size,datamask);

    const char *errorMsgPt = NULL;
    if(aData.dimensions == _MaskImage.dimensions)
      {
	if(_processingInPlaceFlag)
	  {
	    switch(aData.type)
	      {
	      case Data::UINT8:
		if(aData.type == _MaskImage.type)
		  {
		    _INPLACE((unsigned char*)aData.data(),aData.size(),
			     (unsigned char*)_MaskImage.data());
		  }
		else
		  {
		    errorMsgPt = "mask image must be an unsigned char";
		    goto error;
		  }
		break;
	      case Data::UINT16:
		switch(_MaskImage.type)
		  {
		  case Data::UINT8:
		    _INPLACE((unsigned short*)aData.data(),aData.size(),
			     (unsigned char*)_MaskImage.data());
		    break;
		  case Data::UINT16:
		    _INPLACE((unsigned short*)aData.data(),aData.size(),
			     (unsigned short*)_MaskImage.data());
		    break;
		  default:
		    errorMsgPt = "mask image must be an unsigned char or unsigned short";
		    goto error;
		  }
		break;
	      case Data::INT16:
		switch(_MaskImage.type)
		  {
		  case Data::UINT8:
		    _INPLACE((short*)aData.data(),aData.size(),
			     (unsigned char*)_MaskImage.data());
		    break;
		  case Data::UINT16:
		    _INPLACE((short*)aData.data(),aData.size(),
			     (unsigned short*)_MaskImage.data());
		    break;
		  case Data::INT16:
		    _INPLACE((short*)aData.data(),aData.size(),
			     (short*)_MaskImage.data());
		    break;
		  default:
		    errorMsgPt = "mask image must be an unsigned char, unsigned short or short";
		    goto error;
		  }
		break;
	      case Data::UINT32:
		switch(_MaskImage.type)
		  {
		  case Data::UINT8:
		    _INPLACE((unsigned int*)aData.data(),aData.size(),
			     (unsigned char*)_MaskImage.data());
		    break;
		  case Data::UINT16:
		    _INPLACE((unsigned int*)aData.data(),aData.size(),
			     (unsigned short*)_MaskImage.data());
		    break;
		  case Data::UINT32:
		    _INPLACE((unsigned int*)aData.data(),aData.size(),
			     (unsigned int*)_MaskImage.data());
		    break;
		  default:
		    errorMsgPt = "mask image must be an unsigned char,unsigned short or unsigned int";
		    goto error;
		  }
		break;
	      case Data::INT32:
		switch(_MaskImage.type)
		  {
		  case Data::UINT8:
		    _INPLACE((int*)aData.data(),aData.size(),
			     (unsigned char*)_MaskImage.data());
		    break;
		  case Data::UINT16:
		    _INPLACE((int*)aData.data(),aData.size(),
			     (unsigned short*)_MaskImage.data());
		    break;
		  case Data::UINT32:
		    _INPLACE((int*)aData.data(),aData.size(),
			     (unsigned int*)_MaskImage.data());
		    break;
		  case Data::INT32:
		    _INPLACE((int*)aData.data(),aData.size(),
			     (int*)_MaskImage.data());
		    break;
		  default:
		    errorMsgPt = "mask image must be an unsigned char,unsigned short,unsigned int or int";
		    goto error;
		  }
		break;
	      default: 
		errorMsgPt = "Data type not managed";
		goto error;
	      }
	    return aData;
	  }
	else
	  {
	    Data aNewData = aData.copyHeader(aData.type);	// get a new data buffer
	    switch(aData.type)
	      {
	      case Data::UINT8:
		if(aData.type == _MaskImage.type)
		  {
		    _COPY((unsigned char*)aData.data(),(unsigned char*)aNewData.data(),
			  aData.size(),(unsigned char*)_MaskImage.data());
		  }
		else
		  {
		    errorMsgPt = "mask image must be an unsigned char";
		    goto error;
		  }
		break;
	      case Data::UINT16:
		switch(_MaskImage.type)
		  {
		  case Data::UINT8:
		    _COPY((unsigned short*)aData.data(),(unsigned short*)aNewData.data(),
			  aData.size(),(unsigned char*)_MaskImage.data());
		    break;
		  case Data::UINT16:
		    _COPY((unsigned short*)aData.data(),(unsigned short*)aNewData.data(),
			  aData.size(),
			  (unsigned short*)_MaskImage.data());
		    break;
		  default:
		    errorMsgPt = "mask image must be an unsigned char or unsigned short";
		    goto error;
		  }
		break;
	      case Data::INT16:
		switch(_MaskImage.type)
		  {
		  case Data::UINT8:
		    _COPY((short*)aData.data(),(short*)aNewData.data(),
			  aData.size(),(unsigned char*)_MaskImage.data());
		    break;
		  case Data::UINT16:
		    _COPY((short*)aData.data(),(short*)aNewData.data(),
			  aData.size(),(unsigned short*)_MaskImage.data());
		    break;
		  case Data::INT16:
		    _COPY((short*)aData.data(),(short*)aNewData.data(),
			  aData.size(),(short*)_MaskImage.data());
		    break;
		  default:
		    errorMsgPt = "mask image must be an unsigned char, unsigned short or short";
		    goto error;
		  }
		break;
	      case Data::UINT32:
		switch(_MaskImage.type)
		  {
		  case Data::UINT8:
		    _COPY((unsigned int*)aData.data(),(unsigned int*)aNewData.data(),
			  aData.size(),
			  (unsigned char*)_MaskImage.data());
		    break;
		  case Data::UINT16:
		    _COPY((unsigned int*)aData.data(),(unsigned int*)aNewData.data(),
			  aData.size(),
			  (unsigned short*)_MaskImage.data());
		    break;
		  case Data::UINT32:
		    _COPY((unsigned int*)aData.data(),(unsigned int*)aNewData.data(),
			  aData.size(),
			  (unsigned int*)_MaskImage.data());
		    break;
		  default:
		    errorMsgPt = "mask image must be an unsigned char,unsigned short or unsigned int";
		    goto error;
		  }
	      case Data::INT32:
		switch(_MaskImage.type)
		  {
		  case Data::UINT8:
		    _COPY((int*)aData.data(),(int*)aNewData.data(),
			  aData.size(),
			  (unsigned char*)_MaskImage.data());
		    break;
		  case Data::UINT16:
		    _COPY((int*)aData.data(),(int*)aNewData.data(),
			  aData.size(),
			  (unsigned short*)_MaskImage.data());
		    break;
		  case Data::INT32:
		    _COPY((int*)aData.data(),(int*)aNewData.data(),
			  aData.size(),
			  (int*)_MaskImage.data());
		    break;
		  case Data::UINT32:
		    _COPY((int*)aData.data(),(int*)aNewData.data(),
			  aData.size(),
			  (unsigned int*)_MaskImage.data());
		    break;
		  default:
		    errorMsgPt = "mask image must be an unsigned char,unsigned short,unsigned int or int";
		    goto error;
		  }
		break;
	      default: 
		errorMsgPt = "Data type not managed";
		goto error;
	      }
	    return aNewData;
	  }
      }
    else
      errorMsgPt = "Source image differ from mask image";

  error:
    if(errorMsgPt)
      {
	char aBuffer[256];
	snprintf(aBuffer,sizeof(aBuffer),"Mask : %s",errorMsgPt);
	throw ProcessException(aBuffer);
      }
    return Data();
  }
