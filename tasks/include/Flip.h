#include "LinkTask.h"
namespace Tasks
{
  class DLL_EXPORT Flip : public LinkTask
  {
  public:
    enum FLIP_MODE {
      FLIP_NONE,
      FLIP_X,
      FLIP_Y,
      FLIP_ALL
    };
    Flip();
    Flip(const Flip&);
    void setFlip(FLIP_MODE);
 
   virtual Data process(Data&);
  private:
    FLIP_MODE _mode;
  };
}
