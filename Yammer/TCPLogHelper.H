#ifndef _YAMMER_TCP_LOG_HELPER_H_
#define _YAMMER_TCP_LOG_HELPER_H_

#include <string>
#include <sstream>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Stream.h>
#include <Stream.H>
#include <TCPStreamAdapter.H>

namespace Yammer {

  using std::string;
  using std::stringstream;

  inline string tcpPeerInfo(ACE_SOCK_Stream &stream)
  {
    ACE_INET_Addr addr;
    stream.get_remote_addr(addr);
    stringstream ss;
    ss << addr.get_host_name() << ':' << addr.get_port_number();
    return ss.str();
  }

  inline string tcpPeerInfo(Stream *stream)
  {
    TCPStreamAdapter *tcpStream= dynamic_cast<TCPStreamAdapter*>(stream);
    if (tcpStream == 0)
      return "unknown host";

    return tcpPeerInfo(tcpStream->getStream());
  }

}
  
#endif

