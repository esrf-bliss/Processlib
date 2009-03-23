#include "Task.h"
namespace Tasks
{
  class FlatfieldCorrection : public Task
  {
  public:
    FlatfieldCorrection();
    FlatfieldCorrection(const FlatfieldCorrection&);
    void setFlatFieldImageData(Data &aData);
    void setXCenter(double);
    void setYCenter(double);
    void setLambda(double);
    void setDetectorDistance(double);
    virtual Data process(Data&);
    virtual Task* copy() const;
  private:
    mutable Data _flatFieldImage;
    double	_xcenter;
    double	_ycenter;
    double	_lambda;
    double	_distance;
    bool	_flatFieldCorrectionDirty;
    void	_calcFlatFieldImage(int xSize,int ySize,
				    Data::TYPE aType);
  };
}
