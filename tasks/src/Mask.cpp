#include "Mask.h"
using namespace Tasks;

Mask::Mask() : LinkTask() {}

Mask::Mask(const Mask &aTask) :
  LinkTask(aTask),
  _MaskImage(aTask._MaskImage)
{}

void Mask::setMaskImageData(Data &aMaskImage)
{
  _MaskImage.setData(aMaskImage);
}

template<class IN,class MASK>
static void _mask_inplace(IN *src,int aSize,MASK *mask)
{
  for(int i = aSize / sizeof(IN);i;--i,++src,++mask)
    if(*mask) *src = IN(*mask);
}

template<class IN,class MASK>
static void _mask(IN *src,IN *dst,int aSize,MASK *mask)
{
  for(int i = aSize / sizeof(IN);i;--i,++src,++dst,++mask)
    {
      if(*mask) 
	*dst = IN(*mask);
      else
	*dst = *src;
    }
}

Data Mask::process(Data &aData)
{
  const char *errorMsgPt = NULL;

  if(aData.height == _MaskImage.height &&
     aData.width == _MaskImage.width)
    {
      if(_processingInPlaceFlag)
	{
	  switch(aData.type)
	    {
	    case Data::UINT8:
	      if(aData.type == _MaskImage.type)
		_mask_inplace((unsigned char*)aData.data(),aData.size(),
			      (unsigned char*)_MaskImage.data());
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
		  _mask_inplace((unsigned short*)aData.data(),aData.size(),
				(unsigned char*)_MaskImage.data());
		  break;
		case Data::UINT16:
		  _mask_inplace((unsigned short*)aData.data(),aData.size(),
				(unsigned short*)_MaskImage.data());
		  break;
		default:
		  errorMsgPt = "mask image must be an unsigned char or unsigned short";
		  goto error;
		}
	      break;
	    case Data::UINT32:
	      switch(_MaskImage.type)
		{
		case Data::UINT8:
		  _mask_inplace((unsigned int*)aData.data(),aData.size(),
				(unsigned char*)_MaskImage.data());
		  break;
		case Data::UINT16:
		  _mask_inplace((unsigned int*)aData.data(),aData.size(),
				(unsigned short*)_MaskImage.data());
		  break;
		case Data::UINT32:
		  _mask_inplace((unsigned int*)aData.data(),aData.size(),
				(unsigned int*)_MaskImage.data());
		  break;
		default:
		  errorMsgPt = "mask image must be an unsigned char,unsigned short or unsigned int";
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
		_mask((unsigned char*)aData.data(),(unsigned char*)aNewData.data(),
		      aData.size(),(unsigned char*)_MaskImage.data());
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
		  _mask((unsigned short*)aData.data(),(unsigned short*)aNewData.data(),
			aData.size(),(unsigned char*)_MaskImage.data());
		  break;
		case Data::UINT16:
		  _mask((unsigned short*)aData.data(),(unsigned short*)aNewData.data(),
			aData.size(),
			(unsigned short*)_MaskImage.data());
		  break;
		default:
		  errorMsgPt = "mask image must be an unsigned char or unsigned short";
		  goto error;
		}
	      break;
	    case Data::UINT32:
	      switch(_MaskImage.type)
		{
		case Data::UINT8:
		  _mask((unsigned int*)aData.data(),(unsigned int*)aNewData.data(),
			aData.size(),
			(unsigned char*)_MaskImage.data());
		  break;
		case Data::UINT16:
		  _mask((unsigned int*)aData.data(),(unsigned int*)aNewData.data(),
			aData.size(),
			(unsigned short*)_MaskImage.data());
		  break;
		case Data::UINT32:
		  _mask((unsigned int*)aData.data(),(unsigned int*)aNewData.data(),
			aData.size(),
			(unsigned int*)_MaskImage.data());
		  break;
		default:
		  errorMsgPt = "mask image must be an unsigned char,unsigned short or unsigned int";
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
    std::cerr << "Mask : " << errorMsgPt << std::endl;
  return Data();
}
