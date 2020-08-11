#ifndef _YAMMER_NETWORK_EXCEPTIONS_H_
#define _YAMMER_NETWORK_EXCEPTIONS_H_

#include <string>
#include <sstream>
#include <ace/OS.h>

namespace Yammer {

  using std::string;
  using std::stringstream;

  // network exceptions

  class NetworkError {
  public:
    virtual ~NetworkError() {}
    virtual const char *what() const = 0;
  };

  class BrokenPipe : public NetworkError {
  public:
    const char *what() const { return "BrokenPipe"; }
  };

  class PeerClosed : public NetworkError {
  public:
    const char *what() const { return "PeerClosed"; }
  };

  class SocketError : public NetworkError {
  public: 
    SocketError() {
      stringstream ss;
      int err = ACE_OS::last_error();
      ss << "SocketError (" << err << ") " << ACE_OS_String::strerror(err);
      msg_ = ss.str();
    }
    const char *what() const  { return msg_.c_str(); }

  private:
    string msg_;
  };

  class InterruptException : public NetworkError {
  public:
    const char *what() const { return "InterruptException"; }
  };

  class TimeoutException : public NetworkError {
  public:
    const char *what() const { return "TimeoutException"; }
  };

}

#endif

