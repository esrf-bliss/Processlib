namespace Tasks
{
  class Flip : LinkTask
  {
%TypeHeaderCode
#include <Flip.h>
%End
  public:
  enum FLIP_MODE {FLIP_NONE, FLIP_X, FLIP_Y, FLIP_ALL};
  void setFlip(FLIP_MODE);
  virtual Data process(Data&);
  };
};