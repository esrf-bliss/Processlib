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

#if !defined(PROCESSLIB_SIDEBAND_COMPRESSEDDATA_H)
#define PROCESSLIB_SIDEBAND_COMPRESSEDDATA_H

#include <utility>
#include <vector>

#include "processlib/Sideband/Data.h"

namespace Sideband
{
  // A Blob is a block of bytes, which can be copied & xferred
  typedef std::pair<std::shared_ptr<void>, std::size_t> Blob;
  typedef std::vector<Blob> BlobList;

  // Sideband data providing compressed blobs for a frame
  class CompressedData : public Sideband::Data
  {
  public:
    std::vector<int> decomp_dims;
    int pixel_depth;
    BlobList comp_blobs;

    CompressedData(std::vector<int> dims, int depth, BlobList blobs)
      : decomp_dims(std::move(dims)), pixel_depth(depth),
	comp_blobs(std::move(blobs)) {}
  };

} // namespace Sideband

#endif // PROCESSLIB_SIDEBAND_COMPRESSEDDATA_H
