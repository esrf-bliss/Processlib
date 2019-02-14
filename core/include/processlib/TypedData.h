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

#pragma once

#if !defined(PROCESSLIB_TYPEDDATA_H)
#define PROCESSLIB_TYPEDDATA_H

#ifndef __unix
#pragma warning(disable : 4251)
#pragma warning(disable : 4244)
#endif

#include <array>
#include <memory>
#include <type_traits>

#include <processlib_export.h>

//#include "processlib/Buffer.h"
#include "processlib/Metadata.h"
#include "processlib/ProcessExceptions.h"

enum class PixelType { UNDEF, UINT8, INT8, UINT16, INT16, UINT32, INT32, UINT64, INT64, FLOAT, DOUBLE };

namespace detail 
{
    template <typename T> struct pixel_traits {};
    template <> struct pixel_traits<uint8_t>
    {
        inline static const char* Desc = "UINT8";
        inline static const PixelType Type = PixelType::UINT8;
    };
    template <> struct pixel_traits<int8_t>
    {
        inline static const char* Desc = "INT8";
        inline static const PixelType Type = PixelType::INT8;
    };
    template <> struct pixel_traits<uint16_t>
    {
        inline static const char* Desc = "UINT16";
        inline static const PixelType Type = PixelType::UINT16;
    };
    template <> struct pixel_traits<int16_t>
    {
        inline static const char* Desc = "INT16";
        inline static const PixelType Type = PixelType::INT16;
    };
    template <> struct pixel_traits<int32_t>
    {
        inline static const char* Desc = "UINT32";
        inline static const PixelType Type = PixelType::UINT32;
    };
    template <> struct pixel_traits<uint32_t>
    {
        inline static const char* Desc = "INT32";
        inline static const PixelType Type = PixelType::INT32;
    };
    template <> struct pixel_traits<uint64_t>
    {
        inline static const char* Desc = "UINT64";
        inline static const PixelType Type = PixelType::UINT64;
    };
    template <> struct pixel_traits<int64_t>
    {
        inline static const char* Desc = "INT64";
        inline static const PixelType Type = PixelType::INT64;
    };
    template <> struct pixel_traits<float>
    {
        inline static const char* Desc = "FLOAT";
        inline static const PixelType Type = PixelType::FLOAT;
    };
    template <> struct pixel_traits<double>
    {
        inline static const char* Desc = "DOUBLE";
        inline static const PixelType Type = PixelType::DOUBLE;
    };
}


template <typename Pixel, typename Allocator = std::allocator<Pixel> >
struct TypedData
{
  // Pixel type
  using pixel_t = Pixel;
  using allocator_t = Allocator;
  using buffer_t = std::shared_ptr<Pixel[]>;

  TypedData(const Allocator& alloc = Allocator()) : m_alloc(alloc), frameNumber(-1), timestamp(0.), buffer(NULL) {}

  /// Assign the buffer and dimensions of rhs only
  void setData(const TypedData &rhs)
  {
    setBuffer(rhs.buffer);
    //type       = rhs.type;
    dimensions = rhs.dimensions;
  }

  void setBuffer(buffer_t aBuffer)
  {
    // if (aBuffer) aBuffer->ref();
    // if (buffer) buffer->unref();
    buffer = aBuffer;
  }

  // void releaseBuffer()
  // {
  //   if (buffer) buffer->unref();
  //   buffer = NULL;
  //   //type = UNDEF;
  //   frameNumber = -1;
  // }

  inline void *data() { return buffer->get(); }
  inline const void *data() const { return buffer->get(); }

  inline std::size_t size() const
  {
    std::size_t res = depth();
    for (auto && dim : dimensions)
      res *= dim;
    return res;
  }

  inline std::size_t depth() const
  {
    return sizeof(pixel_t);
  }

  inline bool is_signed() const
  {
      return std::is_signed<pixel_t>::value;
  }

  /// Check whether the buffer is empty
  inline bool empty() const { return buffer; }

  /// Returns a data with the same header but with a newly allocated buffer
  template <typename Pixel2, typename Allocator2 = Allocator>
  inline auto copyHeader()
  {
    TypedData<Pixel2, Allocator2> res;
    //res.type        = aType;
    res.m_alloc  = m_alloc;
    res.dimensions  = dimensions;
    res.frameNumber = frameNumber;
    res.timestamp   = timestamp;
    // aReturnData.header = header;
    res.buffer = std::allocate_shared<char>(m_alloc, res.size());
    return res;
  }

  inline TypedData copy() const
  {
    TypedData res;
    //res.type        = type;
    res.m_alloc  = m_alloc;
    res.dimensions  = dimensions;
    res.frameNumber = frameNumber;
    res.timestamp   = timestamp;
    // aReturnData.header = header;
    if (buffer)
    {
      res.buffer = std::allocate_shared<char>(m_alloc, size());
      memcpy(res.data(), data(), size());
    }
    else
      res.buffer = NULL;
    return res;
  }

  //inline Data cast(Data::TYPE aType);
  
