namespace Tasks
{
  class Binning :  Task
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
