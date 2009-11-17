processlib-objs := ../core/src/GslErrorMgr.o ../core/src/PoolThreadMgr.o ../core/src/TaskEventCallback.o ../core/src/LinkTask.o ../core/src/SinkTask.o ../core/src/TaskMgr.o ../core/src/HeaderContainer.o ../tasks/src/BackgroundSubstraction.o ../tasks/src/FlatfieldCorrection.o ../tasks/src/RoiCounter.o ../tasks/src/Binning.o ../tasks/src/Flip.o ../tasks/src/SoftRoi.o ../tasks/src/Bpm.o ../tasks/src/Mask.o

SRCS = $(processlib-objs:.o=.cpp)

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
