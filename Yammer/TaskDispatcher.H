#ifndef _YAMMER_TASK_DISPATCHER_H_
#define _YAMMER_TASK_DISPATCHER_H_

#include <Task.H> 
#include <MessageQueue.H> 
#include <ace/Singleton.h> 

namespace Yammer { 

  // TaskDispatcher calls the run method on each task; tasks can be run on 
  // the calling thread, or by a pool of threads.  The size of the thread pool
  // will default to the number of processors online.  The start() method can
  // be called directly to run the TaskDispatcher on the calling thread only.
  // Alternatively, the startThreadPool() method can be called to start a
  // pool of threads in which each thread calls the start() method.

  class TaskDispatcher {
  public:
    TaskDispatcher(size_t queueDepth = MessageQueue<Task*>::DEFAULT_QUEUE_DEPTH,
                   size_t nthreads = 0);
    
    void start();
    void stop();

    void startThreadPool();
    static void *run(void *This);

    void dispatch(Task *task);

  private:
    MessageQueue<Task*> requestQueue_;
    size_t nthreads_;
  };

  typedef ACE_Singleton<TaskDispatcher, ACE_Thread_Mutex> 
    TaskDispatcherSingleton;

}

#endif

