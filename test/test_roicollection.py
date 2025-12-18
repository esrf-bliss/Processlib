from lima import processlib
import numpy
import time


def test_roicollection():
    a = numpy.ones((2048,2048),dtype=numpy.uint16)

    d = processlib.Data()
    d.frameNumber=0
    d.buffer=a

    m = processlib.Tasks.RoiCollectionManager()
    xstart=numpy.arange(0,2048,4)
    ystart=numpy.arange(0,2048,4)

    rois = []
    #create 262144 rois
    for y in ystart:
        for x in xstart:
            rois.append((x,y,3,3))
    m.setRois(rois)

    print("Will process 262144 rois on an image 2048x2048-16bpp")
    t0=time.time()
    m.prepare()
    print(f"prepare() took {time.time()-t0:.6f} sec.")
    t0=time.time()
    m.process(d)
    print(f"process() took {time.time()-t0:.6f} sec.")

    r=m.getResult(0)
    s=r.spectrum
    print (f"Processed {len(s)} roi counters successfully")


def test_ram_usage():
    print (f"check now your RAM usage, we loop on reading the spectrum (issue #22 in Processlib")
    m = processlib.Tasks.RoiCollectionManager()
    r=m.getResult(0)
    for _ in range(1_000_000):
        r.spectrum
