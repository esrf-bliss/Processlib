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
#include <stdio.h>      /* for printf() and fprintf() */
#ifdef __unix
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <sstream>
#include <string>

class DLL_EXPORT Stat
{
public:
  Stat(const Data &data,const std::string &info) : _data(data),_info(info) 
  {
    if(!info.empty())
      gettimeofday(&_start,NULL);
  }
  ~Stat()
  {
    if(!_info.empty())
      {
	gettimeofday(&_end,NULL);
	double diff = (_end.tv_sec - _start.tv_sec) + 
	  (_end.tv_usec - _start.tv_usec) / 1e6;
	std::stringstream str;
	str << "take : " << diff << "s";
   
	_data.header.insertOrIncKey(_info,str.str());
      }
  }
private:
  Data		_data;
  std::string	_info;
  struct timeval _start,_end;
};
