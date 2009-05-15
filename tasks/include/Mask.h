#include "LinkTask.h"

namespace Tasks
{
  class Mask : public LinkTask
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
