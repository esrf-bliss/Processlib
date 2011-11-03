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

processlib-objs := ../core/src/GslErrorMgr.o ../core/src/PoolThreadMgr.o ../core/src/TaskEventCallback.o ../core/src/LinkTask.o ../core/src/SinkTask.o ../core/src/TaskMgr.o ../core/src/HeaderContainer.o ../tasks/src/BackgroundSubstraction.o ../tasks/src/FlatfieldCorrection.o ../tasks/src/RoiCounter.o ../tasks/src/Binning.o ../tasks/src/Flip.o ../tasks/src/SoftRoi.o ../tasks/src/Bpm.o ../tasks/src/Mask.o ../tasks/src/Roi2Spectrum.o ../tasks/src/Rotation.o

SRCS = $(processlib-objs:.o=.cpp)

INCLUDES=-I../core/include -I../tasks/include
CXXFLAGS += $(INCLUDES) -fPIC -Wall -pthread -O3


all: libprocesslib.so


libprocesslib.so: $(processlib-objs)
	gcc -shared -o $@ $+ $(LDFLAGS) -lgsl -lgslcblas -pthread

clean:
	rm -f libprocesslib.so $(processlib-objs) ../core/src/*.P ../tasks/src/*.P 

%.o : %.cpp
	$(COMPILE.cpp) -MD $(CXXFLAGS) -o $@ $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include $(SRCS:.cpp=.P)
