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
import time

lastData = None

class Ev(processlib.TaskEventCallback) :
    def __init__(self,name) :
        processlib.TaskEventCallback.__init__(self)
        self.__name = name

    def finished(self,data) :
        end = time.time()
        print '%s : Frame %d finished' % (self.__name,data.frameNumber)

TaskMgr = processlib.TaskMgr();

#background
bckground = processlib.Tasks.BackgroundSubstraction();

backGroundImg = numpy.ones((2048,2048),dtype = numpy.ushort)
backGroundData = processlib.Data()
backGroundData.buffer = backGroundImg;

bckground.setBackgroundImageData(backGroundData)
backGroundEvent = Ev('Background Sub')
bckground.setEventCallback(backGroundEvent)
TaskMgr.setLinkTask(0,bckground)

#roi
softRoi = processlib.Tasks.SoftRoi()
softRoi.setRoi(256,769,256,513)
softRoiEvent = Ev('SoftRoi')
softRoi.setEventCallback(softRoiEvent)
TaskMgr.setLinkTask(1,softRoi)

#binning
binning = processlib.Tasks.Binning()
binning.mXFactor = 2
binning.mYFactor = 2
binningEvent = Ev('Binning')
binning.setEventCallback(binningEvent)
TaskMgr.setLinkTask(2,binning)

#Python Task
class PyLinkTask(processlib.LinkTask) :
    def __init__(self) :
        processlib.LinkTask.__init__(self)
    def process(self,data) :
        print data.buffer
        return data

pyTask = PyLinkTask()
TaskMgr.setLinkTask(3,pyTask)

SrcData = processlib.Data()
poolThreadMgr = processlib.PoolThreadMgr.get()
poolThreadMgr.setNumberOfThread(2)

for i in range(10) :
    SrcData.frameNumber = i
    dataArray = numpy.ones((2048,2048),dtype = numpy.ushort)
    dataArray *= i
    SrcData.buffer = dataArray
    TaskMgr.setInputData(SrcData)
    poolThreadMgr.addProcess(TaskMgr)

print "Before quit" 
poolThreadMgr.quit()
print "After quit"

