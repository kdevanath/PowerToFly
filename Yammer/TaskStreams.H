#ifndef _TASK_STREAMS_H_
#define _TASK_STREAMS_H_

#include <memory> 
#include <Task.H> 
#include <Stream.H> 
#include <NetworkExceptions.H> 
#include <UnknownTask.H> 

namespace Yammer {

  using std::auto_ptr;

  // stream I/O operators for Task objects

  Stream &operator>>(Stream&, Task*&) throw (UnknownTask, NetworkError);

  Stream &operator<<(Stream&, const Task*) throw (NetworkError);


  // helper functions are for syntax only, semantics are the same
  Stream &operator>>(Stream&, std::auto_ptr<Task>&)
    throw (UnknownTask, NetworkError);

  Stream &operator<<(Stream&, const Task&) throw (NetworkError);

  Stream &operator<<(Stream&, auto_ptr<Task>&) throw (NetworkError);

}

#endif

