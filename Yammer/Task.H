#ifndef _YAMMER_TASK_H_
#define _YAMMER_TASK_H_

#include <string>
#include <Stream.H>
#include <NetworkExceptions.H>

namespace Yammer {

  using std::string;

  class TaskSync;  // used to synchronize request/reply communication

  // A serializable task interface
  class Task {
  public:
    virtual ~Task() {}

    virtual Task *clone() const = 0;  // Prototype pattern

    virtual void run() = 0;  // Command pattern

    virtual int type() const = 0;

    virtual string name() const = 0;

    // for serialization
    virtual void toStream(Stream&) const throw (NetworkError) = 0;
    virtual void fromStream(Stream&) throw (NetworkError) = 0;

    // for synchronization
    // A waiting thread can block on the sync object until signaled by a 
    // worker thread.  Use of the needsReply() method allows code to handle
    // both request-only or request-reply communication.
    virtual void sync(TaskSync *sync) {}
    virtual bool needsReply() const { return false; }
  };


  // for ordering in a hashed container, or run-time type identification
  inline bool operator==(const Task &lhs, const Task &rhs)
    { return lhs.type() == rhs.type(); }

  inline bool operator!=(const Task &lhs, const Task &rhs)
    { return !(lhs == rhs); }

  // for ordering in a std (tree-based) container
  inline bool operator<(const Task &lhs, const Task &rhs)
    { return lhs.type() < rhs.type(); }

}

#endif

