#ifndef _YAMMER_SERVICE_H_
#define _YAMMER_SERVICE_H_

#include <Stream.H>

namespace Yammer {

  class Task;

  // The Service class is one of the base classes in the application server
  // framework.  Since the framework is for creating concurrent servers
  // (servers which handle multiple clients at the same time), as opposed to
  // iterative servers (servers which handle clients one at a time), each
  // service must run asynchronously.  As such, the spawn() function should
  // spawn a thread and run an event loop.  The service is created by the 
  // ServiceAccessPoint class based on the type of the task it receives.  This
  // starting task is passed to the service through the spawn() method.  The
  // service should take ownership of the Stream and Task passed to it. 
  // The spawn() method is called by the ServiceAccessPoint and should not
  // throw an exception.  The Service should destroy itself when necessary,
  // i.e. client disconnect or fatal service error.

  class Service {  // a service handler
  public:
    virtual ~Service() {}

    virtual Service *clone() const = 0;

    virtual void spawn(Stream *stream, Task *startTask) throw() = 0;
  };

}

#endif

