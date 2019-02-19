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

#if !defined(PROCESSLIB_STAT_H)
#define PROCESSLIB_STAT_H

#include <sstream>
#include <stdlib.h>
#include <string>

class PROCESSLIB_EXPORT Stat
{
  public:
    Stat(const Data &data, const std::string &info) : _data(data), _info(info)
    {
        if (!info.empty())
            _start = std::chrono::high_resolution_clock::now();
    }
    ~Stat()
    {
        if (!_info.empty())
        {
            _end      = std::chrono::high_resolution_clock::now();
            auto diff = _end - _start;
            std::stringstream str;
            str << "take : " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << "ms";

            _data.header.insertOrIncKey(_info, str.str());
        }
    }

  private:
    Data _data;
    std::string _info;
    std::chrono::high_resolution_clock::time_point _start, _end;
};

#endif //! defined(PROCESSLIB_STAT_H)
