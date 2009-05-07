#include "LinkTask.h"
namespace Tasks
{
  class Binning : public LinkTask
  {
  public:
    Binning();
    Binning(const Binning&);

    virtual Data process(Data&);

    int mXFactor;
    int mYFactor;
  };
}
