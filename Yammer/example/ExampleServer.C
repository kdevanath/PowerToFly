#include <ExampleServer.H> 
#include <libgen.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <sstream>
#include <ace/OS.h>
#include <ServiceAccessPoint.H> 
#include <TaskDispatcher.H> 
#include <Logger.H> 
#include <ExampleWebServer.H> 

namespace ReferenceImplementation {

  using std::stringstream;
  using Yammer::TaskDispatcherSingleton;
  using Yammer::ServiceAccessPoint;
  using Yammer::Logger;

  void ExampleServer::configure(int argc, char **argv)
  {
    if (argc != 2) {
      cerr << "usage: " << argv[0] << " <port>" << endl;
      exit(1);
    }
    execName_ = ::basename(argv[0]);
    port_ = atoi(argv[1]);
  }

  void ExampleServer::serve() 
  { 
    // set the log file - a message will be logged here upon a core dump
    string log(execName_);
    log += ".log";
    logFileName(log.c_str());
    
    // set the signal command - this command will be called upon a core dump
    string cmd;
#ifdef __linux
    // append a stack trace to the log file
    cmd += "export PATH=/bin:/usr/bin; (echo STACK TRACE; pgrep -x ";
    cmd += execName_;
    cmd += " | xargs pstack | c++filt ) >> ";
    cmd += log;
#elif defined __sun
    // generate a named core file
    pid_t pid = ACE_OS::getpid();
    std::stringstream ss;
    ss << "/usr/bin/gcore -o " << execName_ << ".core " << pid;
    cmd = ss.str();
#endif
    signalCmd(cmd.c_str());

    // open the file used by the LOG macro
    Logger::fp_ = fopen(log.c_str(), "a");
    LOG("The server is starting up");
    LOG("port=" << port_);

    TaskDispatcherSingleton::instance()->startThreadPool();

    try {
      new ServiceAccessPoint(port_);  // allow clients to connect
    } catch (ServiceAccessPoint::BindFailed) { 
      LOG("ServiceAccessPoint: bind failed - another instance may be running");
      exit(0);
    }

    try {
      new ExampleWebServer(8080);  // web admin port
    } catch (ExampleWebServer::BindFailed) { 
      LOG("WebServer: bind failed - another instance may be running");
    }
    
  }

}

