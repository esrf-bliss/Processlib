#ifndef __ROICOUNTER_H__
#define __ROICOUNTER_H__

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
  
  class RoiCounterTask : public SinkTask<RoiCounterResult>
  {
  public:
    RoiCounterTask(RoiCounterManager&);
    RoiCounterTask(const RoiCounterTask&);
    virtual void process(Data&);

    void setRoi(int x,int y,int width,int height);
  private:
    int _x,_y;
    int _width,_height;
  };
}
#endif
