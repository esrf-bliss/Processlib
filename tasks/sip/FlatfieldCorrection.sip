
namespace Tasks
{
  class FlatfieldCorrection :  Task
  {
%TypeHeaderCode
#include <FlatfieldCorrection.h>
%End
  public:
    void setFlatFieldImageData(Data &aData);
    void setXCenter(double);
    void setYCenter(double);
    void setLambda(double);
    void setDetectorDistance(double);
    virtual Data process(Data&);
  };
};
