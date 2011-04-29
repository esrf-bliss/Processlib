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
import processlib
import numpy
import time

a = numpy.arange(2048*2048,dtype=numpy.uint16)
a.shape=2048,-1
d = processlib.Data()
d.buffer = a
d.frameNumber=2048
RCMgr = processlib.Tasks.RoiCounterManager()
RCTask = processlib.Tasks.RoiCounterTask(RCMgr)
RCTask.setRoi(0,0,2048,2048)
    
def func() :

    RCTask.process(d)

    s = time.time()
    result = RCMgr.getResult()
    print 'take : ',time.time() - s,
    print result.sum,result.average,result.std


for i in range(10) :
    func()
