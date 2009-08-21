processlib-objs := ../core/src/GslErrorMgr.o ../core/src/PoolThreadMgr.o ../core/src/TaskEventCallback.o ../core/src/LinkTask.o ../core/src/SinkTask.o ../core/src/TaskMgr.o ../core/src/HeaderContainer.o ../tasks/src/BackgroundSubstraction.o ../tasks/src/FlatfieldCorrection.o ../tasks/src/RoiCounter.o ../tasks/src/Binning.o ../tasks/src/Flip.o ../tasks/src/SoftRoi.o ../tasks/src/Bpm.o ../tasks/src/Mask.o


SRCS=../core/src/GslErrorMgr.cpp ../core/src/PoolThreadMgr.cpp ../core/src/TaskEventCallback.cpp ../core/src/LinkTask.cpp ../core/src/SinkTask.cpp ../core/src/TaskMgr.cpp ../core/src/HeaderContainer.cpp ../tasks/src/BackgroundSubstraction.cpp ../tasks/src/FlatfieldCorrection.cpp ../tasks/src/RoiCounter.cpp ../tasks/src/Binning.cpp ../tasks/src/Flip.cpp ../tasks/src/SoftRoi.cpp ../tasks/src/Bpm.cpp ../tasks/src/Mask.cpp

INCLUDES=-I../core/include -I../tasks/include
CXXFLAGS += $(INCLUDES) -fPIC -Wall -pthread


all: libprocesslib.so


libprocesslib.so: $(processlib-objs)
	gcc -shared -o $@ $+ -lgsl -lgslcblas -pthread

clean:
	rm -f libprocesslib.so $(processlib-objs) ../core/src/*.P ../tasks/src/*.P 

%.o : %.cpp
	$(COMPILE.cpp) -MD $(CXXFLAGS) -o $@ $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include $(SRCS:.cpp=.P)
