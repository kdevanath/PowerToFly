#ifndef _YAMMER_SERVICE_REQUEST_ACK_H_
#define _YAMMER_SERVICE_REQUEST_ACK_H_

#include <Task.H> 
#include <TypeIds.H> 

namespace Yammer {

  class ServiceRequestAck : public Task {
  public:
    ServiceRequestAck *clone() const
      { return new ServiceRequestAck(*this); }

    void run() {}

    int type() const { return ServiceRequestAckId; }

    string name() const { return "ServiceRequestAck"; }

    void toStream(Stream &stream) const throw (NetworkError)
      { stream << type(); }

    void fromStream(Stream &stream) throw (NetworkError) {}
  };
}

#endif

