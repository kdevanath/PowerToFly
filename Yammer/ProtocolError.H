#ifndef _YAMMER_PROTOCOL_ERROR_H_
#define _YAMMER_PROTOCOL_ERROR_H_

#include <string> 
#include <Task.H> 
#include <TypeIds.H> 

namespace Yammer {

  using std::string;

  // sent when a service receives a different Task than expected
  class ProtocolError : public Task {
  public:
    ProtocolError() {}
    ProtocolError(const string &msg) : msg_(msg) {}

    ProtocolError *clone() const { return new ProtocolError(*this); }

    void run() {}

    int type() const { return ProtocolErrorId; }

    string name() const { return "ProtocolError"; }

    void toStream(Stream &stream) const throw (NetworkError)
      { stream << type(); }

    void fromStream(Stream &stream) throw (NetworkError) {}

    string msg() const { return msg_; }
    void msg(const string &msg) { msg_ = msg; }

  private:
    string msg_;
  };
}

#endif
