#include <deque>
#include "Data.h"

class LinkTask;
class SinkTaskBase;

class TaskMgr
{
  struct Task
  {
    Task() : _linkTask(NULL) {}
    ~Task();
    Task* copy() const;
    LinkTask		       *_linkTask;
    std::deque<SinkTaskBase*>	_sinkTaskQueue;
  };
  typedef std::deque<Task*> StageTask;
  typedef std::deque<std::pair<SinkTaskBase*,bool> > PendingSinkTask;
  
public:
  class TaskWrap
  {
    friend class TaskMgr;
  public:
    virtual ~TaskWrap(){};
    virtual void process() = 0;
  protected:
    TaskWrap(TaskMgr &aMgr) : _Mgr(aMgr) {};

    inline void _endLinkTask(LinkTask *aFinnishedTask) 
      {_Mgr._endLinkTask(aFinnishedTask);}
    inline void _endSinkTask(SinkTaskBase *aFinnishedTask)
      {_Mgr._endSinkTask(aFinnishedTask);}
    inline void _setNextData(Data &aNextData)
      {_Mgr._nextData = aNextData;}
    inline Data& _currentData() 
      {return _Mgr._currentData;}
    TaskMgr &_Mgr;
  };
  friend class TaskWrap;
  
  TaskMgr();
  TaskMgr(const TaskMgr&);
  ~TaskMgr();

  void setInputData(Data &aData) {_currentData = aData;}
  bool setLinkTask(int aStage,LinkTask *);
  void addSinkTask(int aStage,SinkTaskBase *);
  TaskWrap* next();
  //@brief do all the task in synchronously
  void syncProcess();
private:
  StageTask		_Tasks;
  LinkTask		*_PendingLinkTask;
  PendingSinkTask	_PendingSinkTask;
  Data       _currentData;
  Data       _nextData;

  void _endLinkTask(LinkTask *aFinnishedTask);
  void _endSinkTask(SinkTaskBase *aFinnishedTask);
  void _goToNextStage();
};
