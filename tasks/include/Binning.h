#include "Task.h"
namespace Tasks
{
  class Binning : public Task
  {
  public:
    Binning();
    Binning(const Binning&);

    virtual Data process(Data&);
    virtual Task* copy() const;

    int mXFactor;
    int mYFactor;
  };
}
