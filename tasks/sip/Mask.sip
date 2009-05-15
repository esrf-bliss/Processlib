namespace Tasks
{
  class Mask : LinkTask
  {
%TypeHeaderCode
#include <Mask.h>
%End
  public:
    void setMaskImageData(Data &);
    virtual Data process(Data&);
  };
};