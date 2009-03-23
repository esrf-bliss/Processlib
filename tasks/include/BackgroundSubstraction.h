#include "Task.h"
namespace Tasks
{
  class BackgroundSubstraction : public Task
  {
  public:
    BackgroundSubstraction();
    BackgroundSubstraction(const BackgroundSubstraction&);
    void setBackgroundImageData(Data &aData);
    virtual Data process(Data&);
    virtual Task* copy() const;
  private:
    mutable Data _backgroundImageData;
  };
}
