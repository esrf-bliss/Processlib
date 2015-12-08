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
#ifndef PROCESSLIB_SOFTROI_H
#define PROCESSLIB_SOFTROI_H
#include "processlib/LinkTask.h"
namespace Tasks
{
  class DLL_EXPORT SoftRoi : public LinkTask
  {
  public:
    SoftRoi();
    SoftRoi(const SoftRoi&);
    virtual Data process(Data&);
    void setRoi(int x1,int x2,
		int y1,int y2);
  private:
    int _x1,_x2;
    int _y1,_y2;
  };
}
#endif
