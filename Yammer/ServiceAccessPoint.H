#ifndef _YAMMER_SERVICE_ACCESS_POINT_H_
#define _YAMMER_SERVICE_ACCESS_POINT_H_

#include <ace/SOCK_Acceptor.h> 

namespace Yammer {

  class ServiceAccessPoint {
  public: 
    class BindFailed {};

    ServiceAccessPoint(int port) throw (BindFailed);

    static void *run(void *This);

    ACE_SOCK_Acceptor socket() { return acceptor_; }

  private: 
    ACE_SOCK_Acceptor acceptor_;
  }; 

}

#endif

