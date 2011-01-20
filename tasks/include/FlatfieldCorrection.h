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
#include "LinkTask.h"
namespace Tasks
{
  class DLL_EXPORT FlatfieldCorrection : public LinkTask
  {
  public:
    FlatfieldCorrection();
    FlatfieldCorrection(const FlatfieldCorrection&);
    void setFlatFieldImageData(Data &aData);
    void setXCenter(double);
    void setYCenter(double);
    void setLambda(double);
    void setDetectorDistance(double);
    virtual Data process(Data&);
  private:
    mutable Data _flatFieldImage;
    double	_xcenter;
    double	_ycenter;
    double	_lambda;
    double	_distance;
    bool	_flatFieldCorrectionDirty;
    void	_calcFlatFieldImage(int xSize,int ySize,
				    Data::TYPE aType);
  };
}
