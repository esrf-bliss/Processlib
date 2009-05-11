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
