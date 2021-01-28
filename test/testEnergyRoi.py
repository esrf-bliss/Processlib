from Lima import Core
import numpy


a = numpy.ones((2048,2048),dtype=numpy.uint16)

d = Core.Processlib.Data()
d.frameNumber=0
d.buffer=a

m = Core.Processlib.Tasks.RoiCollectionManager()
xstart=numpy.arange(0,2048,4)
ystart=numpy.arange(0,2048,4)
energy=1
for y in ystart:
    for x in xstart:
        m.addRoi(x,y,3,3,energy)
        energy += 1

