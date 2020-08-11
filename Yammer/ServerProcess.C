#include <ServerProcess.H>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ace/OS.h>
#include <ace/ACE.h>
#include <Logger.H>

namespace Yammer {
   
  char *ServerProcess::logFileName_;
  char *ServerProcess::signalCmd_;

  extern "C" void __exitHandler()
  {
    LOG("The server is exiting");
  }

  // a signal handler should only call async-signal-safe functions
  extern "C" void __signalHandler(int sig)
  {
    const char *logFile = ServerProcess::logFileName();
    if (logFile) {
      int fd = ACE_OS::open(logFile, O_WRONLY | O_APPEND);
      if (fd != -1) {
        //                 0    5    0    5    0    5    012345
        char msg[]     = "\nCORE DUMPED  [ received signal    ]\n\n",
             cdigits[] = "0123456789";
        // overwrite positions 32 and 33; a signal is at most 2 digits
        for (int i = 33; sig; sig /= 10) 
          msg[i--] = cdigits[sig % 10];

        ACE_OS::write(fd, msg, sizeof(msg) - 1);
      }
    }

    const char *cmd = ServerProcess::signalCmd();
    if (cmd)
      system(cmd);

#ifdef __linux
    // The intial idea for signalCmd was to call gcore to generate a uniquely
    // named core file.  Since Linux does this already, we'll just abort to
    // force a core image.
    ACE_OS::signal(SIGABRT, SIG_DFL);  // reset signal disposition
    ACE_OS::kill(ACE_OS::getpid(), SIGABRT);  // dump core 
#endif
    _exit(0);  // don't call exit handlers
  }

  ServerProcess::~ServerProcess()
  {
    delete [] logFileName_;
    delete [] signalCmd_;
  }

  void ServerProcess::start() 
  {
    char dir[2048];
    getcwd(dir, sizeof(dir));

    ACE::daemonize(dir);  // fork away from controlling terminal

    // Attach standard streams to /dev/null to prevent standard I/O errors 
    ACE_HANDLE fd = ACE_OS::open("/dev/null", O_RDWR);
    ACE_OS::dup(fd);
    ACE_OS::dup(fd);

    atexit(__exitHandler);  // set exit handler

    // Catch synchronous signals that cause a core file so we can change its 
    // name as the default behavior overwrites existing core files.
    struct sigaction sa;
    sa.sa_handler = __signalHandler;  // process-wide handler
    ACE_OS::sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    int signals[] = { SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGBUS, SIGSEGV,
                      SIGTRAP, SIGSYS, SIGXCPU, SIGXFSZ };
    for (int i = 0; i < sizeof(signals) / sizeof(int); i++)
      ACE_OS::sigaction(signals[i], &sa, 0);

    // Ignore signals from writes on broken pipes; handled in-line
    sa.sa_handler = SIG_IGN;  
    ACE_OS::sigaction(SIGPIPE, &sa, 0);

    // Mask SIGTERM; will be inherited by spawned threads
    sigset_t set;
    ACE_OS::sigemptyset(&set);
    ACE_OS::sigaddset(&set, SIGTERM);
    ACE_OS::pthread_sigmask(SIG_BLOCK, &set, 0);
    
    serve();

    // Wait on SIGTERM; sent from a kill 
    int ret, sig;
    do ret = ACE_OS::sigwait(&set, &sig);
    while (ret == -1 && errno == EINTR);  // ignore interrupts, e.g. when traced

    // SIGTERM was delivered; call exit handlers
    exit(0); 
  }

  void ServerProcess::logFileName(const char *name)
  {
    if (name && name[0]) {
      delete [] logFileName_;
      strcpy(logFileName_ = new char[strlen(name) + 1], name);
    }
  }

  void ServerProcess::signalCmd(const char *cmd)
  {
    if (cmd && cmd[0]) {
      delete [] signalCmd_;
      strcpy(signalCmd_ = new char[strlen(cmd) + 1], cmd);
    }
  }

}

