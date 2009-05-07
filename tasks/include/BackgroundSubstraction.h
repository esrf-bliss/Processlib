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
