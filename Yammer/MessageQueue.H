#ifndef _YAMMER_MESSAGE_QUEUE_H_
#define _YAMMER_MESSAGE_QUEUE_H_

#include <ace/Synch.h>
#include <ace/Synch_T.h>
#include <queue>

namespace Yammer {

  using std::queue;

  // A synchronized, bounded queue
  template <typename T> 
  class MessageQueue {
  public:
    enum { DEFAULT_QUEUE_DEPTH = 128 };

    MessageQueue(size_t sz = DEFAULT_QUEUE_DEPTH) : sema_(sz), cond_(mutex_) {}

    void push(const T &msg)
    {
      sema_.acquire();  // block if full

      ACE_Guard<ACE_Thread_Mutex> guard(mutex_);
      queue_.push(msg);
      cond_.signal();
    }

    void pop(T &msg)
    {
      ACE_Guard<ACE_Thread_Mutex> guard(mutex_);

      while (queue_.empty()) 
        cond_.wait();  // block if empty

      msg = queue_.front();
      queue_.pop();

      guard.release();
      sema_.release();
    }

    bool pop(T &msg, long timeout)
    {
      ACE_Guard<ACE_Thread_Mutex> guard(mutex_);

      if (queue_.empty()) {
        ACE_Time_Value millisecs(ACE_OS::time() + timeout / 1000, 
          timeout % 1000 * 1000);  // milliseconds -> microseconds
        cond_.wait(&millisecs);
        if (queue_.empty())
          return false;  // timed out
      }

      msg = queue_.front();
      queue_.pop();

      guard.release();
      sema_.release();
      return true;
    }

    bool empty() const 
    {
      ACE_Guard<ACE_Thread_Mutex> guard(mutex_);
      return queue_.empty();
    }
      
  private:
    ACE_Semaphore sema_;
    ACE_Thread_Mutex mutex_;
    ACE_Condition<ACE_Thread_Mutex> cond_;
    queue<T> queue_;
  };
}

#endif