  inline auto mask() const
  {
    TypedData<int8_t, Allocator> res(m_alloc);
    res.dimensions  = dimensions;
    res.frameNumber = frameNumber;
    res.timestamp   = timestamp;
    // aReturnData.header = header;
    if (depth() != 1) {
      //aReturnData.type   = INT8;
      res.buffer = std::allocate_shared<char>(m_alloc, res.size());
      make_mask(*this, res);
    } else {
      //res.type = type;
      res.setBuffer(buffer);
    }
    return res;
  }
  
  using iterator = Pixel *;
  using const_iterator = Pixel const *;
  
  iterator begin() { return buffer.get(); }
  const_iterator begin() const { return buffer.get(); }
  iterator end() { return buffer.get() + size(); }
  const_iterator end() const { return buffer.get() + size(); }

  //TYPE type;
  Allocator m_alloc;             //<! Allocator of the data
  std::array<int, 2> dimensions; //<! Dimensions of the data
  int frameNumber;               //<! Frame number (starting from 0)
  double timestamp;              //<! Unix timestamp
  // mutable HeaderContainer 	header;
  //mutable Buffer *buffer;
  buffer_t buffer;              //<! The (shared) buffer
};


// std::for_each bu on TypedData
template <typename Pixel, typename Allocator, typename UnaryFunction>
UnaryFunction for_each(const TypedData<Pixel, Allocator> &range, UnaryFunction f)
{
    return std::for_each(range.begin(), range.end(), f);
}

template <typename Pixel1, typename Pixel2, typename Allocator, typename UnaryFunction>
void transform(
    const TypedData<Pixel1, Allocator> &src,
    TypedData<Pixel2, Allocator> &dst,
    UnaryFunction f)
{
    return std::transform(src.begin(), src.end(), dst.begin(), f);
}


template <typename Input, typename Output, typename Allocator>
static void make_mask(const TypedData<Input, Allocator> &src, TypedData<Output, Allocator> &dst)
{
  int pixelnb = src.size() / src.depth();
  transform(src, dst, [](auto src) {
      return char(src);
  });
}

template <class Output, class Input>
static void _cast(Output *dst, Input *src, int nbItems)
{
  while (nbItems) {
    *dst = OUTPUT(*src);
    ++dst, ++src, --nbItems;
  }
}

