#include "LinkTask.h"
namespace Tasks
{
  class DLL_EXPORT Binning : public LinkTask
  {
  public:
    Binning();
    Binning(const Binning&);

    virtual Data process(Data&);

    int mXFactor;
    int mYFactor;
  };
}
