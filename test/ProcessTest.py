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
