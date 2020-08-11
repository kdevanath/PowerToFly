#ifndef _YAMMER_DISCONNECTION_REQUEST_H_
#define _YAMMER_DISCONNECTION_REQUEST_H_

#include <Task.H> 
#include <TypeIds.H> 

namespace Yammer {

  class DisconnectionRequest : public Task {
  public:
    DisconnectionRequest *clone() const
      { return new DisconnectionRequest(*this); }

    void run() {}

    int type() const { return DisconnectionRequestId; }

    string name() const { return "DisconnectionRequest"; }

    void toStream(Stream &stream) const throw (NetworkError)
      { stream << type(); }

    void fromStream(Stream &stream) throw (NetworkError) {}
  };

}

#endif

