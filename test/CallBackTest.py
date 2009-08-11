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

