#ifndef _YAMMER_UDP_STREAM_ADAPTER_H_
#define _YAMMER_UDP_STREAM_ADAPTER_H_

#include <ace/SOCK_Dgram.h> 
#include <ace/INET_Addr.h> 
#include <Stream.H> 

namespace Yammer {

  // This class adapts an ACE UDP socket to the Stream interface.

  class UDPStreamAdapter : public Stream {  // Adapter pattern
  public:
    UDPStreamAdapter();
    UDPStreamAdapter(const ACE_SOCK_Dgram &socket, const ACE_INET_Addr &addr);

    int read(void *buffer, size_t len) throw (NetworkError);
    int write(const void *buffer, size_t len) throw (NetworkError);

    int readv(const iovec *vec, int len) throw (NetworkError);
    int writev(const iovec *vec, int len) throw (NetworkError);

    void flush() {}
    void close();
    int handle() { return socket_.get_handle(); }

    ACE_SOCK_Dgram &getSocket() { return socket_; }
    void setSocket(const ACE_SOCK_Dgram &socket) { socket_ = socket; }

    ACE_INET_Addr &getAddress() { return addr_; }
    void setAddress(const ACE_INET_Addr &addr) { addr_ = addr; }

  private:
    ACE_SOCK_Dgram socket_;
    ACE_INET_Addr addr_;
  };

}

#endif

