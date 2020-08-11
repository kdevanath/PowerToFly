#ifndef _REFERENCE_IMPLEMENTATION_EXAMPLE_WEB_SERVER_H_
#define _REFERENCE_IMPLEMENTATION_EXAMPLE_WEB_SERVER_H_

#include <ace/SOCK_Acceptor.h> 

namespace ReferenceImplementation {

  class ExampleWebServer {
  public: 
    class BindFailed {};

    ExampleWebServer(int port) throw (BindFailed);

    static void *run(void *This);

    // add web services here

    ACE_SOCK_Acceptor socket() { return acceptor_; }

  private: 
    ACE_SOCK_Acceptor acceptor_;
  }; 
}

#endif

