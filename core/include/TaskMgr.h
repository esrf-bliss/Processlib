#include <deque>
#include "Data.h"
class Task;

class TaskMgr
{
  typedef std::deque<std::deque<Task*> > StageTask;
  typedef std::deque<std::pair<Task*,bool> > PendingTask;
public:
  class TaskWrap
  {
    friend class TaskMgr;
  public:
    ~TaskWrap();
    void operator()();
  private:
    TaskWrap(TaskMgr&,Task*);
    Task *_Task;
    TaskMgr &_Mgr;
  };
  friend class TaskWrap;
  
  TaskMgr();
  TaskMgr(const TaskMgr&);
  ~TaskMgr();

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
