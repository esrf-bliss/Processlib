#ifndef __BACKGROUNDSUBSTRACTION__H_
#define __BACKGROUNDSUBSTRACTION__H_
#include "LinkTask.h"
namespace Tasks
{
  class BackgroundSubstraction : public LinkTask
  {
  public:
    BackgroundSubstraction();
    BackgroundSubstraction(const BackgroundSubstraction&);
    void setBackgroundImageData(Data &aData);
    virtual Data process(Data&);
  private:
    mutable Data _backgroundImageData;
  };
}
#endif
