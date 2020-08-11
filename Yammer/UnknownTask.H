#ifndef _YAMMER_UNKNOWN_TASK_H_
#define _YAMMER_UNKNOWN_TASK_H_

#include <Task.H> 
#include <TypeIds.H> 

namespace Yammer {

  class UnknownTask : public Task {
  public:
    UnknownTask *clone() const { return new UnknownTask(*this); }

    void run() {}

    int type() const { return UnknownTaskId; }

    string name() const { return "UnknownTask"; }

    void toStream(Stream &stream) const throw (NetworkError)
      { stream << type(); }

    void fromStream(Stream &stream) throw (NetworkError) {}

  };
}

#endif
