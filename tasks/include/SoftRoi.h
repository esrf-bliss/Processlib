#include "Task.h"
namespace Tasks
{
  class SoftRoi : public Task
  {
  public:
    SoftRoi();
    SoftRoi(const SoftRoi&);
    virtual Data process(Data&);
    virtual Task* copy() const;
    void setRoi(int x1,int x2,
		int y1,int y2);
  private:
    int _x1,_x2;
    int _y1,_y2;
  };
}
