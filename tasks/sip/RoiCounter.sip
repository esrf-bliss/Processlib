namespace Tasks
{
  class RoiCounterManager
   {
%TypeHeaderCode
#include <RoiCounter.h>
%End
    public:
    enum ErrorCode {OK,NO_MORE_AVAILABLE,TIMEDOUT};
    enum RUN_MODE {Counter,Monitor};
    RoiCounterManager(int = 4);
    ~RoiCounterManager();

    Tasks::RoiCounterResult	getResult(double = 0.,int = -1) const;
    void	resizeHistory(int aSize);
    void  resetHistory();
    int	  historySize() const;
    int   lastFrameNumber() const;
   };


  struct RoiCounterResult
  {
%TypeHeaderCode
#include <RoiCounter.h>
%End
    double                       sum;
    double                       average;
    double                       std;
    int                          frameNumber;
    Tasks::RoiCounterManager::ErrorCode errorCode;
  };
  
  class RoiCounterTask : SinkTaskBase
  {
%TypeHeaderCode
#include <RoiCounter.h>
%End
  public:
    RoiCounterTask(Tasks::RoiCounterManager&);
    RoiCounterTask(const Tasks::RoiCounterTask&);
    virtual void process(Data&);

    void setRoi(int,int,int,int);
    void getRoi(int &x /Out/,int &y /Out/,
		int &width /Out/,int &height /Out/);
   };
};
