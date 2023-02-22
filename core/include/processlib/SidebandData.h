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

#if !defined(PROCESSLIB_SIDEBANDDATA_H)
#define PROCESSLIB_SIDEBANDDATA_H

#include "processlib/Data.h"

namespace Sideband
{
  // processlib ::Data API
  template <typename T>
  void AddData(const char *key, ::Data& data, std::shared_ptr<T> sb_data)
  {
    AddContainerData<T>(key, data.sidebandData, sb_data);
  }

  template <typename T>
  std::shared_ptr<T> GetData(const char *key, ::Data& data)
  {
    return GetContainerData<T>(key, data.sidebandData);
  }

  inline bool RemoveData(const char *key, ::Data& data)
  {
    return RemoveContainerData(key, data.sidebandData);
  }

} // namespace Sideband

#endif // PROCESSLIB_SIDEBANDDATA_H
