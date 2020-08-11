#ifndef _YAMMER_UNKNOWN_SERVICE_REQUEST_H_
#define _YAMMER_UNKNOWN_SERVICE_REQUEST_H_

#include <Task.H> 
#include <TypeIds.H> 

namespace Yammer {

  class UnknownServiceRequest : public Task {
  public:
    UnknownServiceRequest *clone() const
      { return new UnknownServiceRequest(*this); }

    void run() {}

    int type() const { return UnknownServiceRequestId; }

    string name() const { return "UnknownServiceRequest"; }

    void toStream(Stream &stream) const throw (NetworkError)
      { stream << type(); }

    void fromStream(Stream &stream) throw (NetworkError) {}

  };
}

#endif

