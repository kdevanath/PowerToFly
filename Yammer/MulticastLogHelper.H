#ifndef _YAMMER_MC_LOG_HELPER_H_
#define _YAMMER_MC_LOG_HELPER_H_

#include <ace/INET_Addr.h>
#include <MulticastStreamAdapter.H>
#include <BufferedStream.H>
#include <Logger.H>

// macros to log a message along with peer hostname and port info
// macros are used to show proper file and line info

// args: stream is a BufferedStream using MulticastStreamAdapter
//       str is a string
#define logPeerInfo(stream, str)                                         \
{                                                                        \
  ACE_INET_Addr addr;                                                    \
  using Yammer::MulticastStreamAdapter;                                  \
  MulticastStreamAdapter *mc_stream;                                          \
  if (mc_stream = dynamic_cast<MulticastStreamAdapter*>(stream.getStream()))  \
  addr = mc_stream->getAddress();					\
  LOG(str << addr.get_host_name() << ':' << addr.get_port_number());     \
}

// args: stream is a BufferedStream using MulticastStreamAdapter
//       ex is an exception having a what() method
#define logException(stream, ex)                                         \
  logPeerInfo(stream, ex.what() << ": client=")
  
#endif

