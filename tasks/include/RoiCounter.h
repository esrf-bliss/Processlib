#ifndef __ROICOUNTER_H__
#define __ROICOUNTER_H__

#include <sstream>
#include "SinkTask.h"

namespace Tasks
{
  class RoiCounterResult;
  
  typedef SinkTaskMgr<RoiCounterResult> RoiCounterManager;

  struct RoiCounterResult
  {
    RoiCounterResult() : 
      sum(0.),average(0.),std(0.),
      frameNumber(-1),
      errorCode(RoiCounterManager::OK)
    {}
    explicit RoiCounterResult(RoiCounterManager::ErrorCode code) : 
      errorCode(code) {}

    double                       sum;
    double                       average;
    double                       std;
    int                          frameNumber;
    RoiCounterManager::ErrorCode errorCode;
  };

  inline std::ostream& operator<<(std::ostream &os,const RoiCounterResult &aRoiResult)
    {
      os << "<"
	 << "frameNumber=" << aRoiResult.frameNumber << ", "
	 << "sum=" << aRoiResult.sum << ", "
	 << "average=" << aRoiResult.average << ", "
	 << "std=" << aRoiResult.std;
      os << ">";
      return os;
    }

  class RoiCounterTask : public SinkTask<RoiCounterResult>
  {
  public:
    RoiCounterTask(RoiCounterManager&);
    RoiCounterTask(const RoiCounterTask&);
    virtual void process(Data&);
    
    void setMask(Data &aMask) {_mask = aMask;}

    void setRoi(int x,int y,int width,int height);
    void getRoi(int &x,int &y,int &width,int &height)
    {
      x = _x,y = _y,width = _width,height = _height;
    }
  private:
    int _x,_y;
    int _width,_height;
    Data _mask;
  };
}
#endif
