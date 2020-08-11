#ifndef _REFERENCE_IMPLEMENTATION_EXAMPLE_SERVICE_REQUEST_H_
#define _REFERENCE_IMPLEMENTATION_EXAMPLE_SERVICE_REQUEST_H_

#include <Task.H> 
#include <ExampleTypeIds.H> 

namespace ReferenceImplementation {

  using std::string;
  using Yammer::Task;
  using Yammer::Stream;
  using Yammer::NetworkError;

  class ExampleServiceRequest : public Task {
  public:
    ExampleServiceRequest *clone() const
      { return new ExampleServiceRequest(*this); }

    void run() {}

    int type() const { return ExampleServiceRequestId; }

    string name() const { return "ExampleServiceRequest"; }

    void toStream(Stream &stream) const throw (NetworkError)
      { stream << type(); }

    void fromStream(Stream &stream) throw (NetworkError) {}
  };
}

#endif

