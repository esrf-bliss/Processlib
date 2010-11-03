#include "LinkTask.h"

namespace Tasks
{
  class DLL_EXPORT Mask : public LinkTask
  {
  public:
    Mask();
    Mask(const Mask&);
    void setMaskImageData(Data &);
    Data process(Data&);
  private:
    Data _MaskImage;
  };
}
