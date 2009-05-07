#include "LinkTask.h"
namespace Tasks
{
  class FlatfieldCorrection : public LinkTask
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
