#include <deque>
#include "Data.h"
class Task;

class BackgroundProcessMgr
{
  typedef std::deque<std::deque<Task*> > StageTask;
  typedef std::deque<std::pair<Task*,bool> > PendingTask;
public:
  class TaskWrap
  {
    friend class BackgroundProcessMgr;
  public:
    ~TaskWrap();
    void operator()();
  private:
    TaskWrap(BackgroundProcessMgr&,Task*);
    Task *_Task;
    BackgroundProcessMgr &_Mgr;
  };
  friend class TaskWrap;
  
  BackgroundProcessMgr();
  BackgroundProcessMgr(const BackgroundProcessMgr&);
  ~BackgroundProcessMgr();

  void setInputData(Data &aData) {_currentData = aData;}
  void addTask(int aStage,Task *);
  TaskWrap next();
  //@brief do all the task in synchronously
  void syncProcess();
private:
  StageTask   _Tasks;
  PendingTask _PendingTask;
  Data       _currentData;
  Data       _nextData;

  void _endTask(Task *aFinnishedTask);
};
