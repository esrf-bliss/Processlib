############################################################################
# This file is part of ProcessLib, a submodule of LImA project the
# Library for Image Acquisition
#
# Copyright (C) : 2009-2011
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
#!/usr/bin/env python


import processlib
import numpy

TaskMgr = processlib.TaskMgr();

#background
bckground = processlib.Tasks.BackgroundSubstraction();

backGroundImg = numpy.ones((2048,2048),dtype = numpy.ushort)
backGroundData = processlib.Data()
backGroundData.buffer = backGroundImg;

bckground.setBackgroundImageData(backGroundData)
TaskMgr.setLinkTask(0,bckground)

#binning
binning = processlib.Tasks.Binning()
binning.mXFactor = 2
binning.mYFactor = 2
TaskMgr.setLinkTask(1,binning)

#Roi Counter
RCMgr = processlib.Tasks.RoiCounterManager()
RCTask = processlib.Tasks.RoiCounterTask(RCMgr)
RCTask.setRoi(0,0,2048,2048)
TaskMgr.addSinkTask(1,RCTask);

#And finnaly set the chain task for all input images
poolThreadMgr = processlib.PoolThreadMgr.get()
poolThreadMgr.setNumberOfThread(2)
poolThreadMgr.setTaskMgr(TaskMgr)
