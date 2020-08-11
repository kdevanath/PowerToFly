#ifndef _REFERENCE_IMPLEMENTATION_EXAMPLE_SERVER_H_
#define _REFERENCE_IMPLEMENTATION_EXAMPLE_SERVER_H_

#include <string> 
#include <ace/Singleton.h> 
#include <ace/Synch.h> 
#include <ServerProcess.H> 

namespace ReferenceImplementation {

  using std::string;
  using Yammer::ServerProcess;

  class ExampleServer : public ServerProcess {
  public:
    void configure(int argc, char **argv);

    void serve();

  private:
    string execName_;
    int port_;
  };

  typedef ACE_Singleton<ExampleServer, ACE_Thread_Mutex> ExampleServerSingleton;

}

#endif