// Data Data::cast(Data::TYPE aType)
// {
//   if (aType == type) return *this;
// 
//   Data aReturnData;
//   aReturnData.type        = aType;
//   aReturnData.dimensions  = dimensions;
//   aReturnData.frameNumber = frameNumber;
//   aReturnData.timestamp   = timestamp;
//   // aReturnData.header = header;
// 
//   aReturnData.buffer = new Buffer(aReturnData.size());
//   int nbItems        = aReturnData.size() / aReturnData.depth();
// 
//   switch (type) {
//   case UINT8:
//     switch (aReturnData.type) {
//     case UINT16:
//       Data::_cast((unsigned short *)aReturnData.data(), (unsigned char *)data(), nbItems);
//       break;
//     case INT16:
//       Data::_cast((short *)aReturnData.data(), (unsigned char *)data(), nbItems);
//       break;
//     case UINT32:
//       Data::_cast((unsigned int *)aReturnData.data(), (unsigned char *)data(), nbItems);
//       break;
//     case INT32:
//       Data::_cast((int *)aReturnData.data(), (unsigned char *)data(), nbItems);
//       break;
//     case UINT64:
//       Data::_cast((unsigned long long *)aReturnData.data(), (unsigned char *)data(), nbItems);
//       break;
//     case INT64:
//       Data::_cast((long long *)aReturnData.data(), (unsigned char *)data(), nbItems);
//       break;
//     case FLOAT:
//       Data::_cast((float *)aReturnData.data(), (unsigned char *)data(), nbItems);
//       break;
//     case DOUBLE:
//       Data::_cast((double *)aReturnData.data(), (unsigned char *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   case INT8:
//     switch (aReturnData.type) {
//     case UINT16:
//       Data::_cast((unsigned short *)aReturnData.data(), (char *)data(), nbItems);
//       break;
//     case INT16:
//       Data::_cast((short *)aReturnData.data(), (char *)data(), nbItems);
//       break;
//     case UINT32:
//       Data::_cast((unsigned int *)aReturnData.data(), (char *)data(), nbItems);
//       break;
//     case INT32:
//       Data::_cast((int *)aReturnData.data(), (char *)data(), nbItems);
//       break;
//     case UINT64:
//       Data::_cast((unsigned long long *)aReturnData.data(), (char *)data(), nbItems);
//       break;
//     case INT64:
//       Data::_cast((long long *)aReturnData.data(), (char *)data(), nbItems);
//       break;
//     case FLOAT:
//       Data::_cast((float *)aReturnData.data(), (char *)data(), nbItems);
//       break;
//     case DOUBLE:
//       Data::_cast((double *)aReturnData.data(), (char *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   case UINT16:
//     switch (aReturnData.type) {
//     case UINT32:
//       Data::_cast((unsigned int *)aReturnData.data(), (unsigned short *)data(), nbItems);
//       break;
//     case INT32:
//       Data::_cast((int *)aReturnData.data(), (unsigned short *)data(), nbItems);
//       break;
//     case UINT64:
//       Data::_cast((unsigned long long *)aReturnData.data(), (unsigned short *)data(), nbItems);
//       break;
//     case INT64:
//       Data::_cast((long long *)aReturnData.data(), (unsigned short *)data(), nbItems);
//       break;
//     case FLOAT:
//       Data::_cast((float *)aReturnData.data(), (unsigned short *)data(), nbItems);
//       break;
//     case DOUBLE:
//       Data::_cast((double *)aReturnData.data(), (unsigned short *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   case INT16:
//     switch (aReturnData.type) {
//     case UINT16:
//       Data::_cast((unsigned short *)aReturnData.data(), (short *)data(), nbItems);
//       break;
//     case UINT32:
//       Data::_cast((unsigned int *)aReturnData.data(), (short *)data(), nbItems);
//       break;
//     case INT32:
//       Data::_cast((int *)aReturnData.data(), (short *)data(), nbItems);
//       break;
//     case UINT64:
//       Data::_cast((unsigned long long *)aReturnData.data(), (short *)data(), nbItems);
//       break;
//     case INT64:
//       Data::_cast((long long *)aReturnData.data(), (short *)data(), nbItems);
//       break;
//     case FLOAT:
//       Data::_cast((float *)aReturnData.data(), (short *)data(), nbItems);
//       break;
//     case DOUBLE:
//       Data::_cast((double *)aReturnData.data(), (short *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   case UINT32:
//     switch (aReturnData.type) {
//     case UINT64:
//       Data::_cast((unsigned long long *)aReturnData.data(), (unsigned int *)data(), nbItems);
//       break;
//     case INT64:
//       Data::_cast((long long *)aReturnData.data(), (unsigned int *)data(), nbItems);
//       break;
//     case FLOAT:
//       Data::_cast((float *)aReturnData.data(), (unsigned int *)data(), nbItems);
//       break;
//     case DOUBLE:
//       Data::_cast((double *)aReturnData.data(), (unsigned int *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   case INT32:
//     switch (aReturnData.type) {
//     case UINT32:
//       Data::_cast((unsigned int *)aReturnData.data(), (int *)data(), nbItems);
//       break;
//     case UINT64:
//       Data::_cast((unsigned long long *)aReturnData.data(), (int *)data(), nbItems);
//       break;
//     case INT64:
//       Data::_cast((long long *)aReturnData.data(), (int *)data(), nbItems);
//       break;
//     case FLOAT:
//       Data::_cast((float *)aReturnData.data(), (int *)data(), nbItems);
//       break;
//     case DOUBLE:
//       Data::_cast((double *)aReturnData.data(), (int *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   case UINT64:
//     switch (aReturnData.type) {
//     case FLOAT:
//       Data::_cast((float *)aReturnData.data(), (unsigned long long *)data(), nbItems);
//       break;
//     case DOUBLE:
//       Data::_cast((double *)aReturnData.data(), (unsigned long long *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   case INT64:
//     switch (aReturnData.type) {
//     case UINT64:
//       Data::_cast((unsigned long long *)aReturnData.data(), (long long *)data(), nbItems);
//       break;
//     case FLOAT:
//       Data::_cast((float *)aReturnData.data(), (long long *)data(), nbItems);
//       break;
//     case DOUBLE:
//       Data::_cast((double *)aReturnData.data(), (long long *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   case FLOAT:
//     switch (aReturnData.type) {
//     case INT32:
//       Data::_cast((int *)aReturnData.data(), (float *)data(), nbItems);
//       break;
//     case INT64:
//       Data::_cast((long long *)aReturnData.data(), (float *)data(), nbItems);
//       break;
//     case DOUBLE:
//       Data::_cast((double *)aReturnData.data(), (float *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   case DOUBLE:
//     switch (aReturnData.type) {
//     case INT32:
//       Data::_cast((int *)aReturnData.data(), (double *)data(), nbItems);
//       break;
//     case INT64:
//       Data::_cast((long long *)aReturnData.data(), (double *)data(), nbItems);
//       break;
//     case FLOAT:
//       Data::_cast((float *)aReturnData.data(), (double *)data(), nbItems);
//       break;
//     default:
//       throw ProcessException("This cast is not manage");
//     }
//     break;
//   default:
//     throw ProcessException("This cast is not manage");
//   }
//   return aReturnData;
// }

template <typename Pixel, typename Allocator>
inline std::ostream &operator<<(std::ostream &os, const TypedData<Pixel, Allocator> &data)
{
  os << "<"
     << "type=" << data.type << " (" << detail::pixel_traits<Pixel>::Desc << "), ";

  int dimNb = 0;
  for (auto && dim : data.dimensions)
    os << "dimension_" << dimNb++ << "=" << dim << ", ";

  os << "frameNumber=" << data.frameNumber << ", "
     << "timestamp=" << data.timestamp << ", ";
  //<< "header=" << data.header << ", ";
  if (!data.buffer)
  //   os << "buffer=" << *data.buffer;
  // else
    os << "buffer="
       << "NULL";
  os << ">";
  return os;
}

#endif //! defined(PROCESSLIB_TYPEDDATA_H)
