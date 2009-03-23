namespace Tasks
{
  class SoftRoi : Task
  {
%TypeHeaderCode
#include <SoftRoi.h>
%End
  public:
  void setRoi(int x1,int x2,
	      int y1,int y2);		
  virtual Data process(Data&);
  };
};