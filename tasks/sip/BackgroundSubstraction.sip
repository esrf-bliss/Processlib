namespace Tasks
{
  class BackgroundSubstraction : LinkTask
  {
%TypeHeaderCode
#include <BackgroundSubstraction.h>
%End
  public:
    void setBackgroundImageData(Data &aData);
    virtual Data process(Data&);
  };
};