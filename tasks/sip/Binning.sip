namespace Tasks
{
  class Binning :  LinkTask
  {
%TypeHeaderCode
#include <Binning.h>
%End
  public:
    virtual Data process(Data&);
    int mXFactor;
    int mYFactor;
  };
};
