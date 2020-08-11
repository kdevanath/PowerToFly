#ifndef _REFERENCE_IMPLEMENTATION_EXAMPLE_SERVICE_H_
#define _REFERENCE_IMPLEMENTATION_EXAMPLE_SERVICE_H_

#include <Service.H>
#include <Task.H>
#include <Stream.H>

namespace ReferenceImplementation {

  using Yammer::Service;
  using Yammer::Task;
  using Yammer::Stream;

  // client event loop
  class ExampleService : public Service {
  public: 
    ExampleService();
    ~ExampleService();

    ExampleService *clone() const { return new ExampleService(*this); } 

    void spawn(Stream *stream, Task *startTask) throw();

    static void *run(void *This);

    Stream *stream() { return stream_; }

  private:
    Stream *stream_;
  }; 
}

#endif

