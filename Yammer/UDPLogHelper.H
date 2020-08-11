#ifndef _YAMMER_UDP_LOG_HELPER_H_
#define _YAMMER_UDP_LOG_HELPER_H_

#include <ace/INET_Addr.h>
#include <UDPStreamAdapter.H>
#include <BufferedStream.H>
#include <Logger.H>

// macros to log a message along with peer hostname and port info
// macros are used to show proper file and line info

// args: stream is a BufferedStream using UDPStreamAdapter
//       str is a string
#define logPeerInfo(stream, str)                                         \
{                                                                        \
  ACE_INET_Addr addr;                                                    \
  using Yammer::UDPStreamAdapter;                                  \
  UDPStreamAdapter *udp_stream;                                          \
  if (udp_stream = dynamic_cast<UDPStreamAdapter*>(stream.getStream()))  \
  addr = udp_stream->getAddress();					\
  LOG(str << addr.get_host_name() << ':' << addr.get_port_number());     \
}

// args: stream is a BufferedStream using UDPStreamAdapter
//       ex is an exception having a what() method
#define logException(stream, ex)                                         \
  logPeerInfo(stream, ex.what() << ": client=")
  
#endif

