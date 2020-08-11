#include <TaskSync.H>

namespace Yammer {

  TaskSync::TaskSync() : task_(0), notified_(false), cond_(mutex_) {}

  Task *TaskSync::wait()
  {
    ACE_Guard<ACE_Thread_Mutex> guard(mutex_);
    while (notified_ == false) 
      cond_.wait();

    Task *task = task_;

    // reset state
    notified_ = false;
    task_ = 0;

    return task;
  }

  void TaskSync::notify(Task *task)
  {
    ACE_Guard<ACE_Thread_Mutex> guard(mutex_);
    task_ = task;
    notified_ = true;
    cond_.signal();
  }

}

