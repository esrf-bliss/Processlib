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

#if !defined(PROCESSLIB_SIDEBAND_DATA_H)
#define PROCESSLIB_SIDEBAND_DATA_H

#include <memory>

#include "processlib/Compatibility.h"

namespace sideband
{

  // base class for all Sideband data
  class DLL_EXPORT Data
  {
  public:
    virtual ~Data() {}
  };

  typedef std::shared_ptr<Data> DataPtr;

  // cast a Sideband data into its original type
  template <typename T>
  std::shared_ptr<T> DataCast(DataPtr p)
  {
    return std::dynamic_pointer_cast<T>(p);
  }

} // namespace sideband

#endif // PROCESSLIB_SIDEBAND_DATA_H
