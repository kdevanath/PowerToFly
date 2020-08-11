#ifndef _YAMMER_SERVER_PROCESS_H_
#define _YAMMER_SERVER_PROCESS_H_

namespace Yammer {

  // ServerProcess daemonizes, installs signal handlers, and then waits to be 
  // terminated
  class ServerProcess {
  public:
    virtual ~ServerProcess();

    void start();
    virtual void serve() {}  // Template Method pattern

    static const char *logFileName() { return logFileName_; }
    static const char *signalCmd() { return signalCmd_; }

    static void logFileName(const char *name);
    static void signalCmd(const char *cmd);

  private:
    // used by a signal handler, statics are neeed
    static char *logFileName_;
    static char *signalCmd_;
  };
}

#endif
