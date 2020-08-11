#ifndef _YAMMER_MULTICAST_STREAM_ADAPTER_H_
#define _YAMMER_MULTICAST_STREAM_ADAPTER_H_

#include <ace/SOCK_Dgram_Mcast.h> 
#include <Stream.H> 

namespace Yammer {

  // This class adapts an ACE Multicast UDP socket to the Stream interface.
  //
  // The class is used for receiving data from a multicast group.  For sending
  // data to a multicast group, use UDPStreamAdapter.  The subscribe() method in
  // ACE_SOCK_Dgram_Mcast should be called prior to using this adapter.  If the 
  // host is multihomed, having more than one network card, the appropriate 
  // interface should be specified in the subscribe() method.

  class MulticastStreamAdapter : public Stream {  // Adapter pattern
  public:
    MulticastStreamAdapter();
    MulticastStreamAdapter(const ACE_SOCK_Dgram_Mcast &socket,
      const ACE_INET_Addr &addr);

    int read(void *buffer, size_t len) throw (NetworkError);
    int write(const void *buffer, size_t len) throw (NetworkError);

    int readv(const iovec *vec, int len) throw (NetworkError);
    int writev(const iovec *vec, int len) throw (NetworkError);

    void flush() {}
    void close();
    int handle() { return socket_.get_handle(); }

    ACE_SOCK_Dgram_Mcast &getSocket() { return socket_; }
    void setSocket(const ACE_SOCK_Dgram_Mcast &socket) { socket_ = socket; }

    ACE_INET_Addr &getAddress() { return addr_; }
    void setAddress(const ACE_INET_Addr &addr) { addr_ = addr; }

  private:
    ACE_SOCK_Dgram_Mcast socket_;
    ACE_INET_Addr addr_;
  };

}

#endif

