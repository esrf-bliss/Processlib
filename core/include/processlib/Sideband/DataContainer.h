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

#if !defined(PROCESSLIB_SIDEBAND_DATACONTAINER_H)
#define PROCESSLIB_SIDEBAND_DATACONTAINER_H

#include "processlib/Sideband/Data.h"
#include <string>
#include <map>

#include "processlib/Compatibility.h"

namespace Sideband
{

  // Sideband container API
  typedef std::map<std::string, DataPtr> DataContainer;

  // add sideband data T to the container
  template <typename T>
  void AddContainerData(const std::string& key, DataContainer& cont,
			std::shared_ptr<T> sb_data)
  {
    cont[key] = sb_data;
  }

  // return sideband data T from the container
  template <typename T>
  std::shared_ptr<T> GetContainerData(const std::string& key, DataContainer& cont)
  {
    // Find Sideband
    typename DataContainer::iterator sit = cont.find(key);
    if (sit == cont.end())
      return {};
    // Return casted data
    return DataCast<T>(sit->second);
  }

  // remove sideband data from the container, return true if key existed
  inline bool RemoveContainerData(const std::string& key, DataContainer& cont)
  {
    std::size_t nb_erased = cont.erase(key);
    return (nb_erased > 0);
  }

} // namespace Sideband

#endif // PROCESSLIB_SIDEBAND_DATACONTAINER_H
