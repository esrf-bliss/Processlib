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

def parab(p,x) :
    return p[0] + p[1] * x + p[2] * (x * x)

errfunc = lambda p,x,y: parab(p,x) - y


def gauss(p,x) :
    return (p[0] * numpy.exp((-(x - p[1]) ** 2) / p[2])) + p[3]

errfuncGauss = lambda p,x,y: gauss(p,x) - y

import processlib
import EdfFile
import time
import numpy
from scipy import optimize

f = EdfFile.EdfFile('crl8_0.edf')
beam = f.GetData(0)

lines = beam.sum(axis=0)
column = beam.sum(axis=1)

lFile = file('line.txt','w')
for i,x in enumerate(lines) :
    lFile.write('%s %s\n' % (str(i),str(x)))


cFile = file('column.txt','w')
for i,x in enumerate(column):
    cFile.write('%s %s\n' % (str(i),str(x)))


s = time.time()
p1,sucess = optimize.leastsq(errfunc,
                             [0.,1.,1.],
                             args = (numpy.arange(lines.shape[0]),lines))
e = time.time()
print 'x',-p1[1]/(2 * p1[2]),e - s


maxVal = lines.max()
indMax = lines.argmax()
noiseVal = lines[0]

s = time.time()
p3,sucess = optimize.leastsq(errfuncGauss,
                             [maxVal - noiseVal,indMax,1,noiseVal],
                             args = (numpy.arange(lines.shape[0]),lines))
e = time.time()
print 'x gauss',p3[1],e-s


s = time.time()
p2,sucess = optimize.leastsq(errfunc,
                             [0.,1.,1.],
                             args = (numpy.arange(column.shape[0]),column))
e = time.time()

print 'y',-p2[1]/(2 * p2[2]),e - s

maxVal = column.max()
indMax = column.argmax()
noiseVal = column[0]

s = time.time()
p4,sucess = optimize.leastsq(errfuncGauss,
                             [maxVal - noiseVal,indMax,1,noiseVal],
                             args = (numpy.arange(column.shape[0]),column))
e = time.time()

print 'y gauss',p4,p4[1],e-s


BpmMgr = processlib.Tasks.BpmManager()
BpmTask = processlib.Tasks.BpmTask(BpmMgr)


beamData = processlib.Data()
beamData.frameNumber = 1

beamData.buffer = beam


BpmTask.mEnableX = True
BpmTask.mEnableY = True
#BpmTask.mThreshold = 42
s = time.time()
BpmTask.process(beamData)
print 'Bpm take',time.time() - s
result = BpmMgr.getResult()

print "beam_intensity",result.beam_intensity
print "beam_center_x",result.beam_center_x
print "beam_center_y",result.beam_center_y
print "beam_fwhm_x",result.beam_fwhm_x
print "beam_fwhm_min_x_index",result.beam_fwhm_min_x_index
print "beam_fwhm_max_x_index",result.beam_fwhm_max_x_index
print "beam_fwhm_y",result.beam_fwhm_y
print "beam_fwhm_min_y_index",result.beam_fwhm_min_y_index
print "beam_fwhm_max_y_index",result.beam_fwhm_max_y_index
print "max_pixel_value",result.max_pixel_value
print "max_pixel_x",result.max_pixel_x
print "max_pixel_y",result.max_pixel_y

print "frameNumber",result.frameNumber
      
print "AOI_automatic",result.AOI_automatic
print "AOI_extension_factor",result.AOI_extension_factor
print "AOI_min_x",result.AOI_min_x
print "AOI_max_x",result.AOI_max_x
print "AOI_min_y",result.AOI_min_y
print "AOI_max_y",result.AOI_max_y
print "border_exclusion",result.border_exclusion

