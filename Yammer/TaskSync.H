#ifndef _YAMMER_TASK_SYNC_H_
#define _YAMMER_TASK_SYNC_H_

#include <ace/Synch.h>
#include <ace/Synch_T.h>
#include <Task.H>

namespace Yammer {

  // A Task pointer container used to synchronize request/reply communication.
  // Allows one thread to work on a request while another blocks for its reply.
  // The reply is sent through notify() and returned by wait().  State is
  // cleared when wait() returns causing a subsequent wait() to block again.
  // If synchronization is not needed, that is, when the work is not farmed out
  // to a separate thread, then the get/setTask methods can be used; in which 
  // case, the TaskSync class is merely a return mechanism.

  class TaskSync {
  public:
    TaskSync();

    Task *wait();
    void notify(Task *task);

    // non-locking versions
    Task *getTask() { return task_; }
    void setTask(Task *task) { task_ = task; }

  private:
    Task *task_;
    bool notified_;
    ACE_Thread_Mutex mutex_;
    ACE_Condition<ACE_Thread_Mutex> cond_;
  };
}

#endif

